#!/bin/bash


# Carpeta docker

docker login


# Carpeta docker

docker build -t orantoon/tareacorta2-ic76020124 .

#docker images

#docker run -it --rm orantoon/tareacorta2-ic76020124
#docker exec -it tareacorta2 bash

docker push orantoon/tareacorta2-ic76020124