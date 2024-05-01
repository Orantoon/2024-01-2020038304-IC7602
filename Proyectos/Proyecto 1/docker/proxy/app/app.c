#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>


struct Server {
    char ip[16];
    int port;
    int weight;
};

int modeTCP (cJSON *json);
int modeUDP (cJSON *json);
int modeHTTP (cJSON *json);

int proxyTcp(int port, struct Server *servers, int numServers);
int proxyUdp(const char *port, const char *serverIp, const char *serverPort, const char *weight);
int proxyHttp(const char *port, const char *serverIp, const char *serverPort, const char *weight, const char *type, const char *path);



// ============================================ MAIN ============================================

int main(int argc, char *argv[]) {

    // Abrir archivo "config.json"
    FILE *fp = fopen("config.json", "r");
    if (fp == NULL) {
        printf("Error: No se pudo abrir el archivo.\n");
        return 1;
    }

    // Guardar contenido de "config.json" en un buffer
    char buffer[4000];
    int len = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    // Parsear buffer
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) { 
        const char *error_ptr = cJSON_GetErrorPtr(); 
        if (error_ptr != NULL) { 
            printf("Error: %s\n", error_ptr); 
        } 
        printf("Error: Ocurrió un error al parsear el archivo de configuración.\n");
        cJSON_Delete(json);
        return 1; 
    }

    // ----------------------------------------

    // Dependiendo del mode, la funcion que se utiliza cambia
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(json, "mode"); 

    if (strcmp(mode->valuestring, "tcp") == 0)
    {
        printf("Mode: TCP.\n");
        if (modeTCP(json) == 1) {
            printf("Error: Algo falló en la lectura TCP.\n");
            cJSON_Delete(json);
            return 1;
        }
    }
    else if (strcmp(mode->valuestring, "udp") == 0)
    {
        printf("Mode: UDP.\n");
        if (modeUDP(json) == 1) {
            printf("Error: Algo falló en la lectura UDP.\n");
            cJSON_Delete(json);
            return 1;
        }
    }
    else if (strcmp(mode->valuestring, "http") == 0)
    {
        printf("Mode: HTTP.\n");
        if (modeHTTP(json) == 1) {
            printf("Error: Algo falló en la lectura HTTP.\n");
            cJSON_Delete(json);
            return 1;
        }
    }
    else
    {
        printf("Error: El mode seleccionado (%s) no existe.\n", mode->valuestring);
        cJSON_Delete(json);
        return 1;
    }

    // Eliminar JSON 
    cJSON_Delete(json); 
    return 0; 
}



// ============================================ MODES ============================================

