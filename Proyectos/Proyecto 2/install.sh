#!/bin/bash
#cd "/mnt/d/Tareas David/TEC/Semestre 9/Redes/2023-02-2020038304-IC7602/Proyectos/Proyecto 2/"

# Correr Interceptor localmente
#gcc dns_interceptor.c -o app -lb64 -lcurl
#sudo ./app

# Correr API localmente
#python3 dns_api.py

# NSLOOKUP
# set timeout=1000
# server 127.0.0.1
# www.google.com
# mail.google.com
# meet.google.com
# workspace.google.com
# hola.google.com


# Carpeta del Proyecto

helm upgrade --install dnsinterceptor dnsinterceptor