from flask import Flask, request
from concurrent.futures import ThreadPoolExecutor
import firebase_admin
from firebase_admin import db, credentials
from dnslib import DNSRecord
import ipaddress
import socket
import base64
import random
import yaml


multi_id = 0

# Crea una instancia de la aplicación Flask
app = Flask(__name__)
# Cantidad máxima de threads que se van a correr simultaneamente
executor = ThreadPoolExecutor(max_workers=10)


# =========== Petición HTTP POST dns_resolver ===========

@app.route('/api/dns_resolver', methods=['POST'])
def dns_resolver_router():
    executor.submit(dns_resolver(request)) # Nuevo thread 
    return ""

def dns_resolver(request):
    print()
    print("--------------")
    print("=== DNS RESOLVER ===")
    print()

    content = request.get_data(as_text=True)
    print("Contenido de la petición recibida: ", content)

    content_decoded = base64.b64decode(content)
    print("Contenido de la petición recibida decodificado en Base64:", content_decoded)
    print()

    # Se envia la peticion al DNS Remoto y se recibe una respuesta
    respuesta_dns = dns_resolver_remote(content_decoded, "8.8.8.8")

    # Se le envia la respuesta al Interceptor
    send_interceptor(respuesta_dns)

# Petición a DNS Remoto
def dns_resolver_remote(content_decoded, servidor_dns):
    cliente_dns = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    cliente_dns.settimeout(30)  # Timeout si no recibe respuesta del DNS Remoto en 30 segundos

    respuesta = ""
    try:
        # Se le envia peticion al DNS Remoto
        cliente_dns.sendto(content_decoded, (servidor_dns, 53))
        
        # Recibe la respuesta del DNS Remoto
        respuesta, _ = cliente_dns.recvfrom(1024)
        
        print()
        print("Respuesta obtenida del servidor DNS: ", respuesta)
    
    except socket.timeout:
        print("Tiempo de espera agotado. No se recibió ninguna respuesta del servidor DNS Remoto.")
    
    finally:
        # Se cierra el socket
        cliente_dns.close()

    return respuesta


# =========== Petición HTTP GET exists ===========

@app.route('/api/exists', methods=['GET'])
def exists_router():
    executor.submit(exists(request)) # Nuevo thread 
    return ""

def exists(request):
    print()
    print("--------------")
    print("=== EXISTS ===")
    print()

    # Argumento 'message'
    message = request.args.get('message')
    print("Argumento 'message' recibido en la petición: ", message)
    message_decoded = ""

    if message:
        # Se verifica que el 'message' no esté vacío
        try:
            message_decoded = base64.b64decode(message)
            print("Argumento 'message' de la petición decodificado en Base64: ", message_decoded)
            print()
        except Exception as e:
            print("Error al decodificar el 'message' en Base64: ", e)
    else:
        print("No se recibió ningún 'message' en la petición.")

    # Argumento 'source_ip'
    source_ip = request.args.get('source_ip')
    print("Argumento 'source_ip' recibido en la petición: ", source_ip)
    source_ip_decoded = ""

    if source_ip:
        # Se verificar que el 'source_ip' no esté vacío
        try:
            source_ip_decoded = base64.b64decode(source_ip)
            source_ip_decoded = source_ip_decoded.decode('utf-8')
            source_ip_decoded = source_ip_decoded.strip('\x00') # Se le quitan caracteres adicionales de la decodificación
            print("Argumento 'source_ip' de la petición decodificado en Base64: ", source_ip_decoded)
            print()
        except Exception as e:
            print("Error al decodificar el 'source_ip' en Base64: ", e)
    else:
        print("No se recibió ningún 'source_ip' en la petición")


    # Se extrae el HOST de la petición
    dns_record = DNSRecord.parse(message_decoded)
    host = str(dns_record.q.qname)
    
    # Conversion del HOST a formato Firebase
    parts = host.split(".")
    reversed_parts = reversed(parts)
    host_firebase = "/".join(reversed_parts)
    #print("HOST Firebase ", host_firebase)

    # Conexión con Firebase
    ref1 = db.reference(host_firebase, app=firebase_admin.get_app(name='app1'))
    query = ref1.get()

    print()
    if query:
        print("El registro '", host_firebase, "' existe en Firebase.")
        print()

        # --- Type ---
        type = query.get("type")

        # SINGLE
        if (type == "single"):
            
            typeSingle(query, message_decoded)

        # MULTI
        elif (type == "multi"):

            typeMulti(query, message_decoded)

        # WEIGHT
        elif (type == "weight"):

            typeWeight(query, message_decoded)

        # GEO
        elif (type == "geo"):

            typeGeo(query, message_decoded, source_ip_decoded)

        else:
            print("Type inesperado.")

    else:
        print("El registro no existe.")

        # Se le envia "No Existe" al Interceptor
        send_interceptor("No Existe")

    return ""

# Type SINGLE
def typeSingle(query, message_decoded):
    print("-- SINGLE --")
    print()

    ip = query.get("ip")

    print("IP seleccionado directamente para enviar al cliente: ", ip)

    # Se genera la respuesta que cumple con RFC2929 y RFC1035
    response = dns_response(message_decoded, ip)

    # Se envía la respuesta al Interceptor
    send_interceptor(response)

