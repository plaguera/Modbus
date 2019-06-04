/*
 * ModbusServer.cpp
 *
 *  Created on: Mar 21, 2019
 *      Author: pedro
 */
#include "ModbusServer.hpp"

namespace modbus {

// Crear dispositivo a partir de ID en decimal
ModbusServer::ModbusServer(int id) : ModbusServer((byte)id) {}

// Crear dispositivo a partir de ID en byte
ModbusServer::ModbusServer(byte id) {
	this->id = id;
	time_start = std::clock();
	digital_input = std::vector<bool>(N_DIGITAL_INPUTS, false);
	digital_output = std::vector<bool>(N_DIGITAL_OUTPUTS, false);
	analog_input = std::vector<int>(N_ANALOG_INPUTS, 0);
	analog_output = std::vector<int>(N_ANALOG_OUTPUTS, 0);

	// Analog Inputs

	/* Counters */
	// Received Petition Counter
	analog_input[0] = 0;
	// Bytes Received
	analog_input[1] = 0;
	// Bytes Sent
	analog_input[2] = 0;

	/* Date */
	// Year
	analog_input[3] = 0;
	// Month
	analog_input[4] = 0;
	// Day
	analog_input[5] = 0;
	// Hour
	analog_input[6] = 0;
	// Minute
	analog_input[7] = 0;
	// Second
	analog_input[8] = 0;

	/* CPU Time */
	// Seconds
	analog_input[9] = 0;
	// Miliseconds
	analog_input[10] = 0;

	/* Process Information */
	// UID
	analog_input[11] = 0;
	// GID
	analog_input[12] = 0;
	// PID
	analog_input[13] = 0;
	// PPID
	analog_input[14] = 0;

	for (int i = 15; i < analog_input.size(); i += 2) {
		analog_input[i] = 1111;
	}

	// Analog Outputs
	for (int i = 0; i < analog_output.size(); i++) {
		analog_output[i] = i * 4;
	}

	// Digital Inputs
	for (int i = 0; i < digital_input.size(); i++) {

	}

	// Digital Outputs
	for (int i = 0; i < digital_output.size(); i += 2) {
		digital_output[i] = true;
	}
}

// Destructor
ModbusServer::~ModbusServer() {
	// TODO Auto-generated destructor stub
}

// Actualizar los registros
void ModbusServer::Update() {

	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);

	/* Date */
	// Year
	analog_input[3] = now->tm_year + 1900;
	// Month
	analog_input[4] = now->tm_mon + 1;
	// Day
	analog_input[5] = now->tm_mday;
	// Hour
	analog_input[6] = now->tm_hour;
	// Minute
	analog_input[7] = now->tm_min;
	// Second
	analog_input[8] = now->tm_sec;

	/* CPU Time */
	std::clock_t time_end = std::clock();
	long double time_elapsed = 1000.0 * (time_end - time_start) / CLOCKS_PER_SEC;
	int time_elapsed_sec = int(time_elapsed / 1000.0);
	int time_elapsed_ms = int(1000.0 * (int(time_elapsed / 1000.0) - time_elapsed_sec));

	// Seconds
	analog_input[9] = time_elapsed_sec;
	// Miliseconds
	analog_input[10] = time_elapsed_ms;

	/* Process Information */
	// UID
	analog_input[11] = getuid();
	// GID
	analog_input[12] = getgid();
	// PID
	analog_input[13] = getpid();
	// PPID
	analog_input[14] = getppid();

	// Digital Inputs
	for (int i = 0; i < digital_input.size(); i++) {
		if (i < 15)
			digital_input[i] = analog_input[i] % 2 == 0;
	}
}

// Procesar una petición y devolver una respuesta
std::vector<byte> ModbusServer::Petition(std::vector<byte> input) {
	if (!IsRequestForMe(input)) return std::vector<byte>(); //throw "Incorrect ID !!";
	if (!Util::CheckCRC(input)) return std::vector<byte>(); //throw "Incorrect CRC Value !!";
	Update();
	std::vector<byte> output;
	switch(input[1]) {
		case 0x01: output = fc01(input); break;
		case 0x02: output = fc02(input); break;
		case 0x03: output = fc03(input); break;
		case 0x04: output = fc04(input); break;
		case 0x05: output = fc05(input); break;
		case 0x06: output = fc06(input); break;
		case 0x0F: output = fc15(input); break;
		case 0x10: output = fc16(input); break;
		default: output = Exception(ILLEGAL_FUNCTION, input);
	}
	analog_input[0]++;
	analog_input[1] += input.size();
	analog_input[2] += output.size();
	return output;
}

