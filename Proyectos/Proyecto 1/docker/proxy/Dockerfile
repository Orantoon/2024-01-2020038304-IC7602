FROM ubuntu:latest

# Configurar el directorio de trabajo y copiar la aplicación
WORKDIR /usr/src/myapp
COPY app/ .

# Actualizar el índice de paquetes
RUN apt-get update

# Instalación de dependencias
RUN apt-get install build-essential -y
RUN apt-get install git -y
RUN apt-get install cmake -y

# Clonar e instalar cJSON
RUN git clone https://github.com/DaveGamble/cJSON.git /usr/src/cJSON && \
    cd /usr/src/cJSON && \
    cmake . && \
    make && \
    make install

# Crear la variable de entorno LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH /usr/local/lib

# Compilar la aplicación
RUN gcc -o myapp app.c -I/usr/local/include -L/usr/local/lib -lcjson

# Permisos y ejecución de la aplicacion app.c

#RUN chmod +x myapp
#RUN /usr/src/myapp/myapp

# Ejecutar la imagen
CMD [ "tail", "-f", "/dev/null"]
