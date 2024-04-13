FROM debian:buster

# Install necessary dependencies to download and compile cJSON library
RUN apt-get update && \
    apt-get install -y git cmake gcc

# Clone cJSON repository and build
RUN git clone https://github.com/DaveGamble/cJSON.git /usr/src/cJSON && \
    cd /usr/src/cJSON && \
    cmake . && \
    make && \
    make install

# Set up environment variables for cJSON library
ENV LD_LIBRARY_PATH /usr/local/lib

# Copy your application code
COPY app/. /usr/src/myapp

# Set working directory
WORKDIR /usr/src/myapp

# Compile your application with cJSON library
RUN gcc -o myapp app.c -I/usr/local/include -L/usr/local/lib -lcjson

# Run your application
CMD ["./myapp"]
