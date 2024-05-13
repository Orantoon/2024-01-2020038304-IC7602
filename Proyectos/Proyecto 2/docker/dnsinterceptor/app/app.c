#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include "b64/cencode.h"
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define API_IP "127.0.0.1"
#define API_PORT 8000
#define PORT 53 
#define MAXLINE 4096 


void queryStandard();
void notQueryStandard(int sockfd, struct sockaddr_in servaddr, unsigned char buffer[MAXLINE], unsigned char bufferCoded[4096], int len, int numBytes);

void encodeBase64(const unsigned char *inputBuffer, int inputSize, unsigned char *outputBuffer);
void sendApi (int sockfd, char message[8192], struct sockaddr_in servaddr);

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

                notQueryStandard(sockfd, servaddr, buffer, bufferCoded, len, numBytes);

                // break;
        }

        close(sockfd);

        return 0; 

}


void queryStandard(){

}


void notQueryStandard(int sockfd, struct sockaddr_in servaddr, unsigned char buffer[MAXLINE], unsigned char bufferCoded[4096], int len, int numBytes){

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
        char postRequest[8192];

        char *request = "POST /api/dns_resolver HTTP/1.1\r\n"
                "Host: %s\r\n"  
                "Content-Type: application/json\r\n"
                "Content-Length: %ld\r\n"
                "\r\n"
                "%s";

        sprintf(postRequest, request, API_IP, strlen(bufferCoded), bufferCoded);

        sendApi (sockfd, postRequest, servaddr);

        printf("Solicitud HTTP POST enviada con éxito al DNS API.\n");


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

void sendApi (int sockfd, char message[8192], struct sockaddr_in servaddr){
        if (sendto(sockfd, message, strlen(message), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
                perror("Algo fallo al hacer un envio de datos.");
                exit(EXIT_FAILURE);
        }
}