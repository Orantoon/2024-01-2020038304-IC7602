FROM debian:buster

# Instalación de git, cmake y GCC
RUN apt-get update && \
    apt-get install -y git cmake gcc

# Se clona el repositorio de cJSON y se compila como parte de la aplicación
RUN git clone https://github.com/DaveGamble/cJSON.git /usr/src/cJSON && \
    cd /usr/src/cJSON && \
    cmake . && \
    make && \
    make install


# Se crea la variable de entorno LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH /usr/local/lib

# Se crea el directorio de trabajo
WORKDIR /usr/src/myapp

# Se copia la carpeta app en el contenedor
COPY app/ /usr/src/myapp/

# Se compila el código de app.c
RUN gcc -o myapp app.c -I/usr/local/include -L/usr/local/lib -lcjson

# Se corre la aplicación (CMD)
CMD ["./myapp"]
