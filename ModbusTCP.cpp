/*
 * ModbusTCP.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: Pedro Lagüera Cabrera | alu0100891485
 */
#include "ModbusTCP.hpp"

namespace modbus {

    // Inicializar el servidor
    ModbusTCP::ModbusTCP(std::vector<int> ids) : devices(std::vector<ModbusServer*>()), sockfd(-1) {
        for (int i = 0; i < ids.size(); i++)
            AddDevice(ids[i]);
    }

    // Destruir todos los dispositivos existentes
    ModbusTCP::~ModbusTCP() {
        for (int i = 0; i < devices.size(); i++)
            delete devices[i];
    }

    // Inicializa y arranca los hilos de ejecución y los sockets
    void ModbusTCP::Start() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0); // Crear un nuevo socket, guardar el file descriptor
        if (sockfd < 0) {
            Util::Exception(UNABLE_TO_OPEN_SOCKET);
            return;
        }

        int reusePort = 1; // Disables default "wait time" after port is no longer in use before it is unbound
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));

        bzero((char *) &serv_addr, sizeof(serv_addr)); // Inicializar serv_addr con ceros

        serv_addr.sin_family = AF_INET; // Address family
        serv_addr.sin_port = htons(portno); // Convierte el número de puerto de host byte order a network byte order
        serv_addr.sin_addr.s_addr = INADDR_ANY; // Guarda la IP de la máquina donde se ejecute el servidor

        // Asignar los parámetros de dirección al socket abierto
        if (bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            Util::Error(UNABLE_TO_BIND);
            return;
        }

        unsigned int backlogSize = 5; // Número de conexiones que pueden estar en espera a que finalice otra
        listen(sockfd, backlogSize);
        std::cout << Util::Color(MAGENTA, "C++ Server Opened on Port " + std::to_string(portno)) << std::endl;

        threads_stop = false;
        t_server = std::thread(&ModbusTCP::Listen, this); // Lanzar la escucha de nuevos clientes en un hilo independiente
        t_server.detach();
        CommandPrompt(); // Ejecutar línea de comandos en el hilo principal de ejecución
    }

    // Permite input de comandos y los procesa
    void ModbusTCP::CommandPrompt() {
        std::string input;
        while (!threads_stop) {
            //std::cout << "> $ ";
            std::getline(std::cin, input);
            ProcessCommand(input);
        }
    }

    // Parsea el texto introducido por línea de comandos y ejecuta las acciones correspondientes
    void ModbusTCP::ProcessCommand(std::string input) {
        if (input.length() == 0 || std::all_of(input.begin(),input.end(),isspace)) return;  // Texto vacío o espacios se descarta
        selected_device->Update();                                                          // Actualizar el dispositivo seleccionado
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);               // Transformar texto a minúsculas
        Command cmd = Util::TokenizeCommand(input);                                         // Parsear texto y obtener un comando
        switch (cmd.command)
        {
        case DEVICES:
            if(!regex_match(input, std::regex("(devices)"))) {
                Util::Error(BAD_COMMAND_SYNTAX);
                break;
            }
            for (int i = 0; i < devices.size(); i++) {
                if (devices[i] == selected_device)
                    std::cout << i+1 << ".- " << BOLD << *devices[i] << "*" << RESET << std::endl;
                else
                    std::cout << i+1 << ".- " << *devices[i] << std::endl;
            }
            break;

        case CONNECTIONS:
            if(!regex_match(input, std::regex("(connections)"))) {
                Util::Error(BAD_COMMAND_SYNTAX);
                break;
            }
            for (int i = 0; i < clients.size(); i++)
                std::cout << Util::FormatSockAddr(clients[i].address) << std::endl;
            break;

        case STOP:
            if(!regex_match(input, std::regex("(stop)"))) {
                Util::Error(BAD_COMMAND_SYNTAX);
                break;
            }
            Stop();
            break;

        case GET:
            {
                if(!regex_match(input, std::regex("(get)\\s*(((analog)|(digital)|(input)|(output))\\s*)*"))) {
                    Util::Error(BAD_COMMAND_SYNTAX);
                    break;
                }
                bool analog = std::find(cmd.args.begin(), cmd.args.end(), "analog") != cmd.args.end();
                bool digital = std::find(cmd.args.begin(), cmd.args.end(), "digital") != cmd.args.end();
                bool input = std::find(cmd.args.begin(), cmd.args.end(), "input") != cmd.args.end();
                bool output = std::find(cmd.args.begin(), cmd.args.end(), "output") != cmd.args.end();
                int mode = analog * 8 + digital * 4 + input * 2 + output * 1;
                selected_device->Print(mode);
            }
            break;

        case SET:
            {
                if(!regex_match(input, std::regex("(set)\\s+((analog)|(digital))\\s+[0-9]{1,2})"))) {
                    Util::Error(BAD_COMMAND_SYNTAX);
                    break;
                }
                bool analog = cmd.args[0] == "analog";
                int position = stoi(cmd.args[1]), value;
                if (position < 15 || position > 19) {
                    Util::Error(BAD_COMMAND_SYNTAX);
                    break;
                }
                std::cout << "Device " << *selected_device << " << " << (analog ? "Analog" : "Digital") << "[" << position << "] >> = ";
                std::cin >> value;
                if (!analog && value != 0 && value != 1) {
                    Util::Error(BAD_COMMAND_SYNTAX);
                    break;
                }
                if (analog) selected_device->SetAnalogInput(position, value);
                else selected_device->SetDigitalInput(position, value);
            }
            break;

        case SELECT:
            {
                if(!regex_match(input, std::regex("(select)\\s*((?:0[xX])?[0-9a-fA-F]+)?"))) {
                    Util::Error(BAD_COMMAND_SYNTAX);
                    break;
                }
                if (cmd.args.empty()) {
                    std::cout << *selected_device << std::endl;
                    break;
                }
                ModbusServer* device = Device(std::stoi(cmd.args[0]));
                if (device != NULL) selected_device = device;
            }
            break;
        
        case HELP:
            if(!regex_match(input, std::regex("(help)"))) {
                Util::Error(BAD_COMMAND_SYNTAX);
                break;
            }
            std::cout << "Commands:" << std::endl;
            std::cout << "\tDEVICES" << std::endl;
            std::cout << "\tGET  [Analog | Digital | Input | Output]*" << std::endl;
            std::cout << "\tHELP" << std::endl;
            std::cout << "\tSELECT [Device ID]?" << std::endl;
            std::cout << "\tSET [Analog | Digital] [position]" << std::endl;
            std::cout << "\tSTOP" << std::endl;
            break;

        case UNKNOWN:
        default: Util::Error(COMMAND_NOT_FOUND); break;
        }
    }

    // Escuchar por nuevos clientes y crear hilos nuevos para manejarlos en el caso de que se conecten
    void ModbusTCP::Listen() {
        while (!threads_stop) {
            int newsockfd;                                                  // Nuevo socket file descriptor
            sockaddr_in cli_addr;                                           // Dirección cliente
            unsigned int cli_len = sizeof(sockaddr_in);                     // Tamaño dirección cliente

            newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);   // Bloquear hasta que se conecte un cliente
            if (newsockfd < 0) {
                 Util::Error(UNABLE_TO_ACCEPT);
                 Stop();
            }
            std::string aux(inet_ntoa(cli_addr.sin_addr) + std::string(" : ") + std::to_string(ntohs(cli_addr.sin_port)));
            std::cout << Util::FormatSockAddr(&cli_addr) << " - " << Util::Color(BOLD, Util::Color(GREEN, "Connected")) << std::endl;

            // Crear hilo que ejecute 'HandleRequest' para el nuevo cliente y almacenarlo en la lista
            clients.push_back(Client(newsockfd, &cli_addr));
            clients.back().thread = std::thread(&ModbusTCP::HandleRequest, this, newsockfd, &cli_addr);
            clients.back().thread.detach();
        }
    }

    // Escucha por mensajes del cliente correspondiente y cuando los recibe, se procesan y devuelve una respuesta
    void ModbusTCP::HandleRequest(int sockfd, sockaddr_in* cli_addr) {
        char buffer[1024];
        ssize_t bytes_received = 0, bytes_sent;
        do {
            bytes_received = recv(sockfd, buffer, 1024, 0);  // Bloquear hasta que se recibe un mensaje
            if (bytes_received <= 0) break;                     // El cliente se ha desconectado

            byte* array = (byte*) buffer;                                       // Convertir el mensaje de char* a byte*
            std::vector<byte> input(array, array + sizeof(array)/sizeof(byte)); // Convertir el byte* a vector<byte>

            std::cout << Util::FormatSockAddr(cli_addr) + " - {Input}" << Util::ToString(input) << std::endl;
            std::vector<byte> output = ProcessPetition(input);                  // Procesar petición y guardar respuesta
            if (!output.empty())
                std::cout << " --> {Response}: " + Util::ToString(output) << std::endl;
            else
                std::cout << " --> {Response}: [(0) ]" << std::endl;

            const char* foo = reinterpret_cast<const char*>(output.data());     // Convertir respuesta de vector<byte> a char*
            bytes_sent = send(sockfd, foo, output.size(), 0);                   // Enviar respuesta
            if (bytes_sent < 0) break;
            
        } while (bytes_received > 0);
        
        if (bytes_received == 0) std::cout << Util::FormatSockAddr(cli_addr) << " - " << Util::Color(BOLD, Util::Color(RED, "Disconnected")) << std::endl;
        if (bytes_received == -1)  Util::Error(RECEIVE_ERROR);
        for (int i = 0; i < clients.size(); i++)
            if (clients[i].sockfd == sockfd)
                clients.erase(clients.begin() + i);                             // Eliminar cliente de la lista
        close(sockfd);                                                          // Cerrar socket
    }

    // Buscar dispositivo correspondiente a la petición y que la procese
    std::vector<byte> ModbusTCP::ProcessPetition(std::vector<byte> input) {
        ModbusServer* device = Device(input[0]);
        if (device != NULL)
            return device->Petition(input);
        return std::vector<byte>();
    }

    // Para todos los hilos de ejecución y cierre el socket principal
    void ModbusTCP::Stop() {
        threads_stop = true;
        sockfd = -1;
        std::cout << Util::Color(RED, "Stopping server...") << std::endl;
        close(sockfd);
    }

    // Añadir un nuevo dispositivo con el identificador especificado
    void ModbusTCP::AddDevice(byte id) {
        devices.push_back(new ModbusServer(id));
        if (devices.size() == 1) selected_device = devices.back();
    }

    // Devuelve el dispositivo con el identificador especificado
    ModbusServer* ModbusTCP::Device(byte id) {
        for (int i = 0; i < devices.size(); i++)
            if (devices[i]->GetID() == id)
                return devices[i];
        Util::Error(INVALID_DEVICE_ID);
        return NULL;
    }

}