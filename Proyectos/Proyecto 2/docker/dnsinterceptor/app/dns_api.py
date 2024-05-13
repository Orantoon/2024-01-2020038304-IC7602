import socket

HOST = '127.0.0.1'
PORT = 8000

def main():
    # Se crea un socket UDP
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        # Se enlaza el socket al ip y puerto
        s.bind((HOST, PORT))

        print(f"Escuchando en {PORT}...")

        # Se reciben datos
        while True:
            # Se espera a que se reciban datos
            recv, addr = s.recvfrom(8192)
            data = recv.decode()
            print('Datos recibidos de', addr, ':')
            print(data)

            dataLines = data.split('\n')

            # Se lee el tipo de peticion
            request = dataLines[0]
            print("Tipo de Peticion:", request)

            # Se lee el cuerpo de la peticion
            body = ""

            bodyIndex = data.find("\n\n")

            if bodyIndex == -1:
                bodyIndex = data.find("\r\n\r\n")
            if bodyIndex != -1:
                body = data[bodyIndex+2:]

            body = body.strip()
                
            print("Cuerpo de la Peticion:", body)

            # Tipo de peticion POST /api/dns_resolver HTTP/1.1
            if request == "POST /api/dns_resolver HTTP/1.1":
                print("PETICION DNS RESOLVER")
            if request == "GET /api/exists HTTP/1.1":
                print("PETICION EXISTS")


main()