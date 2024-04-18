#!/bin/bash

# Carga las variables de entorno desde el archivo variables.env
source /usr/src/myapp/variables.env

# Ejecuta el comando CMD (tu aplicación)
exec "$@"