#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_FAVS 100

#define MAX_COMMANDS 10
#define MAX_ARGS 10

// Estructura para almacenar los comandos favoritos
typedef struct {
    int id;
    char command[256];
} Favorite;

Favorite favs[MAX_FAVS]; // Array para almacenar los favoritos
int fav_count = 0;      // Contador de favoritos

// Función para crear un archivo de favoritos
void favs_crear(char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error al crear el archivo de favoritos");
        return;
    }
    fclose(file);
    printf("Archivo de favoritos creado en %s\n", path);
}

// Función para mostrar todos los favoritos
void favs_mostrar() {
    for (int i = 0; i < fav_count; i++) {
        printf("%d: %s\n", favs[i].id, favs[i].command);
    }
}

// Función para eliminar favoritos por ID
void favs_eliminar(int ids[], int num_ids) {
    for (int i = 0; i < num_ids; i++) {
        int found = 0;
        for (int j = 0; j < fav_count; j++) {
            if (favs[j].id == ids[i]) {
                found = 1;
                for (int k = j; k < fav_count - 1; k++) {
                    favs[k] = favs[k + 1];
                }
                fav_count--;
                break;
            }
        }
        if (!found) {
            printf("Comando con ID %d no encontrado.\n", ids[i]);
        }
    }
}

// Función para buscar comandos en favoritos
void favs_buscar(char *cmd) {
    for (int i = 0; i < fav_count; i++) {
        if (strstr(favs[i].command, cmd) != NULL) {
            printf("%d: %s\n", favs[i].id, favs[i].command);
        }
    }
}

// Función para borrar todos los favoritos
void favs_borrar() {
    fav_count = 0; // Limpiar la lista de favoritos
}

// Función para guardar los favoritos en un archivo
void favs_guardar(char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error al guardar los favoritos");
        return;
    }
    for (int i = 0; i < fav_count; i++) {
        fprintf(file, "%d %s\n", favs[i].id, favs[i].command);
    }
    fclose(file);
    printf("Favoritos guardados en %s\n", path);
}

// Función para cargar favoritos desde un archivo
void favs_cargar(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error al cargar los favoritos");
        return;
    }
    fav_count = 0;
    while (fscanf(file, "%d %[^\n]", &favs[fav_count].id, favs[fav_count].command) != EOF) {
        fav_count++;
    }
    fclose(file);
    printf("Favoritos cargados desde %s\n", path);
}

// Función para agregar un comando a favoritos automáticamente
void agregar_fav_automatico(char *cmd) {
    for (int i = 0; i < fav_count; i++) {
        if (strcmp(favs[i].command, cmd) == 0) {
            return; // Ya está en la lista de favoritos
        }
    }
    if (fav_count < MAX_FAVS) {
        favs[fav_count].id = fav_count + 1;
        strcpy(favs[fav_count].command, cmd);
        fav_count++;
    }
}

// Función para ejecutar un comando favorito por su ID
void favs_num_ejecutar(int num) {
    for (int i = 0; i < fav_count; i++) {
        if (favs[i].id == num) {
            char *args[100];
            int j = 0;
            args[j] = strtok(favs[i].command, " ");
            while (args[j] != NULL) {
                j++;
                args[j] = strtok(NULL, " ");
            }
            pid_t pid = fork();
            if (pid == 0) {
                execvp(args[0], args);
                perror("Error al ejecutar el comando favorito");
                exit(EXIT_FAILURE);

                
            } else {
                wait(NULL); // Esperar a que el proceso hijo termine
            }
            return;
        }
    }
    printf("Comando no encontrado en favoritos.\n");
}

// Función para establecer un recordatorio
void set_recordatorio(int segundos, char *mensaje) {
    pid_t pid = fork();
    if (pid == 0) {
        sleep(segundos); // Esperar el tiempo especificado
        printf("Recordatorio: %s\n", mensaje); // Mostrar el mensaje
        exit(0);
    }
}


void parse_commands(char* input, char* comandos[MAX_COMMANDS][MAX_ARGS], int* cant_comandos) {
    char* token;
    char* rest = input;

    *cant_comandos = 0;

    // Divide los comandos por "|"
    while ((token = strtok_r(rest, "|", &rest)) != NULL) {
        int arg_index = 0;
        char* arg_rest = token;
        char* arg;

        // Divide cada comando en sus argumentos
        while ((arg = strtok_r(arg_rest, " ", &arg_rest)) != NULL) {
            comandos[*cant_comandos][arg_index] = arg;
            arg_index++;
        }
        comandos[*cant_comandos][arg_index] = NULL;  // Fin de los argumentos para este comando
        (*cant_comandos)++;
    }
}

