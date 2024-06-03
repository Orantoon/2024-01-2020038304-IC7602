FROM ubuntu:latest

# Configura el directorio de trabajo y copia la aplicación en el contenedor
WORKDIR /usr/src/app
COPY app/ .

# Actualiza ubuntu
RUN apt-get update

# Instala las librerías gcc, git, cmake, libb64, y libcurl
RUN apt-get install build-essential -y
RUN apt-get install git -y
RUN apt-get install cmake -y
RUN apt-get install libb64-dev -y
RUN apt-get install libcurl4-openssl-dev -y

# Se crea la variable de entorno LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH /usr/local/lib

# Compilar la aplicación
RUN gcc server.c -o app 

# Permisos y ejecución de la aplicacion app.c
RUN chmod +x app

# Mantiene la imagen en ejecución
CMD [ "./app"]
