/*
 * Util.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: pedro
 */

#include "Util.hpp"

namespace modbus {

std::map<std::string, e_command> Util::map_command = { 	{ COMMAND_DEVICES, DEVICES },
														{ COMMAND_GET, GET },
														{ COMMAND_HELP, HELP },
														{ COMMAND_SELECT, SELECT },
														{ COMMAND_SET, SET },
														{ COMMAND_STOP, STOP } };

int Util::ToInt(byte lsb, byte msb) {
	return msb | lsb << 8;
}

byte Util::ToByte(std::string string) {
	std::stringstream str;
	str << string;
	int value;
	str >> std::hex >> value;
	return (byte)value;
}

std::string Util::ToString(byte value) {
	std::stringstream ss;
	ss << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)value;
	return ss.str();
}

bool Util::IsInt(std::string string) {
	return std::all_of(string.begin(), string.end(), ::isdigit);
}

std::vector<byte> Util::CRC(std::vector<byte> input) {
	unsigned int crc = 0xFFFF;
	for (int i = 0; i < input.size(); i++) {
		crc ^= input[i];
		for (int j = 0; j < 8; j++) {
			if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
			else crc >>= 1;
		}
	}
	std::vector<byte> output;
	output.push_back((byte)(crc & 0xFF));
	output.push_back((byte)(crc >> 8));
	return output;
}

std::vector<byte> Util::AddCRC(std::vector<byte> input) {
	std::vector<modbus::byte> crc = Util::CRC(input);
	for (int i = 0; i < crc.size(); i++)
    	input.push_back(crc[i]);
	return input;
}

bool Util::CheckCRC(std::vector<byte> input) {
	byte lsb = input.back();
	input.pop_back();
	byte msb = input.back();
	input.pop_back();
	std::vector<byte> crc = CRC(input);
	return crc[0] == msb && crc[1] == lsb;
}

std::string Util::ToString(std::vector<byte> input) {
	std::stringstream ss;
	ss << "[("  << input.size() << ") ";
	for (int i = 0; i < input.size(); i++)
		ss << Util::ToString(input[i]) << " ";
	ss << "]" << std::dec;
	return ss.str();
}

std::string Util::Color(std::string color, std::string string) {
	std::stringstream ss;
	ss << color << string << RESET;
	return ss.str(); 
}

std::string Util::FormatSockAddr(sockaddr_in* addr) {
	std::string aux(std::string("[ ") + inet_ntoa(addr->sin_addr) + std::string(" : ") + std::to_string(ntohs(addr->sin_port)) + std::string(" ]"));
    return Util::Color(BOLD, Util::Color(BLUE, aux));
}

std::string Util::RemoveWhitespaces(std::string input) {
	std::string output("");
	for (int i = 0; i < input.length(); i++)
		if (input[i] != ' ')
			output += input[i];
	std::cout << input.length() << "\n";
	return output;
}

Command Util::TokenizeCommand(const std::string& str, const std::string& delimiters) {
	std::vector<std::string> tokens;
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
	e_command e_cmd = map_command.find(tokens.front()) != map_command.end() ? map_command[tokens.front()] : UNKNOWN;
	Command cmd(e_cmd, std::vector<std::string>(tokens.begin()+1, tokens.end()));
	return cmd;
}

void Util::Error(e_error code, std::string text) {
	std::cerr << RED << "ERROR [" << code << "]: " << text << RESET << std::endl;
}

void Util::Error(e_error code) {
	std::string text;
	switch (code) {
		case FUNCTION_NOT_IMPLEMENTED: text = "Function Not Implemented"; break;
		case RESGISTRY_OUT_OF_RANGE: text = "Registry Out of Range"; break;
		case INVALID_DATA: text = "Invalid Request Data"; break;
		case BAD_COMMAND_SYNTAX: text = "Bad Command Syntax"; break;
		case COMMAND_NOT_FOUND: text = "Command Not Found"; break;
		case INVALID_DEVICE_ID: text = "Invalid Device ID"; break;
		case INVALID_CRC: text = "Invalid CRC Code"; break;
		case UNABLE_TO_OPEN_SOCKET: text = "Unable to Open Socket"; break;
		case UNABLE_TO_BIND: text = "Unable to Bind"; break;
		case UNABLE_TO_ACCEPT: text = "Unable to Accept"; break;
		case RECEIVE_ERROR: text = "Receive Error"; break;
		default: break;
	}
	std::cerr << BOLD << RED << "ERROR [" << code << "]: " << text << RESET << std::endl;
}

void Util::Exception(e_error code, std::string text) {
	std::stringstream ss;
	ss << RED << "ERROR [" << code << "]: " << text << RESET;
	throw ss.str();
}

} /* namespace modbus */