int modeTCP(cJSON *json) {
    
    cJSON *tcp = cJSON_GetObjectItemCaseSensitive(json, "tcp"); 

    if (tcp == NULL)
    {
        printf("Error: No se encontró la configuración TCP.\n");
        return 1;
    }

    cJSON *ports = cJSON_GetObjectItemCaseSensitive(tcp, "ports");

    if (ports == NULL) {
        printf("Error: No se encontraron los puertos TCP.\n");
        return 1;
    }

    
    cJSON *port; // Puerto que se está iterando en la lista de puertos

    const char *portNumber; // Número de puerto que se está leyendo
    cJSON *portContent; // Contenido del puerto, contiene IP, Port y Weight

    cJSON *portData; // Dato que se está iterando en la lista de elementos de un puerto

    cJSON *weightExists; // Validación de la existencia del elemento weight

    const char *serverIp; // IP del servidor dentro de un puerto N
    const char *serverPort; // Puerto del servidor dentro de un puerto N
    const char *serverWeight; // Weight del servidor dentro de un puerto N

    int numServers; // Cantidad de servers
    struct Server servers[10]; // Lista de servers

    // Loops para extraer los datos de los puertos
    cJSON_ArrayForEach(port, ports) {

        numServers = 0;

        portNumber = port->string;
        portContent = cJSON_GetObjectItemCaseSensitive(ports, portNumber);

        printf("Port: %s\n", portNumber);
        
        if (portContent == NULL || !cJSON_IsArray(portContent)) {
            printf("PORT CONTENT: NULL or not an array\n");
            continue;
        }

        cJSON_ArrayForEach(portData, portContent) {

            if (!cJSON_IsObject(portData)) {
                printf("PORT DATA: NULL or not an object\n");
                continue;
            }

            serverIp = cJSON_GetObjectItemCaseSensitive(portData, "ip")->valuestring;
            serverPort = cJSON_GetObjectItemCaseSensitive(portData, "port")->valuestring;
            weightExists = cJSON_GetObjectItemCaseSensitive(portData, "weight");
            if (weightExists && cJSON_IsString(weightExists)) {
                serverWeight = weightExists->valuestring;
            } else {
                serverWeight = NULL;
            }

            // Revisar por variables de entorno
            if (serverIp[0] == '$'){
                serverIp = getenv(serverIp+1);
            }
            if (serverPort[0] == '$'){
                serverPort = getenv(serverPort+1);
            }
            if (serverWeight != NULL && serverWeight[0] == '$'){
                serverWeight = getenv(serverWeight+1);
            }
            // --------------------------------

            if (serverIp == NULL){
                serverIp = "10.0.0.0";
            }

            printf("  IP: %s, Port: %s", serverIp, serverPort);
            if (serverWeight) {
                printf(", Weight: %s", serverWeight);
            }
            printf("\n\n");

            struct Server server;
            strcpy(server.ip, serverIp);
            server.port = atoi(serverPort);
            if (serverWeight != NULL){
                server.weight = atoi(serverWeight);
            } else {
                server.weight = -1;
            }
            servers[numServers++] = server;
            
        }
        
        // Se realizan las acciones del proxy por puerto
        if (proxyTcp(atoi(portNumber), servers, numServers) == 1) {
            printf("Error: El proxy con el puerto: %s falló.\n", portNumber);
            return 1;
        }
    }

    return 0;
}

int modeUDP(cJSON *json) {
    cJSON *udp = cJSON_GetObjectItemCaseSensitive(json, "udp"); 

    if (udp == NULL)
    {
        printf("Error: No se encontró la configuración UDP.\n");
        return 1;
    }

    cJSON *ports = cJSON_GetObjectItemCaseSensitive(udp, "ports");

    if (ports == NULL) {
        printf("Error: No se encontraron los puertos UDP.\n");
        return 1;
    }

    
    cJSON *port; // Puerto que se está iterando en la lista de puertos

    const char *portNumber; // Número de puerto que se está leyendo
    cJSON *portContent; // Contenido del puerto, contiene IP, Port y Weight

    cJSON *portData; // Dato que se está iterando en la lista de elementos de un puerto

    cJSON *weightExists; // Validación de la existencia del elemento weight

    const char *serverIp; // IP del servidor dentro de un puerto N
    const char *serverPort; // Puerto del servidor dentro de un puerto N
    const char *serverWeight; // Weight del servidor dentro de un puerto N


    // Loops para extraer los datos de los puertos
    cJSON_ArrayForEach(port, ports) {
        portNumber = port->string;
        portContent = cJSON_GetObjectItemCaseSensitive(ports, portNumber);

        printf("Port: %s\n", portNumber);

        if (portContent == NULL || !cJSON_IsArray(portContent)) {
            printf("PORT CONTENT: NULL or not an array\n");
            continue;
        }

        cJSON_ArrayForEach(portData, portContent) {

            if (!cJSON_IsObject(portData)) {
                printf("PORT DATA: NULL or not an object\n");
                continue;
            }

            serverIp = cJSON_GetObjectItemCaseSensitive(portData, "ip")->valuestring;
            serverPort = cJSON_GetObjectItemCaseSensitive(portData, "port")->valuestring;
            weightExists = cJSON_GetObjectItemCaseSensitive(portData, "weight");
            if (weightExists && cJSON_IsString(weightExists)) {
                serverWeight = weightExists->valuestring;
            } else {
                serverWeight = NULL;
            }

            // Revisar por variables de entorno
            if (serverIp[0] == '$'){
                serverIp = getenv(serverIp+1);
            }
            if (serverPort[0] == '$'){
                serverPort = getenv(serverPort+1);
            }
            if (serverWeight != NULL && serverWeight[0] == '$'){
                serverWeight = getenv(serverWeight+1);
            }
            // --------------------------------

            printf("  IP: %s, Port: %s", serverIp, serverPort);
            if (serverWeight) {
                printf(", Weight: %s", serverWeight);
            }
            printf("\n");

            if (proxyUdp(portNumber, serverIp, serverPort, serverWeight)  == 1) {
                printf("Error: El servidor con el IP: %s y Port: %s falló.\n", serverIp, serverPort);
                return 1;
            }
        }
    }
    return 0;
}

