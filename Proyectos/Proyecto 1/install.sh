#!/bin/bash
#cd "/mnt/d/Tareas David/TEC/Semestre 9/Redes/2023-02-2020038304-IC7602/Proyectos/Proyecto 1/"

# Correr localmente
#gcc app.c -o app -lcjson
#./app

# Correr Proxy
#kubectl exec -it proxy-c7f8f5fd4-lchsh -- /bin/bash
#./myapp
#telnet proxy-c7f8f5fd4-lchsh 8080

# Correr Server
#kubectl exec -it apachetcp-794f7bdf69-nl47w -- /bin/bash
#cat logs/access_log


# Carpeta del Proyecto

helm upgrade --install servers servers

sleep 60

helm upgrade --install proxy proxy