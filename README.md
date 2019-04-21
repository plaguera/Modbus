# Computación Ubicua : Modbus
#### Pedro Lagüera Cabrera

## Instrucciones de Uso

El proyecto incluye un Makefile que compila los archivos necesarios para generar el programa. Al ejecutarlo se escucha por peticiones en el puerto 1502 y se abre un *prompt* para introducir comandos en tiempo real. El programa procesa peticiones y escucha por nuevas peticiones de manera concurrente. El uso de `Ctrl-C` está contemplado y termina el programa de manera *suave*.

##### Compilar el programa:

> make

##### Limpiar Ejecutables:

> make clean

##### Ejecutar

> ./main

## Comandos

##### Mostrar los Dispositivos Disponibles

> DEVICES

##### Mostrar los Registros Analógicos, Digitales, Entrada o Salida, o cualquier Combinación

> GET  [Analog | Digital | Input | Output]*

##### Mostrar Ayuda

> HELP

##### Seleccionar el Dispositivo sobre el que Ejecutar Comandos o Mostrar el Seleccionado

> SELECT [Device ID]?

##### Cambiar el Valor de los Últimos 5 Registros Analógicos o Digitales

> SET [Analog | Digital] [position]

##### Parar el Servidor

> STOP

## Envío de Peticiones

En vez de escribir un programa cliente dedicado a enviar, se ha optado por hacer uso de múltiples comandos de linux unidos por `|` *pipes*.

> echo -en '\x06\x03\x00\x00\x00\x01\x85\xBD' | nc localhost 1502 | xxd -ps -u | sed 's/.\{2\}/0x& /g'

 * *echo* - Crea una cadena de bytes sin tener en cuenta el carácter de retorno de carro.
 * *nc* - Envía la cadena de bytes a la dirección y puerto del servidor que está escuchando.
 * *xxd* - Convierte los datos binarios recibidos como respuesta a la petición en un hexdump.
 * *sed* - Formatea el hexdump a una cadena de bytes en formato *0xHH*.