#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <stdbool.h>
#include <curl/curl.h> // Peticion API
#include "b64/cencode.h" // Encode en base64
#include "b64/cdecode.h" // Decode en base64
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> // Sockets
#include <arpa/inet.h> 
#include <arpa/nameser.h>
#include <resolv.h>
#include <netinet/in.h> 

#define API_IP "127.0.0.1"
#define API_PORT 8000
#define PORT 53 
#define MAXLINE 4096 


void queryStandard(int sockfd, struct sockaddr_in cliaddr, unsigned char buffer[MAXLINE], int len, int numBytes);

void notQueryStandard(int sockfd, struct sockaddr_in cliaddr, unsigned char buffer[MAXLINE], int len, int numBytes);

void encodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer);
void decodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer);
void sendApi (char *urlBase, char message[MAXLINE], bool post, char sourceIP[MAXLINE]);
void recvApi(unsigned char *responseBuffer, int sockfd);

int main() { 
        
        int sockfd; 
        unsigned char buffer[MAXLINE] = "";

        struct sockaddr_in servaddr, cliaddr; 

        // Creacion del socket
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
                perror("Creacion del socket fallo."); 
                exit(EXIT_FAILURE); 
        } 

        // Información del Cliente
        memset(&cliaddr, 0, sizeof(cliaddr));
        cliaddr.sin_family    = AF_INET; // IPv4 
        cliaddr.sin_addr.s_addr = INADDR_ANY; 
        cliaddr.sin_port = htons(PORT);


        int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
                perror("setsockopt(SO_REUSEADDR) fallo.");
                exit(EXIT_FAILURE);
        }

        // Bind del socket al Cliente 
        if ( bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 ){ 
                perror("El enlace del socket fallo."); 
                exit(EXIT_FAILURE); 
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(API_IP);
        servaddr.sin_port = htons(API_PORT);

        int len, numBytes; 
        len = sizeof(cliaddr);  //len is value/result 

        while (1){

                printf("\n\n");
                printf("------------------- \n");
                printf("Listening... \n");

                bzero(buffer, MAXLINE);

                numBytes = recvfrom(sockfd, &buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
                if (numBytes == -1) {
                        perror("Hubo un error en la recepcion de datos.");
                        exit(EXIT_FAILURE);
                }

                printf("Largo del buffer: %ld \n", strlen(buffer));

                // QR
                unsigned int qr = (unsigned int) buffer[2];
                qr = qr << 24;
                qr = qr >> 31;
                printf("QR: %u \n", qr);

                // OPCODE
                unsigned int opcode = (unsigned int) buffer[2];
                opcode = opcode << 25;
                opcode = opcode >> 28;
                printf("OPCODE: %u \n\n", opcode);

                if (qr == 0 && opcode == 0){

                        printf("PETICION STANDARD \n\n");

                        queryStandard(sockfd, cliaddr, buffer, len, numBytes);

                } else {

                        printf("PETICION NO STANDARD \n\n");
                        
                        notQueryStandard(sockfd, cliaddr, buffer, len, numBytes);

                }

                //notQueryStandard(sockfd, cliaddr, buffer, len, numBytes);
        }

        close(sockfd);

        return 0; 

}


void queryStandard(int sockfd, struct sockaddr_in cliaddr, unsigned char buffer[MAXLINE], int len, int numBytes){
        char bufferCoded[MAXLINE];
        bzero(bufferCoded, MAXLINE);

        // Se extrae el Source IP del cliente
        char clientIP[INET_ADDRSTRLEN];
        char sourceIP[MAXLINE];
        inet_ntop(AF_INET, &cliaddr.sin_addr, clientIP, INET_ADDRSTRLEN);

        // Codificacion en base64
        encodeBase64(buffer, numBytes, bufferCoded);
        encodeBase64(clientIP, sizeof(clientIP), sourceIP);
        printf("BASE64 Buffer: %s\n", bufferCoded);
        printf("BASE64 Source IP: %s\n", sourceIP);

        // Se envia la peticion HTTP GET a /api/exists
        sendApi ("http://localhost:5000/api/exists", bufferCoded, false, sourceIP);

        printf("Solicitud HTTP GET enviada con éxito al DNS API.\n");

        // Se espera la respuesta del API
        unsigned char responseBuffer[MAXLINE];
        recvApi(responseBuffer, sockfd);

        // Decodificacion en base64
        unsigned char finalResponse[MAXLINE] = "";
        decodeBase64(responseBuffer, MAXLINE, finalResponse);
        
        printf("Respuesta final al cliente: %s\n", finalResponse);

        if (strcmp(finalResponse, "No Existe") == 0){
                bzero(bufferCoded, MAXLINE);
                notQueryStandard(sockfd, cliaddr, buffer, len, numBytes);
        }

        sendto(sockfd, finalResponse, sizeof(finalResponse), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
}

void notQueryStandard(int sockfd, struct sockaddr_in cliaddr, unsigned char buffer[MAXLINE], int len, int numBytes){
        char bufferCoded[MAXLINE];
        bzero(bufferCoded, MAXLINE);

        // Codificacion en base64
        encodeBase64(buffer, numBytes, bufferCoded);
        printf("BASE64 : %s\n", bufferCoded);

        // Se envia la peticion HTTP POST a /api/dns_resolver
        sendApi ("http://localhost:5000/api/dns_resolver", bufferCoded, true, NULL);

        printf("Solicitud HTTP POST enviada con éxito al DNS API.\n");

        // Se espera la respuesta del API
        unsigned char responseBuffer[MAXLINE];
        recvApi(responseBuffer, sockfd);
        
        // Decodificacion en base64
        unsigned char finalResponse[MAXLINE] = "";
        decodeBase64(responseBuffer, MAXLINE, finalResponse);
        
        printf("Respuesta final al cliente: %s\n", finalResponse);

        sendto(sockfd, finalResponse, sizeof(finalResponse), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
}

void encodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer) {
        // Se inicializa el encode base64
        base64_encodestate state;
        base64_init_encodestate(&state);

        // Se codifica el inputBuffer en base64
        int retlen = base64_encode_block(inputBuffer, inputSize, outputBuffer, &state);
        int retlen2 = base64_encode_blockend(outputBuffer + retlen, &state);
        //printf("RETLEN: %d \n", retlen);
        //printf("RETLEN2: %d \n", retlen2);

        outputBuffer[retlen + retlen2] = '\0';
}

void decodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer) {
        // Se inicializa el decode base64
        base64_decodestate state;
        base64_init_decodestate(&state);

        // Se decodifica el inputBuffer en base64
        int retlen = base64_decode_block(inputBuffer, inputSize, outputBuffer, &state);

        outputBuffer[retlen] = '\0';
}

void sendApi(char *urlBase, char message[MAXLINE], bool post, char sourceIP[MAXLINE]) {
    CURL *curl;
    CURLcode res;

    char url[MAXLINE];

    // Inicializa la biblioteca libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    // Crea una instancia de CURL
    curl = curl_easy_init();
    if (curl) {
        if (post) {
            // Para solicitudes POST, simplemente usamos la URL base
            strcpy(url, urlBase);
        } else {
            // Para solicitudes GET, construimos la URL con los parámetros codificados
            char *encoded_message = curl_easy_escape(curl, message, strlen(message));
            char *encoded_sourceIP = curl_easy_escape(curl, sourceIP, strlen(sourceIP));
            snprintf(url, sizeof(url), "%s?message=%s&source_ip=%s", urlBase, encoded_message, encoded_sourceIP);
            curl_free(encoded_message);
            curl_free(encoded_sourceIP);
        }

        printf("URL: %s\n", url);

        // Establece la URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        if (post) {
            // Establece el método HTTP POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // Establece el cuerpo de la petición y su longitud
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
        } else {
            // Establece el método HTTP GET
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        // Realiza la petición HTTP
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Finaliza el proceso
        curl_easy_cleanup(curl);
    }

    // Finaliza la biblioteca libcurl
    curl_global_cleanup();
}

void recvApi(unsigned char *responseBuffer, int sockfd){
        int bytesRead = recv(sockfd, responseBuffer, MAXLINE, 0);

        if (bytesRead < 0) {
                // Manejo de error en la recepción
                perror("Error al recibir datos del servidor");
                exit(EXIT_FAILURE);
        } else if (bytesRead == 0) {
                // El servidor cerró la conexión
                perror("El servidor cerró la conexión.\n");
                exit(EXIT_FAILURE);
        }

        responseBuffer[bytesRead] = '\0';

        printf("Respuesta recibida del servidor: %s\n", responseBuffer);
}