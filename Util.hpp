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

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLD   "\033[1m"      /* Bold */

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

enum e_command { STOP, GET, SET, SELECT, DEVICES, HELP, UNKNOWN };
enum e_error { FUNCTION_NOT_IMPLEMENTED = 1, RESGISTRY_OUT_OF_RANGE = 2, INVALID_DATA = 3, BAD_COMMAND_SYNTAX, COMMAND_NOT_FOUND, INVALID_DEVICE_ID, INVALID_CRC, UNABLE_TO_OPEN_SOCKET, UNABLE_TO_BIND, UNABLE_TO_ACCEPT, RECEIVE_ERROR };
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
	static std::string ToString(byte value);
	static std::string ToString(std::vector<byte> input);
	static std::string Color(std::string color, std::string string);
	static std::string FormatSockAddr(sockaddr_in* addr);
	static bool IsInt(std::string string);
	static std::vector<byte> CRC(std::vector<byte> input);
	static std::vector<byte> AddCRC(std::vector<byte> input);
	static bool CheckCRC(std::vector<byte> input);
	static std::string RemoveWhitespaces(std::string input);
	static Command TokenizeCommand(const std::string& str, const std::string& delimiters = " ");
	static void Error(e_error code, std::string text);
	static void Exception(e_error code, std::string text);

	static void Error(e_error code);
	static void Exception(e_error code);
};

}

#endif /* UTIL_H_ */
