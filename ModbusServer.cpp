/*
 * ModbusServer.cpp
 *
 *  Created on: Mar 21, 2019
 *      Author: pedro
 */
#include "ModbusServer.hpp"

namespace modbus {

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

ModbusServer::~ModbusServer() {
	// TODO Auto-generated destructor stub
}

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

std::vector<byte> ModbusServer::Petition(std::vector<byte> input) {
	if (!IsRequestForMe(input)) throw "Incorrect ID !!";
	if (!Util::CheckCRC(input)) throw "Incorrect CRC Value !!";
	Update();
	std::vector<byte> output;
	switch(input[1]) {
		case 0x01: output = fc01(input); break;
		case 0x02: output = fc02(input); break;
		case 0x03: output = fc03(input); break;
		case 0x04: output = fc04(input); break;
		case 0x05: output = fc05(input); break;
		case 0x06: output = fc06(input); break;
		case 0x15: output = fc15(input); break;
		case 0x16: output = fc16(input); break;
		default: throw "No such operation !!";
	}
	analog_input[0]++;
	analog_input[1] += input.size();
	analog_input[2] += output.size();
	return output;
}

std::vector<byte> ModbusServer::fc01(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int first_coil = Util::ToInt(input[2], input[3]);
	int n_coils = Util::ToInt(input[4], input[5]);

	if (first_coil < 0 || first_coil >= N_DIGITAL_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (first_coil + n_coils >= N_DIGITAL_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	
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
			std::cout << digital_output[current] << std::endl;
			std::cout << std::bitset<8>(value) << std::endl;
			coils_read++;
		}
		start += to_read;
		output.push_back(value);
	}
	return Util::AddCRC(output);
}

std::vector<byte> ModbusServer::fc02(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int first_coil = Util::ToInt(input[2], input[3]);
	int n_coils = Util::ToInt(input[4], input[5]);

	if (first_coil < 0 || first_coil >= N_DIGITAL_INPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (first_coil + n_coils >= N_DIGITAL_INPUTS) throw "Invalid Data: Register is Out of Range !!";
	
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

std::vector<byte> ModbusServer::fc03(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int data_address = Util::ToInt(input[2], input[3]);
	int n_registers = Util::ToInt(input[4], input[5]);

	if (data_address < 0 || data_address >= N_ANALOG_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (data_address + n_registers >= N_ANALOG_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	
	output.push_back((byte)(2 * n_registers));
	for (int i = 0; i < n_registers; i++) {
		output.push_back((char)(analog_output[data_address + i] >> 8));
		output.push_back((char)analog_output[data_address + i]);
	}
	return Util::AddCRC(output);
}

std::vector<byte> ModbusServer::fc04(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1] };
	int data_address = Util::ToInt(input[2], input[3]);
	int n_registers = Util::ToInt(input[4], input[5]);

	if (input.size() != 8) throw "Invalid Request !!";
	if (data_address < 0 || data_address >= N_ANALOG_INPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (data_address + n_registers >= N_ANALOG_INPUTS) throw "Invalid Data: Register is Out of Range !!";

	output.push_back((byte)(2 * n_registers));
	for (int i = 0; i < n_registers; i++)
		output.push_back((byte)analog_input[data_address + i]);
	return Util::AddCRC(output);
}

std::vector<byte> ModbusServer::fc05(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int coil_address = Util::ToInt(input[2], input[3]);
	int status = Util::ToInt(input[4], input[5]);

	if (coil_address < 0 || coil_address >= N_DIGITAL_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (status != 0xFF00 && status != 0x0000) throw "Invalid Data: Value is not 0xFF00 or 0x0000 !!";

	digital_output[coil_address] = status == 0xFF00;
	return Util::AddCRC(output);
}

std::vector<byte> ModbusServer::fc06(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int coil_address = Util::ToInt(input[2], input[3]);
	int value = Util::ToInt(input[4], input[5]);

	if (coil_address < 0 || coil_address >= N_ANALOG_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";

	analog_output[coil_address] = value;
	return Util::AddCRC(output);
}

std::vector<byte> ModbusServer::fc15(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int first_coil = Util::ToInt(input[2], input[3]);
	int n_coils = Util::ToInt(input[4], input[5]);
	int data_bytes = (byte) input[6];

	if (first_coil < 0 || first_coil >= N_DIGITAL_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (first_coil + n_coils >= N_DIGITAL_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";

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

std::vector<byte> ModbusServer::fc16(std::vector<byte> input) {
	std::vector<byte> output { input[0], input[1], input[2], input[3], input[4], input[5] };
	int first_register = Util::ToInt(input[2], input[3]);
	int n_registers = Util::ToInt(input[4], input[5]);
	int data_bytes = (byte) input[6];

	if (first_register < 0 || first_register >= N_ANALOG_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (first_register + n_registers >= N_ANALOG_OUTPUTS) throw "Invalid Data: Register is Out of Range !!";
	if (input.size() != (unsigned)n_registers + 9) throw "Invalid Request !!";

	std::vector<int> values;
	for (int i = 0; i < data_bytes; i += 2) {
    	int a = (int) Util::ToInt(input[7+i], input[7+i+1]);
		values.push_back(a);
	}
	for (int i = 0; i < values.size(); i++)
	std::cout << values[i] << " ";
	std::cout << std::endl;
	for (int i = 0; i < data_bytes; i++) analog_output[first_register+i] = values[i];

	return Util::AddCRC(output);
}

bool ModbusServer::IsRequestForMe(std::vector<byte> input) {
	return id == input[0];
}

byte ModbusServer::GetID() {
	return id;
}

void ModbusServer::SetDigitalInput(int position, int value) {
	digital_input[position] = value != 0 ? 1 : 0;
}

void ModbusServer::SetAnalogInput(int position, int value) {
	analog_input[position] = value;
}

void ModbusServer::Print(int mode) {
	std::stringstream ss;

	if (mode == 15 || mode == 0 || mode == 8 || mode == 10 || mode == 3 || mode == 2) {
		ss << "Analog Inputs:\n";
		for (int i = 0; i < analog_input.size(); i++)
			ss << "\t[" << i+1 << "] = " << analog_input[i] << std::endl;
	}
	if (mode == 15 || mode == 0 || mode == 4 || mode == 6 || mode == 3 || mode == 2) {
		ss << "Digital Inputs:\n";
		for (int i = 0; i < digital_input.size(); i++)
			ss << "\t[" << i+1 << "] = " << digital_input[i] << std::endl;
	}
	if (mode == 15 || mode == 0 || mode == 8 || mode == 9 || mode == 3 || mode == 1) {
		ss << "Analog Outputs:\n";
		for (int i = 0; i < analog_output.size(); i++)
			ss << "\t[" << i+1 << "] = " << analog_output[i] << std::endl;
	}
	if (mode == 15 || mode == 0 || mode == 4 || mode == 5 || mode == 3 || mode == 1) {
		ss << "Digital Outputs:\n";
		for (int i = 0; i < digital_output.size(); i++)
			ss << "\t[" << i+1 << "] = " << digital_output[i] << std::endl;
	}
	std::cout << ss.str();
}

std::ostream& operator<<(std::ostream& os, const ModbusServer& mbs)
{
	os << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)mbs.id << std::dec;
    os << std::resetiosflags(std::ios::showbase);
    return os;
}

} /* namespace modbus */
