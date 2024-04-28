#!/bin/bash

mkdir -p /usr/local/apache2/htdocs/$SERVER_NAME
envsubst < /usr/local/apache2/tmp_files/index.html.template > /usr/local/apache2/htdocs/index.html 
cp /usr/local/apache2/htdocs/index.html /usr/local/apache2/htdocs/$SERVER_NAME/index.html
exec httpd-foreground