#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h> 


char *variable;

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
        cJSON_Delete(json); 
        return 1; 
    }

    // ----------------------------------------

    // Dependiendo del mode, la funcion que se utiliza cambia
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(json, "mode"); 

    int validacion = 0;
    if (strcmp(mode->valuestring, "tcp") == 0)
    {
        validacion = modeTCP(json);
        if (validacion == 1) {
            printf("Error: Algo falló en la lectura TCP.\n");
            return 1;
        }
    }
    else if (strcmp(mode->valuestring, "udp") == 0)
    {
        validacion = modeUDP(json);
        if (validacion == 1) {
            printf("Error: Algo falló en la lectura UDP.\n");
            return 1;
        }
    }
    else if (strcmp(mode->valuestring, "udp") == 0)
    {
        validacion = modeHTTP(json);
        if (validacion == 1) {
            printf("Error: Algo falló en la lectura HTTP.\n");
            return 1;
        }
    }
    else
    {
        printf("Error: El mode seleccionado no existe.\n");
        return 1;
    }

    char entorno = "SERVER1";
    variable = getenv(entorno);

    // Eliminar JSON 
    cJSON_Delete(json); 
    return 0; 
}

int modeTCP(cJSON *json) {
    
    cJSON *tcp = cJSON_GetObjectItemCaseSensitive(tcp, "tcp"); 
    printf(tcp);
    if (tcp == NULL)
    {
        printf("Error: No se encontraron los puertos TCP.\n");
        return 1;
    }

    return 0;
}

int modeUDP(cJSON *json) {
    printf("UDP\n");
    return 0;
}

int modeHTTP(cJSON *json) {
    printf("HTTP\n");
    return 0;
}