/*
 * ModbusServer.hpp
 *
 *  Created on: Mar 21, 2019
 *      Author: pedro
 */
#ifndef MODBUSSERVER_H_
#define MODBUSSERVER_H_

#include "Util.hpp"

namespace modbus {

class ModbusServer {
private:
	byte id;
	std::clock_t time_start;
	std::vector<bool> digital_input;
	std::vector<bool> digital_output;
	std::vector<int> analog_input;
	std::vector<int> analog_output;
public:
	ModbusServer(byte id);
	~ModbusServer();
	void Update();
	std::vector<byte> Petition(std::vector<byte> input);
	std::vector<byte> fc01(std::vector<byte> input);
	std::vector<byte> fc02(std::vector<byte> input);
	std::vector<byte> fc03(std::vector<byte> input);
	std::vector<byte> fc04(std::vector<byte> input);
	std::vector<byte> fc05(std::vector<byte> input);
	std::vector<byte> fc06(std::vector<byte> input);
	std::vector<byte> fc15(std::vector<byte> input);
	std::vector<byte> fc16(std::vector<byte> input);
	bool IsRequestForMe(std::vector<byte> input);
	void Print(int mode);
	byte GetID();
	void SetDigitalInput(int position, int value);
	void SetAnalogInput(int position, int value);
	friend std::ostream& operator<<(std::ostream& os, const ModbusServer& mbs);
};

} /* namespace modbus */

#endif /* MODBUSSERVER_H_ */