int modeHTTP(cJSON *json) {
    cJSON *http = cJSON_GetObjectItemCaseSensitive(json, "http"); 

    if (http == NULL)
    {
        printf("Error: No se encontró la configuración HTTP.\n");
        return 1;
    }

    cJSON *ports = cJSON_GetObjectItemCaseSensitive(http, "ports");

    if (ports == NULL) {
        printf("Error: No se encontraron los puertos HTTP.\n");
        return 1;
    }

    cJSON *port; // Puerto que se está iterando en la lista de puertos

    const char *portNumber; // Número de puerto que se está leyendo
    cJSON *portContent; // Contenido del puerto, contiene hostnames

    cJSON *dns; // DNS que se está iterando en la lista de hostnames del puerto
    const char *hostname; // Hostname que se está leyendo
    //struct addrinfo *result; // Se utiliza para validar que el hostname existe

    cJSON *hostnameContent; // Contenido del hostname: contiene IP, Port, Weight, Type y Path
    cJSON *hostnameData; // Dato que se está iterando en la lista de elementos de un DNS

    cJSON *weightExists; // Validación de la existencia del elemento weight

    const char *serverIp; // IP del servidor dentro de un hostname
    const char *serverPort; // Puerto del servidor dentro de un hostname
    const char *serverWeight; // Weight del servidor dentro de un hostname
    const char *serverType; // Tipo de servidor dentro de un hostname
    const char *serverPath; // Path de servidor dentro de un hostname, depende del serverType

    const char *variableEnt;

    // Loops para extraer los datos de los puertos
    cJSON_ArrayForEach(port, ports) {
        portNumber = port->string;
        portContent = cJSON_GetObjectItemCaseSensitive(ports, portNumber);

        printf("Port: %s\n", portNumber);

        if (portContent == NULL) {
            printf("PORT CONTENT: NULL or not an array\n");
            continue;
        }

        cJSON_ArrayForEach(dns, portContent) {
            hostname = dns->string;
            hostnameContent = cJSON_GetObjectItemCaseSensitive(portContent, hostname);

            printf("Hostname: %s\n", hostname);

            // Validación del hostname
            if (strcmp(hostname, "www.name1.com") != 0) {
                printf("El Hostname '%s' es incorrecto. El hostname debe ser www.name1.com\n", hostname);
                return 1;
            }

            if (hostnameContent == NULL || !cJSON_IsArray(hostnameContent)) {
                printf("HOSTNAME: NULL or not an array\n");
                continue;
            }

            cJSON_ArrayForEach(hostnameData, hostnameContent) {

                if (!cJSON_IsObject(hostnameData)) {
                    printf("HOSTNAME DATA: NULL or not an object\n");
                    continue;
                }

                serverIp = cJSON_GetObjectItemCaseSensitive(hostnameData, "ip")->valuestring;
                serverPort = cJSON_GetObjectItemCaseSensitive(hostnameData, "port")->valuestring;
                weightExists = cJSON_GetObjectItemCaseSensitive(hostnameData, "weight");
                if (weightExists && cJSON_IsString(weightExists)) {
                    serverWeight = weightExists->valuestring;
                } else {
                    serverWeight = NULL;
                }
                serverType = cJSON_GetObjectItemCaseSensitive(hostnameData, "type")->valuestring;
                if (strcmp(serverType, "path") == 0) {
                    serverPath = cJSON_GetObjectItemCaseSensitive(hostnameData, "path")->valuestring;;
                } else {
                    serverPath = NULL;
                }

                // Revisar por variables de entorno
                if (serverIp[0] == '$'){
                    variableEnt = getenv(serverIp+1);
                    serverIp = variableEnt;
                }
                if (serverPort[0] == '$'){
                    serverPort = getenv(serverPort+1);
                }
                if (serverWeight != NULL && serverWeight[0] == '$'){
                    serverWeight = getenv(serverWeight+1);
                }
                if (serverType[0] == '$'){
                    serverType = getenv(serverType+1);
                }
                if (serverPath != NULL && serverPath[0] == '$'){
                    serverPath = getenv(serverPath+1);
                }
                // --------------------------------

                printf("  IP: %s, Port: %s, Type: %s", serverIp, serverPort, serverType);
                if (serverWeight) {
                    printf(", Weight: %s", serverWeight);
                }
                if (serverPath) {
                    printf(", Path: %s", serverPath);
                }
                printf("\n");

                if (proxyHttp(portNumber, serverIp, serverPort, serverWeight, serverType, serverPath) == 1) {
                    printf("Error: El proxy con el IP: %s y Port: %s falló.\n", serverIp, serverPort);
                    return 1;
                }
            }
        }
    }

    return 0;
}



