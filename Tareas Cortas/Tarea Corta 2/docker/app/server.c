#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9666
#define MAXLINE 4096

void getBroadcast(int new_socket, char buffer[MAXLINE], char final_response[MAXLINE]);
void getNetworkNumber(int new_socket, char buffer[MAXLINE], char final_response[MAXLINE]);
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

    printf("Conexión aceptada\n\n");

    char final_response[MAXLINE];
    while (1){
        send(new_socket, "\nEscriba una solicitud, o EXIT para salir: ", strlen("\nEscriba una solicitud, o EXIT para salir: "), 0);

        int valread = read(new_socket, buffer, MAXLINE);
        if (valread < 0) {
            perror("read");
        } else {
            printf("Mensaje recibido: %s\n", buffer);

            // GET BROADCAST
            if (strstr(buffer, "GET BROADCAST") != NULL) {
                getBroadcast(new_socket, buffer, final_response);
            } else 
            // GET NETWORK NUMBER
            if (strstr(buffer, "GET NETWORK NUMBER") != NULL) {
                getNetworkNumber(new_socket, buffer, final_response);
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
                printf("Solicitud innesperada.\n\n");
                send(new_socket, "Error: solicitud innesperada, intente otra vez.\n", strlen("Error: solicitud innesperada, intente otra vez.\n"), 0);
                continue;
            }

            send(new_socket, final_response, strlen(final_response), 0);
            printf("Respuesta %s enviada\n\n", final_response);
            continue;
        }
    }

    // Cerrar el socket
    close(new_socket);
    close(server_fd);
    return 0;

}



void getBroadcast(int new_socket, char buffer[MAXLINE], char final_response[MAXLINE]) {
    char ip_str[16] = "";
    char mask_str[16] = "";

    // Extrae el IP y MASK del buffer
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        if (strcmp(token, "IP") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                strncpy(ip_str, token, sizeof(ip_str) - 1);
                ip_str[sizeof(ip_str) - 1] = '\0';
            }
        } else if (strcmp(token, "MASK") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                strncpy(mask_str, token, sizeof(mask_str) - 1);
                mask_str[sizeof(mask_str) - 1] = '\0';
            }
        }
        token = strtok(NULL, " ");
    }

    // En caso de que no exista algun elemento da error
    if (strlen(ip_str) <= 0 || strlen(mask_str) <= 0){
        printf("IP o MASK invalidos.\n");
        send(new_socket, "Error: IP o MASK invalidos.\n\n", strlen("Error: IP o MASK invalidos.\n\n"), 0);
        return;
    }

    printf("IP: %s\n", ip_str);
    printf("MASK: %s\n", mask_str);

    // Se pasa el IP de string a unit32_t
    uint32_t ip;
    inet_pton(AF_INET, ip_str, &ip);
    ip = ntohl(ip);
    

    // Se pasa el MASK de string a unit32_t
    int mask_num = 0;
    uint32_t mask;
    if (mask_str[0] == '/'){
        mask_num = atoi(&mask_str[1]);

        // En caso de estar en el formato /n
        mask = htonl(~((1 << (32 - mask_num)) - 1));
    } else {
        // En caso de estar en el formato X.X.X.X
        inet_pton(AF_INET, mask_str, &mask);
    }
    mask = ntohl(mask);

    // --- OPERACIONES BITWISE ---

    // Se calcula el Network IP con la operacion bitwise "IP AND MASK"
    uint32_t network = ip & mask;

    // Se calcula el Broadcast IP con la operacion bitwise "NETWORK AND NOT MASK"
    uint32_t broadcast = network | ~mask;

    // ---------------------------

    struct in_addr ip_addr;
    ip_addr.s_addr = htonl(broadcast);
    inet_ntop(AF_INET, &ip_addr, final_response, INET_ADDRSTRLEN);

    printf("Broadcast: %s\n", final_response);
}


void getNetworkNumber(int new_socket, char buffer[MAXLINE], char final_response[MAXLINE]) {
    char ip_str[16] = "";
    char mask_str[16] = "";

    // Extrae el IP y MASK del buffer
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        if (strcmp(token, "IP") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                strncpy(ip_str, token, sizeof(ip_str) - 1);
                ip_str[sizeof(ip_str) - 1] = '\0';
            }
        } else if (strcmp(token, "MASK") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                strncpy(mask_str, token, sizeof(mask_str) - 1);
                mask_str[sizeof(mask_str) - 1] = '\0';
            }
        }
        token = strtok(NULL, " ");
    }

    // En caso de que no exista algun elemento da error
    if (strlen(ip_str) <= 0 || strlen(mask_str) <= 0){
        printf("IP o MASK invalidos.\n");
        send(new_socket, "Error: IP o MASK invalidos.\n\n", strlen("Error: IP o MASK invalidos.\n\n"), 0);
        return;
    }

    printf("IP: %s\n", ip_str);
    printf("MASK: %s\n", mask_str);

    // Se pasa el IP de string a unit32_t
    uint32_t ip;
    inet_pton(AF_INET, ip_str, &ip);
    ip = ntohl(ip);
    

    // Se pasa el MASK de string a unit32_t
    int mask_num = 0;
    uint32_t mask;
    if (mask_str[0] == '/'){
        mask_num = atoi(&mask_str[1]);

        // En caso de estar en el formato /n
        mask = htonl(~((1 << (32 - mask_num)) - 1));
    } else {
        // En caso de estar en el formato X.X.X.X
        inet_pton(AF_INET, mask_str, &mask);
    }
    mask = ntohl(mask);

    // --- OPERACIONES BITWISE ---

    // Se calcula el Network IP con la operacion bitwise "IP AND MASK"
    uint32_t network = ip & mask;

    // ---------------------------

    struct in_addr ip_addr;
    ip_addr.s_addr = htonl(network);
    inet_ntop(AF_INET, &ip_addr, final_response, INET_ADDRSTRLEN);

    printf("Network: %s\n", final_response);
}

void getHostsRange(char buffer[MAXLINE]) {
    printf("getHostsRange\n");
}

void getRandomSubnetsNetwork(char buffer[MAXLINE]){
    printf("getRandomSubnetsNetwork\n");
}