# TPE Protocolos de Comunicación POP3 - Grupo 05

Implementación de un servidor POP3 y cliente administrador sobre este. 
Compatible con Mail User Agent como Dovecot y Thunderbird.

Toda la documentación se encuentra en la carpeta docs

## Instrucciones de instalación y ejecución

### Servidor

Para compilar el servidor, dentro de la carpeta ./src ejecutar:

```bash
make pop3d
```

Para su ejecución, desde la misma carpeta debe introducir el siguiente comando:

```bash
./pop3d [-u <user>:<pass>] -d <maildir> [-p <pop3_port>] [-P <manager_port>]
```

Siendo lo que está entre corchetes opcional ([]) y donde:

* user es el usuario que se quiere autenticar
* pass es la contraseña del usuario
* maildir es el directorio de los mails del servidor 
* pop3_port es el puerto donde se conectan los clientes para usar el servidor POP3
* manager_port es el puerto donde se conectan los clientes para usar el protocolo del manager

## Comandos soportados
| Comandos           | Descripción                                               |
|--------------------|-----------------------------------------------------------|
| -h                 | Imprime la ayuda y termina.                               |
| -p \<pop3 port>    | Puerto entrante conexiones POP3.                          |
| -o \<conf port>    | Puerto entrante conexiones configuración.                 |
| -u \<name:pass>    | Usuario y contraseña de un usuario válido.                |
| -d \<path-to-mais> | Directorio donde se almacenarán los usuarios y sus mails. |
| -v                 | Imprime información sobre la versión y termina.           |


### Aplicación cliente

Para compilar el cliente, dentro de la carpeta ./src ejecutar:

```bash
make mc
```

Para su ejecución, desde la misma carpeta debe introducir el siguiente comando:

```bash
nc -C localhost <pop3_port>
```

Donde pop3_port representa el puerto especificado, y localhost es la dirección.

Una vez ejecutado, la instrucción CAPA permitirá ver los comandos disponibles.
