#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_FAVS 100 // Número máximo de favoritos
#define MAX_COMMANDS 10 // Número máximo de comandos permitidos
#define MAX_ARGS 10 // Número máximo de argumentos por comando

// Estructura para almacenar los comandos favoritos
typedef struct {
    int id; // Identificador único del favorito
    char command[256]; // Comando almacenado como favorito
} Favorite;

Favorite favs[MAX_FAVS]; // Array para almacenar los favoritos
int fav_count = 0; // Contador de favoritos almacenados

// Función para crear un archivo de favoritos
// Recibe como parámetro la ruta del archivo a crear
void favs_crear(char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error al crear el archivo de favoritos");
        return;
    }
    fclose(file);
    printf("Archivo de favoritos creado en %s\n", path);
}

// Función para mostrar todos los favoritos almacenados
// Recorre el array de favoritos y muestra el ID y el comando
void favs_mostrar() {
    for (int i = 0; i < fav_count; i++) {
        printf("%d: %s\n", favs[i].id, favs[i].command);
    }
}

// Función para eliminar uno o varios favoritos por ID
// Recibe un array de IDs y la cantidad de IDs a eliminar
void favs_eliminar(int ids[], int num_ids) {
    for (int i = 0; i < num_ids; i++) {
        int found = 0;
        for (int j = 0; j < fav_count; j++) {
            if (favs[j].id == ids[i]) {
                found = 1;
                // Desplazar el resto de favoritos para ocupar el espacio del eliminado
                for (int k = j; k < fav_count - 1; k++) {
                    favs[k] = favs[k + 1];
                }
                fav_count--; // Decrementar el contador de favoritos
                break;
            }
        }
        if (!found) {
            printf("Comando con ID %d no encontrado.\n", ids[i]);
        }
    }
}

// Función para buscar comandos favoritos por coincidencia de texto
// Recibe el comando a buscar como parámetro
void favs_buscar(char *cmd) {
    for (int i = 0; i < fav_count; i++) {
        if (strstr(favs[i].command, cmd) != NULL) {
            printf("%d: %s\n", favs[i].id, favs[i].command);
        }
    }
}

// Función para eliminar todos los favoritos
// Reinicia el contador de favoritos a cero
void favs_borrar() {
    fav_count = 0; // Limpiar la lista de favoritos
}

// Función para guardar los favoritos en un archivo
// Recibe la ruta del archivo donde se guardarán los favoritos
void favs_guardar(char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error al guardar los favoritos");
        return;
    }
    // Escribir cada favorito en el archivo con su ID y comando
    for (int i = 0; i < fav_count; i++) {
        fprintf(file, "%d %s\n", favs[i].id, favs[i].command);
    }
    fclose(file);
    printf("Favoritos guardados en %s\n", path);
}

// Función para cargar los favoritos desde un archivo
// Recibe la ruta del archivo a cargar
void favs_cargar(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error al cargar los favoritos");
        return;
    }
    fav_count = 0; // Reiniciar el contador de favoritos antes de cargar
    // Leer cada favorito del archivo y almacenarlo en el array
    while (fscanf(file, "%d %[^\n]", &favs[fav_count].id, favs[fav_count].command) != EOF) {
        fav_count++;
    }
    fclose(file);
    printf("Favoritos cargados desde %s\n", path);
}

// Función para agregar automáticamente un comando a favoritos si no está ya presente
// Recibe el comando a agregar como parámetro
void agregar_fav_automatico(char *cmd) {
    // Verificar si el comando ya está en la lista de favoritos
    for (int i = 0; i < fav_count; i++) {
        if (strcmp(favs[i].command, cmd) == 0) {
            return; // El comando ya está en favoritos, no se agrega de nuevo
        }
    }
    // Si no está, se agrega al final del array de favoritos
    if (fav_count < MAX_FAVS) {
        favs[fav_count].id = fav_count + 1; // Asignar un ID único
        strcpy(favs[fav_count].command, cmd); // Copiar el comando
        fav_count++; // Incrementar el contador de favoritos
    }
}

// Función para ejecutar un comando favorito por su ID
// Recibe el número de favorito a ejecutar
void favs_num_ejecutar(int num) {
    for (int i = 0; i < fav_count; i++) {
        if (favs[i].id == num) {
            char *args[100]; // Array para almacenar los argumentos del comando
            int j = 0;
            args[j] = strtok(favs[i].command, " "); // Separar el comando en argumentos
            while (args[j] != NULL) {
                j++;
                args[j] = strtok(NULL, " ");
            }
            pid_t pid = fork(); // Crear un proceso hijo para ejecutar el comando
            if (pid == 0) {
                execvp(args[0], args); // Ejecutar el comando en el proceso hijo
                perror("Error al ejecutar el comando favorito");
                exit(EXIT_FAILURE); // Terminar el proceso hijo si hay error
            } else {
                wait(NULL); // Esperar a que el proceso hijo termine
            }
            return;
        }
    }
    printf("Comando no encontrado en favoritos.\n");
}

