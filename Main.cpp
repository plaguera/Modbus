/*
 * Main.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: Pedro Lagüera Cabrera | alu0100891485
 */
#include "ModbusTCP.hpp"

modbus::ModbusTCP ModbusTCP;

// Función que maneja señales de interrupción
void SignalHandler(int signal) {
	std::string text;
	switch (signal) {
		case 1: text = "SIGHUP"; break;
		case 2: text = "SIGINT"; break;
		case 3: text = "SIGQUIT"; break;
		case 4: text = "SIGILL"; break;
		case 5: text = "SIGTRAP"; break;
		case 6: text = "SIGABRT"; break;
		default: break;
	}
	std::cerr << YELLOW << "\nCaught Signal: " << text << RESET << std::endl;
	ModbusTCP.Stop();
	exit(1);
}
// echo -en '\x06\x03\x00\x00\x00\x01\x85\xBD' | nc localhost 1502 | xxd -ps -u | sed 's/.\{2\}/0x& /g'
// Manejar la señal SIGINT, añadimos los dos dispositivos del enunciado: 33 y 18
int main() {
	signal(SIGINT, SignalHandler);
	ModbusTCP.AddDevice(0x21); // Dec = 33, Hex = 0x21
	ModbusTCP.AddDevice(0x12); // Dec = 18, Hex = 0x12
	try {
		ModbusTCP.Start();
	} catch (const char* msg) {
		std::cerr << msg << std::endl;
	}
	
	return 0;
}