// ============================================ PROXYS ============================================

int proxyTcp(int port, struct Server *servers, int numServers){
    
    // Numero random
    srand(time(NULL));
    int randomNum = rand() % 100;

    // Se escoge el servidor
    int totWeight = 0;
    int selServer = -1;
    for (int i = 0; i < numServers; i++) {
        totWeight += servers[i].weight;
        if (randomNum < totWeight) {
            selServer = i;
            break;
        }
    }

    // En caso de que no se pueda escoger porque no hay weights
    if (selServer == -1){
        selServer = rand() % numServers;
    }

    printf("Se escogio el server %d\n", selServer+1);
    printf("IP: %s\n", servers[selServer].ip);
    printf("PORT: %d\n", servers[selServer].port);

    // --------------

    int sockfd, client_sockfd;
    struct sockaddr_in proxy_addr, server_addr, client_addr;
    char buffer[1024];

    // Se crea el socket TCPcd ..
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Error: La creación del socket falló.\n");
        return 1;
    }

    // Configuración del proxy
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(port);

    // Enlazar el socket a la dirección del proxy
    if (bind(sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) == -1) {
        printf("Error: El enlazamiento del socket falló.\n");
        return 1;
    }

    // Escuchar por conexiones entrantes
    if (listen(sockfd, 1) == -1) {
        printf("Error: Algo falló al escuchar por conexiones entrantes.\n");
        return 1;
    }

    printf("Proxy escuchando en el puerto TCP %d...\n", port);

    // Configuración de la dirección del servidor basado en el server seleccionado
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = inet_addr(servers[selServer].ip);
    //server_addr.sin_port = htons(servers[selServer].port);
    server_addr.sin_addr.s_addr = inet_addr("10.0.0.40");
    server_addr.sin_port = htons(80);

    // Socket para el envio de datos al server
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        printf("Error: Algo falló al crear el socket del server.\n");
        //return 1;
    }

    // Conexion con el server
    if (connect(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("Error: Algo falló al conectarse con el servidor.\n");
        //return 1;
    }

    printf("Conectado al servidor\n");

    socklen_t client_len = sizeof(client_addr);

    // Aceptando una conexión entrante
    if ((client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len)) == -1) {
        printf("Error: Algo falló al aceptar conexiones entrantes.\n");
        //return 1;
    }


    printf("Peticion Recibida.\n");

    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

    if (send(client_sockfd, request, strlen(request), 0) == -1) {
        printf("Error al enviar la solicitud al servidor");
        //return 1;
    }

    printf("Peticion enviada al servidor.\n");

    close(client_sockfd);
    close(server_sockfd);

    close(sockfd);

    printf("Conexiones cerradas\n");

    return 0;
}

int proxyUdp(const char *port, const char *serverIp, const char *serverPort, const char *weight){
    
}

int proxyHttp(const char *port, const char *serverIp, const char *serverPort, const char *weight, const char *type, const char *path){

}