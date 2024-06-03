#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9666
#define MAXLINE 4096

void getBroadcast(char buffer[MAXLINE]);
void getNetworkNumber(char buffer[MAXLINE]);
void getHostsRange(char buffer[MAXLINE]);
void getRandomSubnetsNetwork(char buffer[MAXLINE]);

int main() {

    int server_fd, new_socket;
    struct sockaddr_in address;

    int addrlen = sizeof(address);

    char buffer[MAXLINE] = {0};
    int opt = 1;

    // Se crea el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configuraciones del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configuracion del puerto
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind del socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escucha nuevas conexiones
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Esperando conexiones en el puerto %d...\n", PORT);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Conexión aceptada\n");

    while (1){
        send(new_socket, "\nEscriba una solicitud, o EXIT para salir: ", strlen("\nEscriba una solicitud, o EXIT para salir: "), 0);

        int valread = read(new_socket, buffer, MAXLINE);
        if (valread < 0) {
            perror("read");
        } else {
            printf("Mensaje recibido: %s\n", buffer);

            // GET BROADCAST
            if (strstr(buffer, "GET BROADCAST") != NULL) {
                getBroadcast(buffer);
            } else 
            // GET NETWORK NUMBER
            if (strstr(buffer, "GET NETWORK NUMBER") != NULL) {
                getNetworkNumber(buffer);
            } else
            // GET HOSTS RANGE
            if (strstr(buffer, "GET HOSTS RANGE") != NULL) {
                getHostsRange(buffer);
            } else
            // GET RANDOM SUBNETS NETWORK
            if (strstr(buffer, "GET RANDOM SUBNETS NETWORK") != NULL) {
                getRandomSubnetsNetwork(buffer);
            } else
            if (strstr(buffer, "EXIT") != NULL) {
                break;
            }
            else {
                printf("Solicitud innesperada.\n");
                send(new_socket, "Error: solicitud innesperada, intente otra vez.\n", strlen("Error: solicitud innesperada, intente otra vez.\n"), 0);
                continue;
            }

            char *response = "Mensaje recibido con éxito\n";
            send(new_socket, response, strlen(response), 0);
            printf("Respuesta enviada\n");
            continue;
        }
    }

    // Cerrar el socket
    close(new_socket);
    close(server_fd);
    return 0;

}



void getBroadcast(char buffer[MAXLINE]) {
    printf("getBroadcast\n");
}

void getNetworkNumber(char buffer[MAXLINE]) {
    printf("getNetworkNumber\n");
}

void getHostsRange(char buffer[MAXLINE]) {
    printf("getHostsRange\n");
}

void getRandomSubnetsNetwork(char buffer[MAXLINE]){
    printf("getRandomSubnetsNetwork\n");
}