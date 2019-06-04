/*
 * ModbusServer.hpp
 *
 *  Created on: Mar 21, 2019
 *      Author: Pedro Lagüera Cabrera | alu0100891485
 */
#ifndef MODBUSSERVER_H_
#define MODBUSSERVER_H_

#include "Util.hpp"

namespace modbus {

// Clase que representa un dispositivo Modbus
class ModbusServer {
private:
	byte id;							// Identificador
	std::clock_t time_start;			// Fecha de creación
	std::vector<bool> digital_input;	// Entradas Digitales
	std::vector<bool> digital_output;	// Salidas Digitales
	std::vector<int> analog_input;		// Entradas Analógicas
	std::vector<int> analog_output;		// Salidas Analógicas
public:
	ModbusServer(int id);				// Crear dispositivo a partir de ID en decimal
	ModbusServer(byte id);				// Crear dispositivo a partir de ID en byte
	~ModbusServer();					// Destructor
	void Update();						// Actualizar los registros
	std::vector<byte> Petition(std::vector<byte> input);	// Procesar una petición y devolver una respuesta
	std::vector<byte> fc01(std::vector<byte> input);		// Leer salidas digitales
	std::vector<byte> fc02(std::vector<byte> input);		// Leer entradas digitales
	std::vector<byte> fc03(std::vector<byte> input);		// Leer salidas analógicas
	std::vector<byte> fc04(std::vector<byte> input);		// Leer entradas analógicas
	std::vector<byte> fc05(std::vector<byte> input);		// Escribir salida digital única
	std::vector<byte> fc06(std::vector<byte> input);		// Escribir salida analógica única
	std::vector<byte> fc15(std::vector<byte> input);		// Escribir salidas digitales múltiples
	std::vector<byte> fc16(std::vector<byte> input);		// Escribir salidas analógicas múltiples
	std::vector<byte> Exception(e_exception exception, std::vector<byte> input);	// Genera respuesta adecuada en caso de excepción en una fc
	bool IsRequestForMe(std::vector<byte> input);			// Es la petición especificada para este dispositivo
	void Print(int mode);									// Imprimir los registros
	byte GetID();											// Devolver ID
	void SetDigitalInput(int position, int value);			// Escribir 'value' en la posición 'position' de las entradas digitales
	void SetAnalogInput(int position, int value);			// Escribir 'value' en la posición 'position' de las entradas analógicas
	friend std::ostream& operator<<(std::ostream& os, const ModbusServer& mbs);	// Sobrecarga operador <<
};

} /* namespace modbus */

#endif /* MODBUSSERVER_H_ */
