#!/bin/bash


# Carpeta docker

docker login

docker build -t orantoon/proyecto1-ic76020124 .

docker images

docker run -it --rm --name proyecto1 orantoon/proyecto1-ic76020124
docker push orantoon/proyecto1-ic76020124