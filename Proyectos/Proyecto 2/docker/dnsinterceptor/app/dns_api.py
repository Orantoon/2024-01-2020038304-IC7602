from flask import Flask, request
import socket
import base64

# Crea una instancia de la aplicación Flask
app = Flask(__name__)

@app.route('/api/dns_resolver', methods=['POST'])
def dns_resolver_router():
    print("DNS RESOLVER")

    content = request.get_data(as_text=True)
    print("Contenido de la petición:", content)

    content_decoded = base64.b64decode(content)
    print("Contenido de la petición decodificado:", content_decoded)

    # Se envia la peticion al DNS Remoto y se recibe una respuesta
    respuesta_dns = dns_resolver_remote(content_decoded, "8.8.8.8")

    send_interceptor(respuesta_dns)

    return "DNS Resolver"

def dns_resolver_remote(content_decoded, servidor_dns):
    cliente_dns = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    cliente_dns.settimeout(30)

    try:
        # Envia peticion al DNS Remoto
        cliente_dns.sendto(content_decoded, (servidor_dns, 53))
        
        # Recibe la respuesta del servidor
        respuesta, _ = cliente_dns.recvfrom(1024)
        
        # Se imprime la respuesta recibida
        print("Respuesta del servidor DNS:")
        print(respuesta)
    
    except socket.timeout:
        print("Tiempo de espera agotado. No se recibió ninguna respuesta del servidor DNS.")
    
    finally:
        # Se cierra el socket
        cliente_dns.close()

    return respuesta

def send_interceptor(respuesta_dns):
    print("NORMAL ", respuesta_dns)
    respuesta_bytes = bytes(respuesta_dns)

    respuesta_base64 = base64.b64encode(respuesta_dns).decode()

    print("ENCODE ", respuesta_base64)

    respuesta_base64_2 = base64.b64encode(respuesta_bytes).decode()

    print("ENCODE 2", respuesta_base64_2)


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
    

@app.route('/api/exists', methods=['GET'])
def exists():
    print("EXISTS")
    return "Exists"

def main():
    app.run(debug=True)

main()