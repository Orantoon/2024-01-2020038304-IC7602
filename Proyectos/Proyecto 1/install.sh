#!/bin/bash
#cd "/mnt/d/Tareas David/TEC/Semestre 9/Redes/2023-02-2020038304-IC7602/Proyectos/Proyecto 1/"

#gcc app.c -o app -lcjson
#./app


# Carpeta del Proyecto

helm upgrade --install proxy proxy

sleep 60

helm upgrade --install servers servers