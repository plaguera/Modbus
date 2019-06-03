/*
 * ModbusServer.hpp
 *
 *  Created on: Apr 12, 2019
 *      Author: Pedro Lagüera Cabrera | alu0100891485
 */
#include "ModbusServer.hpp"

namespace modbus {

class ModbusTCP {
private:
    std::vector<ModbusServer*> devices;                             // Lista de dispositivos disponibles
    ModbusServer* selected_device = NULL;                           // Dispositivo seleccionado en la línea de comandos
    int sockfd;                                                     // Socket file descriptor
	const int portno = 1502;                                        // Puerto
	sockaddr_in serv_addr;                                          // Dirección del servidor
    std::thread t_server;                                           // Hilo de ejecución de escucha para clientes nuevos
    std::vector<std::thread> t_clients;                             // Hilos de ejecución de cada cliente conectado
    std::atomic<bool> threads_stop;                                 // Variable atómica que para todos los hilos
    void CommandPrompt();                                           // Inicia la línea de comandos del programa
    void ProcessCommand(std::string input);                         // Parsea y procesa texto de entrada de la línea de comandos
    std::vector<byte> ProcessPetition(std::vector<byte> input);     // Genera una respuesta a partir de una petición Modbus
    void Listen();                                                  // Escucha constatemente por nuevos clientes en un hilo dedicado
    void HandleRequest(int newsockfd, sockaddr_in* cli_addr);       // Escucha en cada hilo por los mensajes de cada cliente conectado
    ModbusServer* Device(byte id);                                  // Devuelve el objeto del dispositivo del id correspondiente
public:
    ModbusTCP();                                                    // Constructor
    ~ModbusTCP();                                                   // Destructor
    void Start();                                                   // Iniciar servidor
    void Stop();                                                    // Parar servidor
    void AddDevice(byte id);                                        // Añadir dispositivo según su id
    void AddDevice(ModbusServer* mbs);                              // Añadir dispositivo según su objeto
};

}