// Genera respuesta adecuada en caso de excepción en una fc
std::vector<byte> ModbusServer::Exception(e_exception exception, std::vector<byte> input) {
	byte function_code = input[1] | 0x80;
	std::vector<byte> output { input[0], function_code };
	byte code;
	switch (exception) {
		case ILLEGAL_FUNCTION: code = 0x01; break;
		case ILLEGAL_DATA_ADDRESS: code = 0x02; break;
		case ILLEGAL_DATA_VALUE: code = 0x03; break;
		case SLAVE_DEVICE_FAILURE: code = 0x04; break;
		case ACKNOWLEDGE: code = 0x05; break;
		case SLAVE_DEVICE_BUSY: code = 0x06; break;
		case NEGATIVE_ACKNOWLEDGE: code = 0x07; break;
		case MEMORY_PARITY_ERROR: code = 0x08; break;
		case GATEWAY_PATH_UNAVAILABLE: code = 0x0A; break;
		case GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND: code = 0x0B; break;
		default: break;
	}
	output.push_back(code);
	return Util::AddCRC(output);
}

// Leer salidas digitales
std::vector<byte> ModbusServer::fc01(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int first_coil = Util::ToInt(input[2], input[3]);
	int n_coils = Util::ToInt(input[4], input[5]);

	if (first_coil < 0 || first_coil >= N_DIGITAL_OUTPUTS)return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (first_coil + n_coils > N_DIGITAL_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	
	int data_bytes = (byte) (ceil( (float) n_coils / 8 ));
	output.push_back(data_bytes);

	int start = first_coil, coils_read = 0;
	for (int i = 0; i < data_bytes; i++) {
		byte value = BYTE_ZERO;
		int to_read = n_coils - coils_read >= 8 ? 8 : n_coils - coils_read;
		int end = start + to_read;
		for (int current = end-1; current >= start; current--) {
			if (coils_read == n_coils) break;
			value <<= 1;
			if (digital_output[current])
				value ^= BYTE_ONE;
			else
				value ^= BYTE_ZERO;
			//std::cout << digital_output[current] << std::endl;
			//std::cout << std::bitset<8>(value) << std::endl;
			coils_read++;
		}
		start += to_read;
		output.push_back(value);
	}
	return Util::AddCRC(output);
}

// Leer entradas digitales
std::vector<byte> ModbusServer::fc02(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int first_coil = Util::ToInt(input[2], input[3]);
	int n_coils = Util::ToInt(input[4], input[5]);

	if (first_coil < 0 || first_coil >= N_DIGITAL_INPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (first_coil + n_coils > N_DIGITAL_INPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	
	int data_bytes = (byte) (ceil( (float) n_coils / 8 ));
	output.push_back(data_bytes);

	int start = first_coil, coils_read = 0;
	for (int i = 0; i < data_bytes; i++) {
		byte value = BYTE_ZERO;
		int to_read = n_coils - coils_read >= 8 ? 8 : n_coils - coils_read;
		int end = start + to_read;
		for (int current = end-1; current >= start; current--) {
			if (coils_read == n_coils) break;
			value <<= 1;
			if (digital_input[current])
				value ^= BYTE_ONE;
			else
				value ^= BYTE_ZERO;
			coils_read++;
		}
		start += to_read;
		output.push_back(value);
	}
	return Util::AddCRC(output);
}

// Leer salidas analógicas
std::vector<byte> ModbusServer::fc03(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int data_address = Util::ToInt(input[2], input[3]);
	int n_registers = Util::ToInt(input[4], input[5]);

	if (data_address < 0 || data_address >= N_ANALOG_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (data_address + n_registers > N_ANALOG_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);

	output.push_back((byte)(2 * n_registers));
	for (int i = 0; i < n_registers; i++) {
		output.push_back((char)(analog_output[data_address + i] >> 8));
		output.push_back((char)analog_output[data_address + i]);
	}
	return Util::AddCRC(output);
}

// Leer entradas analógicas
std::vector<byte> ModbusServer::fc04(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int data_address = Util::ToInt(input[2], input[3]);
	int n_registers = Util::ToInt(input[4], input[5]);

	if (input.size() != 8) return Exception(ILLEGAL_DATA_VALUE, input);
	if (data_address < 0 || data_address >= N_ANALOG_INPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (data_address + n_registers > N_ANALOG_INPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);

	output.push_back((byte)(2 * n_registers));
	for (int i = 0; i < n_registers; i++)
		output.push_back((byte)analog_input[data_address + i]);
	return Util::AddCRC(output);
}

// Escribir salida digital única
std::vector<byte> ModbusServer::fc05(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int coil_address = Util::ToInt(input[2], input[3]);
	int status = Util::ToInt(input[4], input[5]);

	if (coil_address < 0 || coil_address >= N_DIGITAL_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (status != 0xFF00 && status != 0x0000) return Exception(ILLEGAL_DATA_VALUE, input);

	digital_output[coil_address] = status == 0xFF00;
	return Util::AddCRC(output);
}

// Escribir salida analógica única
std::vector<byte> ModbusServer::fc06(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int coil_address = Util::ToInt(input[2], input[3]);
	int value = Util::ToInt(input[4], input[5]);

	if (coil_address < 0 || coil_address >= N_ANALOG_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);

	analog_output[coil_address] = value;
	return Util::AddCRC(output);
}

// Escribir salidas digitales múltiples
std::vector<byte> ModbusServer::fc15(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int first_coil = Util::ToInt(input[2], input[3]);
	int n_coils = Util::ToInt(input[4], input[5]);
	int data_bytes = (byte) input[6];

	if (first_coil < 0 || first_coil >= N_DIGITAL_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (first_coil + n_coils > N_DIGITAL_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);

	int start = first_coil, coils_read = 0;
	for (int i = 0; i < data_bytes; i++) {
		int to_read = n_coils - coils_read >= 8 ? 8 : n_coils - coils_read;
		int end = start + to_read;
		int byte = (int)input[i+7];
		for (int current = start; current < end; current++) {
			if (coils_read == n_coils) break;
			bool coil = byte & BYTE_ONE;
			byte >>= 1;
			digital_output[current] = coil;
			coils_read++;
		}
		start += to_read;
	}
	return Util::AddCRC(output);
}

// Escribir salidas analógicas múltiples
std::vector<byte> ModbusServer::fc16(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int first_register = Util::ToInt(input[2], input[3]);
	int n_registers = Util::ToInt(input[4], input[5]);
	int data_bytes = (byte) input[6];

	if (first_register < 0 || first_register >= N_ANALOG_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (first_register + n_registers > N_ANALOG_OUTPUTS) return Exception(ILLEGAL_DATA_ADDRESS, input);
	if (n_registers*2 != data_bytes) return Exception(ILLEGAL_DATA_VALUE, input);

	std::vector<int> values;
	for (int i = 0; i < data_bytes; i += 2) {
    	int a = (int) Util::ToInt(input[7+i], input[7+i+1]);
		values.push_back(a);
	}
	for (int i = 0; i < n_registers; i++) analog_output[first_register+i] = values[i];

	return Util::AddCRC(output);
}

// Es la petición especificada para este dispositivo
bool ModbusServer::IsRequestForMe(std::vector<byte> input) {
	return id == input[0];
}

// Devolver ID
byte ModbusServer::GetID() {
	return id;
}

// Escribir 'value' en la posición 'position' de las entradas digitales
void ModbusServer::SetDigitalInput(int position, int value) {
	digital_input[position] = value != 0 ? 1 : 0;
}

// Escribir 'value' en la posición 'position' de las entradas analógicas
void ModbusServer::SetAnalogInput(int position, int value) {
	analog_input[position] = value;
}

// Imprimir los registros
// El modo será un entero 0-15 representado por 4 bits
// [Analog] [Digital] [Input] [Output]
// Ejemplos:
//			Analog Input --> 1010 = 10
//			Analog Digital Input Output --> 1111 = 15
//			Digital Input Output --> 0111 = 7
void ModbusServer::Print(int mode) {
	std::stringstream ss;

	if (mode == 15 || mode == 0 || mode == 8 || mode == 10 || mode == 3 || mode == 2 || mode == 11|| mode == 12 || mode == 14) {
		ss << "Analog Inputs:\n";
		for (int i = 0; i < analog_input.size(); i++)
			ss << "\t[" << i+1 << "] = " << analog_input[i] << std::endl;
	}
	if (mode == 15 || mode == 0 || mode == 4 || mode == 6 || mode == 3 || mode == 2 || mode == 7|| mode == 12 || mode == 14) {
		ss << "Digital Inputs:\n";
		for (int i = 0; i < digital_input.size(); i++)
			ss << "\t[" << i+1 << "] = " << digital_input[i] << std::endl;
	}
	if (mode == 15 || mode == 0 || mode == 8 || mode == 9 || mode == 3 || mode == 1 || mode == 11 || mode == 12 || mode == 13) {
		ss << "Analog Outputs:\n";
		for (int i = 0; i < analog_output.size(); i++)
			ss << "\t[" << i+1 << "] = " << analog_output[i] << std::endl;
	}
	if (mode == 15 || mode == 0 || mode == 4 || mode == 5 || mode == 3 || mode == 1 || mode == 7 || mode == 12 || mode == 13) {
		ss << "Digital Outputs:\n";
		for (int i = 0; i < digital_output.size(); i++)
			ss << "\t[" << i+1 << "] = " << digital_output[i] << std::endl;
	}
	std::cout << ss.str();
}

// Sobrecarga operador <<
std::ostream& operator<<(std::ostream& os, const ModbusServer& mbs)
{
	os << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)mbs.id << std::dec;
    os << std::resetiosflags(std::ios::showbase);
    return os;
}

} /* namespace modbus */
