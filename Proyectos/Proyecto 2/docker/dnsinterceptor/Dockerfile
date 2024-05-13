FROM ubuntu:latest

# Configura el directorio de trabajo y copia la aplicación en el contenedor
WORKDIR /usr/src/app
COPY app/ .

# Actualiza ubuntu
RUN apt-get update

# Instala las librerias gcc, git, cmake y libb64
RUN apt-get install build-essential -y
RUN apt-get install git -y
RUN apt-get install cmake -y
RUN apt-get install libb64-dev -y

# Se crea la variable de entorno LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH /usr/local/lib

# Compilar la aplicación
RUN gcc app.c -o app -lb64

# Permisos y ejecución de la aplicacion app.c

#RUN chmod +x app
#RUN /usr/src/app/app

# Mantiene la imagen en ejecución
CMD [ "tail", "-f", "/dev/null"]
