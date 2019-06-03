/*
 * Util.hpp
 *
 *  Created on: Apr 12, 2019
 *      Author: Pedro Lagüera Cabrera | alu0100891485
 */
#ifndef UTIL_H_
#define UTIL_H_

#define BYTE_ZERO 0x00000000
#define BYTE_ONE 0x00000001
#define COMMAND_DEVICES "devices"
#define COMMAND_GET "get"
#define COMMAND_HELP "help"
#define COMMAND_SELECT "select"
#define COMMAND_SET "set"
#define COMMAND_STOP "stop"
#define N_DIGITAL_INPUTS 20
#define N_DIGITAL_OUTPUTS 20
#define N_ANALOG_INPUTS 20
#define N_ANALOG_OUTPUTS 10

/* Colores de Output en el Terminal */
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLD   "\033[1m"      	/* Bold */

#include <algorithm>
#include <arpa/inet.h>
#include <atomic>
#include <bitset>
#include <csignal>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace modbus {

typedef uint8_t byte;

enum e_exception {
	ILLEGAL_FUNCTION,
	ILLEGAL_DATA_ADDRESS,
	ILLEGAL_DATA_VALUE,
	SLAVE_DEVICE_FAILURE,
	ACKNOWLEDGE,
	SLAVE_DEVICE_BUSY,
	NEGATIVE_ACKNOWLEDGE,
	MEMORY_PARITY_ERROR,
	GATEWAY_PATH_UNAVAILABLE,
	GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND
};

// Enumeración de Comandos disponibles
enum e_command {
	STOP,		// Parar servidor
	GET,		// Mostrar registros analógicos, digitales, entrada o salida, o cualquier combinación
	SET,		// Cambiar el valor de los últimos 5 registros analógicos o digitales
	SELECT,		// Seleccionar un dispositivo
	DEVICES,	// Mostrar dispositivos conectados
	HELP,		// Mostrar ayuda
	UNKNOWN		// Comando desconocido
};

// Enumeración de errores contemplados
enum e_error {
	BAD_COMMAND_SYNTAX,				// Comando válido pero error en argumentos
	COMMAND_NOT_FOUND,				// No existe tal comando
	INVALID_DEVICE_ID,				// No existe un dispositivo con el ID especificado
	INVALID_CRC,					// El CRC adjunto no coincide con el calculado
	UNABLE_TO_OPEN_SOCKET,			// Imposible abrir socket
	UNABLE_TO_BIND,					// Imposible unir socket, dirección y puerto
	UNABLE_TO_ACCEPT,				// Imposible aceptar conexión
	RECEIVE_ERROR					// Error al recibir datos
};

// Enumeración de modos de registro
enum e_mode { ANALOG, DIGITAL };

// Objeto Comando que consta de un comando (enum), argumentos y un constructor
struct Command
{
    e_command command;
    std::vector<std::string> args;
	Command(e_command cmd, std::vector<std::string> args) : command(cmd), args(args) {}
};

// Funciones y variables estáticas de utilidades y funciones que todos pueden usar
class Util {
public:
	static std::map<std::string, e_command> map_command;				// Mapa de correspondencia entre comandos en texto y sus valores en la enumeración
	static int ToInt(byte lsb, byte msb);								// Convierte dos bytes en un entero
	static byte ToByte(int value);										// Convierte un int a byte
	static byte ToByte(std::string string);								// Convierte un string a byte
	static std::string ToString(byte value);							// Convierte un byte a string
	static std::string ToString(std::vector<byte> input);				// Convierte un vector de bytes a string
	static std::string Color(std::string color, std::string string);	// Colorea 'string' de 'color'
	static std::string FormatSockAddr(sockaddr_in* addr);				// Construye un string coloreado de una dirección sockaddr_in*
	static bool IsInt(std::string string);								// Devuelve si todos los caracteres de un string son números
	static std::vector<byte> CRC(std::vector<byte> input);				// Calcula el CRC de un array de bytes
	static std::vector<byte> AddCRC(std::vector<byte> input);			// Añade el CRC de un array de bytes al mismo
	static bool CheckCRC(std::vector<byte> input);						// Comprobar que los bytes CRC del vector concuerdan con el CRC del mismo
	static std::string RemoveWhitespaces(std::string input);			// Quita los espacios en blanco de un string
	static Command TokenizeCommand(const std::string& str, const std::string& delimiters = " "); // Divide 'str' en tokens utilizando 'delimiters' como separadores

	static void Error(e_error code);									// Imprimir un código error y un mensaje
	static void Exception(e_error code);								// Imprimir un código error y un mensaje y lanzar excepción

private:
	static std::string ErrorMessage(e_error code);						// Devuelve un mensaje de error con descriptivo para cada código
};

}

#endif /* UTIL_H_ */