void ejecutar_con_pipe(char* comandos[MAX_COMMANDS][MAX_ARGS], int cant_comandos) {
    int cant_pipes = cant_comandos - 1;
    pid_t pid;
    int pipes[cant_pipes][2];

    // Creación de pipes
    for (int i = 0; i < cant_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            printf("Error creando pipe\n");
            exit(1);
        }
    }

    // Creación de procesos
    for (int index = 0; index < cant_comandos; index++) {
        if ((pid = fork()) < 0) {
            printf("ERROR: fallo en el fork\n");
            exit(1);
        }

        if (pid == 0) {  // Proceso hijo
            if (index > 0) {
                dup2(pipes[index-1][0], STDIN_FILENO);  // Redirige stdin al pipe anterior
            }

            if (index + 1 < cant_comandos) {
                dup2(pipes[index][1], STDOUT_FILENO);  // Redirige stdout al siguiente pipe
            }

            // Cerrar todos los pipes
            for (int i = 0; i < cant_pipes; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            // Ejecutar el comando
            if (execvp(comandos[index][0], comandos[index]) == -1) { 
                perror("Comando no reconocido");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Proceso padre cierra todos los pipes
    for (int i = 0; i < cant_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Esperar a que todos los hijos terminen
    for (int i = 0; i < cant_comandos; i++) {
        wait(NULL);
    }
}


int main() {
    char input[256];
    char *args[100]; // Para almacenar los argumentos del comando
    char *comandos[10][100]; // Para comandos con pipes
    int num_comandos;

    // Cargar favoritos al inicio si existe un archivo por defecto
    favs_cargar("favoritos.txt");

    while (1) {
        printf("mishell:$ ");
        fgets(input, sizeof(input), stdin); // Leer entrada

        if (strcmp(input, "\n") == 0) continue; // Si se presiona solo "Enter"

        // Verificar si hay un pipe
        if (strchr(input, '|')) {

            // Elimina el salto de línea al final
            input[strcspn(input, "\n")] = 0;

            char* comandos[MAX_COMMANDS][MAX_ARGS];
            int cant_comandos = 0;

            // Parsear los comandos
            parse_commands(input, comandos, &cant_comandos);

            // Ejecutar los comandos con pipes
            ejecutar_con_pipe(comandos, cant_comandos);
            continue;
        }

        // Parsear la entrada en tokens para ejecución normal
        int i = 0;
        args[i] = strtok(input, " \n");
        while (args[i] != NULL) {
            i++;
            args[i] = strtok(NULL, " \n");
        }

        // Comando para salir de la shell
        if (strcmp(args[0], "exit") == 0) {
            // Guardar favoritos al salir
            favs_guardar("favoritos.txt");
            break;
        }

        // Implementación de comandos personalizados para manejar favoritos
        if (strcmp(args[0], "favs") == 0) {
            if (strcmp(args[1], "crear") == 0 && args[2] != NULL) {
                favs_crear(args[2]);
            } else if (strcmp(args[1], "mostrar") == 0) {
                favs_mostrar();
            } else if (strcmp(args[1], "eliminar") == 0 && args[2] != NULL) {
                int ids[10], num_ids = 0;
                char *id = strtok(args[2], ",");
                while (id != NULL) {
                    ids[num_ids++] = atoi(id);
                    id = strtok(NULL, ",");
                }
                favs_eliminar(ids, num_ids);
            } else if (strcmp(args[1], "buscar") == 0 && args[2] != NULL) {
                favs_buscar(args[2]);
            } else if (strcmp(args[1], "borrar") == 0) {
                favs_borrar();
            } else if (strcmp(args[1], "guardar") == 0 && args[2] != NULL) {
                favs_guardar(args[2]);
            } else if (strcmp(args[1], "cargar") == 0 && args[2] != NULL) {
                favs_cargar(args[2]);
            } else if (strcmp(args[1], "num") == 0 && strcmp(args[2], "ejecutar") == 0 && args[3] != NULL) {
                favs_num_ejecutar(atoi(args[3]));
            }
            continue;
        }

        // Verificar si el comando es "set recordatorio"
        if (strcmp(args[0], "set") == 0 && strcmp(args[1], "recordatorio") == 0 && args[2] != NULL) {
            int segundos = atoi(args[2]);
            char mensaje[256] = "";
            for (int i = 3; args[i] != NULL; i++) {
                strcat(mensaje, args[i]);
                strcat(mensaje, " ");
            }
            set_recordatorio(segundos, mensaje);
            continue;
        }

        // Ejecución de comandos normales
        agregar_fav_automatico(input);  // Agrega comando a favoritos si no es favs
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                // Manejo del error cuando el comando no existe
                printf("Comando no encontrado: %s\n", args[0]);
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("Error en fork");
        }
    }
    return 0;
}