// Función para establecer un recordatorio
// Recibe el número de segundos y el mensaje del recordatorio
void set_recordatorio(int segundos, char *mensaje) {
    pid_t pid = fork(); // Crear un proceso hijo para el recordatorio
    if (pid == 0) {
        sleep(segundos); // Esperar el tiempo especificado
        printf("Recordatorio: %s\n", mensaje); // Mostrar el mensaje del recordatorio
        exit(0); // Terminar el proceso hijo
    }
}

// Función para parsear (analizar) los comandos ingresados por el usuario
// Separa los comandos utilizando "|" como delimitador y luego separa cada comando en sus argumentos
// input: cadena con el comando completo ingresado por el usuario
// comandos: arreglo donde se almacenarán los comandos y sus argumentos
// cant_comandos: cantidad de comandos separados por el pipe
void parse_commands(char* input, char* comandos[MAX_COMMANDS][MAX_ARGS], int* cant_comandos) {
    char* token; // Token para almacenar el comando separado por el pipe
    char* rest = input; // Puntero auxiliar para recorrer el input

    *cant_comandos = 0; // Inicializa el contador de comandos

    // Divide los comandos por el símbolo "|"
    while ((token = strtok_r(rest, "|", &rest)) != NULL) {
        int arg_index = 0; // Índice para los argumentos de cada comando
        char* arg_rest = token; // Puntero auxiliar para los argumentos de cada comando
        char* arg; // Token para almacenar cada argumento

        // Divide cada comando en sus argumentos usando espacio como delimitador
        while ((arg = strtok_r(arg_rest, " ", &arg_rest)) != NULL) {
            comandos[*cant_comandos][arg_index] = arg; // Almacena cada argumento
            arg_index++;
        }
        comandos[*cant_comandos][arg_index] = NULL;  // Marca el fin de los argumentos para este comando
        (*cant_comandos)++; // Incrementa el contador de comandos
    }
}

// Función para ejecutar varios comandos en cadena utilizando pipes
// comandos: arreglo con los comandos y sus argumentos a ejecutar
// cant_comandos: número de comandos a ejecutar
void ejecutar_con_pipe(char* comandos[MAX_COMMANDS][MAX_ARGS], int cant_comandos) {
    int cant_pipes = cant_comandos - 1; // Cantidad de pipes necesarios (número de comandos - 1)
    pid_t pid; // Identificador de proceso
    int pipes[cant_pipes][2]; // Array de pipes, cada pipe tiene una entrada y una salida (2 extremos)

    // Creación de pipes
    for (int i = 0; i < cant_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            printf("Error creando pipe\n");
            exit(1); // Termina el programa si hay un error al crear el pipe
        }
    }

    // Creación de procesos hijos para ejecutar los comandos
    for (int index = 0; index < cant_comandos; index++) {
        if ((pid = fork()) < 0) { // Crea un nuevo proceso
            printf("ERROR: fallo en el fork\n");
            exit(1); // Termina el programa si hay un error al hacer fork
        }

        if (pid == 0) {  // Proceso hijo
            // Si no es el primer comando, redirige stdin al pipe anterior
            if (index > 0) {
                dup2(pipes[index-1][0], STDIN_FILENO);  // Redirige la entrada estándar (stdin) al pipe anterior
            }

            // Si no es el último comando, redirige stdout al siguiente pipe
            if (index + 1 < cant_comandos) {
                dup2(pipes[index][1], STDOUT_FILENO);  // Redirige la salida estándar (stdout) al siguiente pipe
            }

            // Cerrar todos los pipes en el proceso hijo
            for (int i = 0; i < cant_pipes; i++) {
                close(pipes[i][0]); // Cierra la lectura del pipe
                close(pipes[i][1]); // Cierra la escritura del pipe
            }

            // Ejecuta el comando utilizando execvp
            // Si el comando no es válido, muestra un error y termina el proceso hijo
            if (execvp(comandos[index][0], comandos[index]) == -1) { 
                perror("Comando no reconocido");
                exit(EXIT_FAILURE); // Termina el proceso hijo en caso de error
            }
        }
    }

    // Proceso padre cierra todos los pipes
    for (int i = 0; i < cant_pipes; i++) {
        close(pipes[i][0]); // Cierra la lectura del pipe
        close(pipes[i][1]); // Cierra la escritura del pipe
    }

    // Espera a que todos los procesos hijos terminen
    for (int i = 0; i < cant_comandos; i++) {
        wait(NULL); // El proceso padre espera a que termine cada proceso hijo
    }
}

