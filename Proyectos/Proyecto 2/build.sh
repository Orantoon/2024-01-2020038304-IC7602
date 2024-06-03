#!/bin/bash


# Carpeta docker

docker login


# ========== DNS Interceptor ==========

# Carpeta docker/dnsinterceptor

docker build -t orantoon/dnsinterceptor-ic76020124 .

#docker images

#docker run -it --rm orantoon/dnsinterceptor-ic76020124
#docker exec -it dnsinterceptor bash

docker push orantoon/dnsinterceptor-ic76020124


# Carpeta docker/dnsapi

docker build -t orantoon/dnsapi-ic76020124 .

#docker images

#docker run -it --rm orantoon/dnsapi-ic76020124
#docker exec -it dnsapi bash

docker push orantoon/dnsapi-ic76020124