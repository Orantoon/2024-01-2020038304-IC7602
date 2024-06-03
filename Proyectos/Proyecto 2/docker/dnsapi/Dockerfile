FROM python:3.9-slim

# Configura el directorio de trabajo y copia la aplicación en el contenedor
WORKDIR /usr/src/app
COPY app/ .

# Actualiza ubuntu
RUN apt-get update

# Instala las librerias de 'requirements.txt'
COPY app/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Se crea la variable de entorno LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH /usr/local/lib

# Mantiene la imagen en ejecución
CMD ["python3 dns_api.py"]
