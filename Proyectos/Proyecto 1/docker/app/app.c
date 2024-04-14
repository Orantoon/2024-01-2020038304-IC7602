#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h> 


char *variable;
const char *port1;


int main(int argc, char *argv[]) {

    // Abrir archivo "config.json"
    FILE *fp = fopen("config.json", "r");
    if (fp == NULL) {
        printf("Error: No se pudo abrir el archivo.\n");
        return 1;
    }

    // Guardar contenido de "config.json" en un buffer
    char buffer[1024];
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
    else if (strcmp(mode->valuestring, "udp") == 0)
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
        printf("Error: El mode seleccionado no existe.\n");
        cJSON_Delete(json);
        return 1;
    }

    //char entorno = "SERVER1";
    //variable = getenv(entorno);

    // Eliminar JSON 
    cJSON_Delete(json); 
    return 0; 
}

int modeTCP(cJSON *json) {
    
    cJSON *tcp = cJSON_GetObjectItemCaseSensitive(json, "tcp"); 

    if (tcp == NULL)
    {
        printf("Error: No se encontraron los puertos TCP.\n");
        return 1;
    }

    cJSON *ports = cJSON_GetObjectItemCaseSensitive(tcp, "ports");

    if (ports == NULL) {
        printf("Error: No se encontraron los puertos.\n");
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

            printf("  IP: %s, Port: %s", serverIp, serverPort);
            if (serverWeight) {
                printf(", Weight: %s\n", serverWeight);
            } else {
                printf("\n");
            }

            if (servidorTcpUdp(serverIp, serverPort, serverWeight) == 1) {
                printf("Error: El servidor con el IP: %s y Port: %s falló.\n", serverIp, serverPort);
                return 1;
            }
        }
    }

    return 0;
}

int modeUDP(cJSON *json) {
    cJSON *udp = cJSON_GetObjectItemCaseSensitive(json, "udp"); 

    if (udp == NULL)
    {
        printf("Error: No se encontraron los puertos UDP.\n");
        return 1;
    }

    cJSON *ports = cJSON_GetObjectItemCaseSensitive(udp, "ports");

    if (ports == NULL) {
        printf("Error: No se encontraron los puertos.\n");
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

            printf("  IP: %s, Port: %s", serverIp, serverPort);
            if (serverWeight) {
                printf(", Weight: %s\n", serverWeight);
            } else {
                printf("\n");
            }

            if (servidorTcpUdp(serverIp, serverPort, serverWeight) == 1) {
                printf("Error: El servidor con el IP: %s y Port: %s falló.\n", serverIp, serverPort);
                return 1;
            }
        }
    }

    return 0;
}

int modeHTTP(cJSON *json) {
    printf("HTTP\n");
    return 0;
}


int servidorTcpUdp(const char *ip, const char *port, const char *weight){
    printf("SERVER!\n");
    return 0;
}