#include "ModbusServer.hpp"

namespace modbus {

class ModbusTCP {
private:
    std::vector<ModbusServer*> devices;
    ModbusServer* selected_device = NULL;
    int sockfd; // Socket file descriptor
	const int portno = 1502; // Port number
	sockaddr_in serv_addr; // Server address
    std::thread t_server;
    std::vector<std::thread> t_clients;
    std::atomic<bool> threads_stop;
    void CommandPrompt();
    void ProcessCommand(std::string input);
    void Listen();
    void HandleRequest(int newsockfd, sockaddr_in* cli_addr);
    ModbusServer* GetDevice(std::vector<byte> input);
    ModbusServer* Device(byte id);
public:
    ModbusTCP();
    ~ModbusTCP();
    void Start();
    void Stop();
    void Restart();
    void AddDevice(byte id);
    void AddDevice(ModbusServer* mbs);
};

}