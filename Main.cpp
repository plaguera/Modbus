/*
 * Main.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: pedro
 */
#include "ModbusTCP.hpp"

modbus::ModbusTCP ModbusTCP;

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
	std::cerr << "\nCaught signal: " << text << " !!" << std::endl;
	ModbusTCP.Stop();
	exit(1);
}

int main() {
	signal(SIGINT, SignalHandler);
	std::vector<modbus::byte> petition { 0x06, 0x04, 0x00, 0x00, 0x00, 0xAD, 0x30 };
	modbus::Util::PrintByteVector(petition);
	
	ModbusTCP.AddDevice(0x2); // Dec = 33, Hex = 0x21
	ModbusTCP.AddDevice(0x12); // Dec = 18, Hex = 0x12
	try {
		ModbusTCP.Start();
	} catch (const char* msg) {
		std::cerr << msg << std::endl;
	}
	
	return 0;
}
