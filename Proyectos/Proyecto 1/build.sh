#!/bin/bash


# Carpeta docker

docker login

docker build -t orantoon/proyecto1-ic76020124 .

docker images

docker run -it --rm orantoon/proyecto1-ic76020124
docker exec -it proyecto1 bash

docker push orantoon/proyecto1-ic76020124