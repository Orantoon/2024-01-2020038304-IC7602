FROM httpd:2.4


# Install envsubst
RUN apt-get update && apt-get install -y gettext-base

COPY app/docker-entrypoint.sh .
RUN mkdir -p /usr/local/apache2/tmp_files
COPY ./app/ /usr/local/apache2/tmp_files

COPY custom.conf /usr/local/apache2/conf/custom.conf
RUN echo "Include /usr/local/apache2/conf/custom.conf" >> /usr/local/apache2/conf/httpd.conf

# Calls script to substitute environment variables in the HTML template

CMD ["sh", "docker-entrypoint.sh"]
