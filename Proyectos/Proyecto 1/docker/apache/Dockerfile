FROM httpd:2.4


# Install envsubst
RUN apt-get update && apt-get install -y gettext-base

COPY app/docker-entrypoint.sh .
RUN mkdir -p /usr/local/apache2/tmp_files
COPY ./app/ /usr/local/apache2/tmp_files

# Calls script to substitute environment variables in the HTML template

CMD ["sh", "docker-entrypoint.sh"]

# TO RUN docker run --name ServidorB -d  -p 8060:80 -e SERVER_NAME="Richard's Server" apache_server 