int main() {
    char input[256];  // Buffer para almacenar la entrada del usuario
    char *args[100];  // Arreglo para almacenar los argumentos del comando
    char *comandos[10][100];  // Arreglo para comandos con pipes
    int num_comandos;

    // Cargar favoritos al inicio si existe un archivo por defecto
    favs_cargar("favoritos.txt");

    while (1) {
        printf("mishell:$ ");  // Muestra el prompt de la shell
        fgets(input, sizeof(input), stdin);  // Lee la entrada del usuario

        // Si el usuario solo presiona "Enter", se vuelve a mostrar el prompt
        if (strcmp(input, "\n") == 0) continue;

        // Verificar si hay un pipe en la entrada del usuario
        if (strchr(input, '|')) {
            // Elimina el salto de línea al final de la entrada
            input[strcspn(input, "\n")] = 0;

            char* comandos[MAX_COMMANDS][MAX_ARGS];  // Arreglo para almacenar comandos con sus argumentos
            int cant_comandos = 0;  // Inicializa el contador de comandos

            // Parsear los comandos ingresados con pipes
            parse_commands(input, comandos, &cant_comandos);

            // Ejecutar los comandos con pipes
            ejecutar_con_pipe(comandos, cant_comandos);
            continue;
        }

        // Parsear la entrada en tokens para la ejecución de un comando normal (sin pipes)
        int i = 0;
        args[i] = strtok(input, " \n");  // Separa el primer argumento
        while (args[i] != NULL) {  // Sigue separando los argumentos
            i++;
            args[i] = strtok(NULL, " \n");
        }

        // Comando para salir de la shell
        if (strcmp(args[0], "exit") == 0) {
            // Guardar favoritos antes de salir
            favs_guardar("favoritos.txt");
            break;  // Termina el ciclo del programa
        }

        // Implementación de comandos personalizados para manejar favoritos
        if (strcmp(args[0], "favs") == 0) {
            if (strcmp(args[1], "crear") == 0 && args[2] != NULL) {
                favs_crear(args[2]);  // Crear un nuevo favorito
            } else if (strcmp(args[1], "mostrar") == 0) {
                favs_mostrar();  // Mostrar los favoritos
            } else if (strcmp(args[1], "eliminar") == 0 && args[2] != NULL) {
                int ids[10], num_ids = 0;
                char *id = strtok(args[2], ",");
                while (id != NULL) {  // Parsear y eliminar favoritos por ID
                    ids[num_ids++] = atoi(id);
                    id = strtok(NULL, ",");
                }
                favs_eliminar(ids, num_ids);
            } else if (strcmp(args[1], "buscar") == 0 && args[2] != NULL) {
                favs_buscar(args[2]);  // Buscar un favorito
            } else if (strcmp(args[1], "borrar") == 0) {
                favs_borrar();  // Borrar todos los favoritos
            } else if (strcmp(args[1], "guardar") == 0 && args[2] != NULL) {
                favs_guardar(args[2]);  // Guardar favoritos en un archivo específico
            } else if (strcmp(args[1], "cargar") == 0 && args[2] != NULL) {
                favs_cargar(args[2]);  // Cargar favoritos desde un archivo
            } else if (strcmp(args[1], "num") == 0 && strcmp(args[2], "ejecutar") == 0 && args[3] != NULL) {
                favs_num_ejecutar(atoi(args[3]));  // Ejecutar un favorito por su número
            }
            continue;  // Volver al prompt después de ejecutar el comando de favoritos
        }

        // Comando para configurar un recordatorio
        if (strcmp(args[0], "set") == 0 && strcmp(args[1], "recordatorio") == 0 && args[2] != NULL) {
            int segundos = atoi(args[2]);  // Tiempo en segundos
            char mensaje[256] = "";
            for (int i = 3; args[i] != NULL; i++) {
                strcat(mensaje, args[i]);  // Construir el mensaje del recordatorio
                strcat(mensaje, " ");
            }
            set_recordatorio(segundos, mensaje);  // Configurar el recordatorio
            continue;
        }

        // Ejecución de comandos normales
        agregar_fav_automatico(input);  // Agrega el comando a favoritos si no es un comando de gestión de favoritos
        pid_t pid = fork();  // Crear un proceso hijo
        if (pid == 0) {  // En el proceso hijo
            if (execvp(args[0], args) == -1) {  // Ejecutar el comando
                // Manejo del error si el comando no existe
                printf("Comando no encontrado: %s\n", args[0]);
                exit(EXIT_FAILURE);  // Terminar el proceso hijo en caso de error
            }
        } else if (pid > 0) {  // En el proceso padre
            wait(NULL);  // Esperar a que el proceso hijo termine
        } else {
            perror("Error en fork");  // Manejar errores en el fork
        }
    }
    return 0;
}
