#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <curl/curl.h> // Peticion API
#include "b64/cencode.h" // Encode en base64
#include "b64/cdecode.h" // Decode en base64
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> // Sockets
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define API_IP "127.0.0.1"
#define API_PORT 8000
#define PORT 53 
#define MAXLINE 4096 


void queryStandard();
void notQueryStandard(int sockfd, struct sockaddr_in cliaddr, unsigned char buffer[MAXLINE], unsigned char bufferCoded[4096], int len, int numBytes);

void encodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer);
void decodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer);
void sendApi (char *url, char message[8192]);

int main() { 
        
        int sockfd; 
        unsigned char buffer[MAXLINE] = "";
        char bufferCoded[4096];

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
                bzero(bufferCoded, 4096);

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

                        queryStandard();

                } else {
                        
                        //notQueryStandard(sockfd, servaddr, cliaddr, buffer, len, numBytes);

                }

                notQueryStandard(sockfd, cliaddr, buffer, bufferCoded, len, numBytes);

                // break;
        }

        close(sockfd);

        return 0; 

}


void queryStandard(){

}


void notQueryStandard(int sockfd, struct sockaddr_in cliaddr, unsigned char buffer[MAXLINE], unsigned char bufferCoded[4096], int len, int numBytes){

        // Archivos para guardar el primer request
        FILE* log_file, *log_file2; 

        // Se guarda todo el buffer en el log2.txt
        log_file = fopen("log2.txt", "wb");
        //fprintf (log_file, "%s", buffer);
        fwrite(buffer, 1, numBytes, log_file);
        printf("Log 2 guardado\n");

        // Codificacion en base64
        encodeBase64(buffer, numBytes, bufferCoded);
        
        printf("BASE64 : %s\n", bufferCoded);

        // Se guarda el buffer codificado en base64
        log_file2 = fopen("log3.txt", "wb");
        fwrite(bufferCoded, 1, numBytes, log_file2);
        printf("Log 3 guardado\n");


        // Se envia la peticion HTTP POST a /api/dns_resolver
        sendApi ("http://localhost:5000/api/dns_resolver", bufferCoded);

        printf("Solicitud HTTP POST enviada con éxito al DNS API.\n");

        // Se espera la respuesta del servidor
        unsigned char responseBuffer[MAXLINE];
        int bytesRead = recv(sockfd, responseBuffer, MAXLINE, 0);

        if (bytesRead < 0) {
                // Manejo de error en la recepción
                perror("Error al recibir datos del servidor");
        } else if (bytesRead == 0) {
                // El servidor cerró la conexión
                printf("El servidor cerró la conexión.\n");
        }
        // Procesamiento de la respuesta recibida
        printf("Respuesta recibida del servidor: %s\n", responseBuffer);
        
        unsigned char finalResponse[MAXLINE];
        
        // Decodificacion en base64
        decodeBase64(responseBuffer, MAXLINE, finalResponse);
        printf("Respuesta final: %s\n", finalResponse);

        sendto(sockfd, finalResponse, strlen(finalResponse), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);

        fclose(log_file);
        fclose(log_file2);
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
        int retlen = base64_decode_block((const char *)inputBuffer, inputSize, (char *)outputBuffer, &state);        //printf("RETLEN: %d \n", retlen);

        outputBuffer[retlen] = '\0';
}

void sendApi(char *url, char message[8192]) {
        CURL *curl;
        CURLcode res;

        // Inicializa la biblioteca libcurl
        curl_global_init(CURL_GLOBAL_ALL);

        // Crea una instancia de CURL
        curl = curl_easy_init();
        if(curl) {
                // Establece la URL
                curl_easy_setopt(curl, CURLOPT_URL, url);

                // Establece el método HTTP POST
                curl_easy_setopt(curl, CURLOPT_POST, 1L);

                // Habilitar la depuración en libcurl
                //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

                // Establece el cuerpo de la petición y su longitud
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);

                // Realiza la petición HTTP
                res = curl_easy_perform(curl);
                if(res != CURLE_OK)
                        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

                // Finaliza el proceso
                curl_easy_cleanup(curl);
        }

        // Finaliza la biblioteca libcurl
        curl_global_cleanup();
}
