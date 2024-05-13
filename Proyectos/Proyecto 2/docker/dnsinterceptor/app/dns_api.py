from flask import Flask, request
import base64

# Crea una instancia de la aplicación Flask
app = Flask(__name__)

@app.route('/api/dns_resolver', methods=['POST'])
def dnsResolver():
    print("DNS RESOLVER")

    content = request.get_data(as_text=True)
    print("Contenido de la petición:", content)

    contentDecoded = base64.b64decode(content)
    print("Contenido de la petición decodificado:", contentDecoded)

    return "DNS Resolver"

@app.route('/api/exists', methods=['GET'])
def exists():
    print("EXISTS")
    return "Exists"

def main():
    app.run(debug=True)

main()