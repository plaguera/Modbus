/*
 * Util.hpp
 *
 *  Created on: Apr 12, 2019
 *      Author: pedro
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

#include <algorithm>
#include <arpa/inet.h>
#include <atomic>
#include <bitset>
#include <csignal>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <iostream>
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

enum e_command { STOP, GET, SET, SELECT, DEVICES, HELP, UNKNOWN };

enum e_mode { ANALOG, DIGITAL };

struct Command
{
    e_command command;
    std::vector<std::string> args;
	Command(e_command cmd, std::vector<std::string> args) : command(cmd), args(args) {}
};

class Util {
public:
	static std::map<std::string, e_command> map_command;
	static int ToInt(byte lsb, byte msb);
	static byte ToByte(std::string string);
	static bool IsInt(std::string string);
	static std::vector<byte> CRC(std::vector<byte> input);
	static std::vector<byte> AddCRC(std::vector<byte> input);
	static bool CheckCRC(std::vector<byte> input);
	static void PrintByteVector(std::vector<byte> input);
	static std::string RemoveWhitespaces(std::string input);
	static Command TokenizeCommand(const std::string& str, const std::string& delimiters = " ");
};

}

#endif /* UTIL_H_ */
