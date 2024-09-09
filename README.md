# MiShell

MiShell es una shell simple implementada en C que proporciona varias funcionalidades adicionales, como la gestión de comandos favoritos, recordatorios y soporte para pipes entre comandos. Este repositorio incluye el código fuente y las instrucciones para compilar y ejecutar la shell.

## Características

- **Gestión de Comandos Favoritos**: Crear, mostrar, eliminar, buscar, borrar, guardar y cargar comandos favoritos.
- **Recordatorios**: Establecer recordatorios con mensajes personalizados.
- **Pipes**: Ejecutar comandos con pipes entre ellos.
- **Ejecución de Comandos**: Ejecución de comandos normales y adición automática de comandos a favoritos.

## Requisitos

- Compilador C (como `gcc`)
- Sistema operativo basado en Unix (Linux, macOS, etc.)

## Compilación

Para compilar el código fuente de MiShell, sigue estos pasos:

1. Clona el repositorio:

    `git clone https://github.com/TomasGutierrez777/Shell_SO.git`

    `cd Shell_SO`

2. Compila el código:

    `gcc -o mishell main.c`

   Esto generará un ejecutable llamado `mishell`.

## Ejecución

Para ejecutar MiShell, usa el siguiente comando:

`./shell`

## Uso

Una vez que la shell esté en funcionamiento, puedes usar los siguientes comandos:

### Comandos de Favoritos

- **Crear un archivo de favoritos**: Crea un archivo para almacenar comandos favoritos.

  `favs crear <ruta>`

  Ejemplo:

  `favs crear favoritos.txt`

- **Mostrar comandos favoritos**: Muestra todos los comandos favoritos almacenados.

  `favs mostrar`

- **Eliminar comandos favoritos por ID**: Elimina comandos favoritos especificando sus IDs (separados por comas).

  `favs eliminar <IDs>`

  Ejemplo:

  `favs eliminar 1,3,5`

- **Buscar comandos favoritos**: Busca comandos favoritos que contengan la cadena especificada.

  `favs buscar <cadena>`

  Ejemplo:

  `favs buscar ls`

- **Borrar todos los favoritos**: Elimina todos los comandos favoritos.

  `favs borrar`

- **Guardar favoritos en un archivo**: Guarda los comandos favoritos en el archivo especificado.

  `favs guardar <ruta>`

  Ejemplo:

  `favs guardar favoritos.txt`

- **Cargar favoritos desde un archivo**: Carga comandos favoritos desde el archivo especificado.

  `favs cargar <ruta>`

  Ejemplo:

  `favs cargar favoritos.txt`

- **Ejecutar un comando favorito por ID**: Ejecuta un comando favorito dado su ID.

  `favs num ejecutar <ID>`

  Ejemplo:

  `favs num ejecutar 1`

### Recordatorios

- **Establecer un recordatorio**: Establece un recordatorio que se activará después de un número específico de segundos con un mensaje.

  `set recordatorio <segundos> <mensaje>`

  Ejemplo:

  `set recordatorio 10 "Esto es un recordatorio"`

### Pipes

- **Ejecutar comandos con pipes**: Usa el operador `|` para conectar la salida de un comando con la entrada de otro.

  `comando1 | comando2`

  Ejemplo:

  `ps -aux | head -10`

### Comandos Normales

- **Ejecutar comandos**: Los comandos que no sean parte de los comandos de favoritos o de recordatorio se ejecutan normalmente. Los comandos se agregarán automáticamente a la lista de favoritos si no forman parte de los comandos de la shell.

  Ejemplo:

  `ls -l`