# Type MULTI
def typeMulti(query, message_decoded):
    print("-- MULTI --")
    print()

    # Round-robin para obtener el IP
    global multi_id

    ip_list = query.get("ips")

    ip_obj = ip_list[multi_id]
    ip = ip_obj.get("ip")
    multi_id = (multi_id + 1) % len(ip_list)

    print("IP seleccionado por 'round-robin' para enviar al cliente: ", ip)

    # Se genera la respuesta que cumple con RFC2929 y RFC1035
    response = dns_response(message_decoded, ip)

    # Se envía la respuesta al Interceptor
    send_interceptor(response)

# Type WEIGHT
def typeWeight(query, message_decoded):
    print("-- WEIGHT --")
    print()

    ip_list = query.get("ips")

    # Se escoge un IP random, los IP con mayor 'weight' tienen más probabilidad de ser escogidos
    num = random.randint(1, 100)
    total_weight = 0

    for obj in ip_list:
        total_weight += obj.get("weight")

        if total_weight >= num:
            ip = obj.get("ip")
            break

    print("IP seleccionado basado en el peso de los registros para enviar al cliente: ", ip)

    # Se genera la respuesta que cumple con RFC2929 y RFC1035
    response = dns_response(message_decoded, ip)

    # Se envía la respuesta al Interceptor
    send_interceptor(response)

# Type GEO
def typeGeo(query, message_decoded, source_ip):
    print("-- GEO --")
    print()

    # Conexión con Firebase de 'IP to Country'
    ref2 = db.reference(app=firebase_admin.get_app(name='app2'))
    data = ref2.get()

    #source_ip = "1.0.0.250"
    country = ""
    source_ip_obj = ipaddress.IPv4Address(source_ip)

    # Se busca el país al que pertenece el IP
    for entry in data:

        ip_start = entry.get('0_0_0_0')
        ip_end = entry.get('0_255_255_255')

        if ip_start is not None and ip_end is not None:
            ip_start_obj = ipaddress.IPv4Address(ip_start)
            ip_end_obj = ipaddress.IPv4Address(ip_end)

            if ip_start_obj <= source_ip_obj <= ip_end_obj:
                print(f"La dirección IP {source_ip} está dentro del rango [{ip_start}, {ip_end}]")
                country = entry.get('ZZ')
                print("COUNTRY: ", country)
                break

    # En caso de no encontrar el país correspondiente (o 'ZZ') devuelve 1.0.0.0
    ip = "1.0.0.0"
    if country != "":
        ip_list = query.get("ips")

        for ip_obj in ip_list:
            if country == ip_obj.get("country"):
                ip = ip_obj.get("ip")
                break

    print("El Source IP pertenece al pais: ", country)
    print("IP seleccionado basado en Source IP del cliente para enviar al cliente: ", ip)

    # Se genera la respuesta que cumple con RFC2929 y RFC1035
    response = dns_response(message_decoded, ip)

    # Se envía la respuesta al Interceptor
    send_interceptor(response)


# =========== Tools ===========

# Envia un mensaje codificado en Base64 al DNS Interceptor
def send_interceptor(respuesta_dns):
    if isinstance(respuesta_dns, str):
        respuesta_base64 = base64.b64encode(respuesta_dns.encode()).decode()
    else:
        respuesta_base64 = base64.b64encode(respuesta_dns).decode()
    print("Respuesta codificada en Base64: ", respuesta_base64)
    print()

    Interceptor_IP = "127.0.0.1"
    Interceptor_PORT = 53

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        # Envia los datos codificados en base64 al servidor C
        sock.sendto(respuesta_base64.encode(), (Interceptor_IP, Interceptor_PORT))
        print("Datos enviados correctamente a", Interceptor_IP, "en el puerto", Interceptor_PORT)
        #print(respuesta_base64)
    except socket.error as e:
        print("Error al enviar los datos:", e)
    finally:
        # Se cierra el socket
        sock.close()
    
# A partir de una petición DNS y un IP genera una respuesta DNS
def dns_response(data, ip):
    # Se copia el header de la petición
    dns_header = data[:2]

    # Se copia la pregunta de la petición
    dns_question = data[12:]

    # Se arma la respuesta DNS
    dns_response = dns_header + b"\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00"
    dns_response += dns_question
    dns_response += b"\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x1e\x00\x04"
    dns_response += socket.inet_aton(ip)

    print()
    print("Respuesta DNS generada manualmente: ", dns_response)

    return dns_response


# =========== MAIN ===========

def main():
    
    # Certificados de Firebase
    with open('../../../config/config.yaml', 'r') as file:
        config = yaml.safe_load(file)

    # Inicializa Firebase normal
    cred1 = credentials.Certificate(config['firebase']['app1']['credentials'])
    firebase_admin.initialize_app(cred1, {
        'databaseURL': config['firebase']['app1']['databaseURL']
    }, name='app1')

    # Inicializa Firebase de la base 'IP To Country'
    cred2 = credentials.Certificate(config['firebase']['app2']['credentials'])
    firebase_admin.initialize_app(cred2, {
        'databaseURL': config['firebase']['app2']['databaseURL']
    }, name='app2')
    

    app.run(debug=True, threaded=True)

main()