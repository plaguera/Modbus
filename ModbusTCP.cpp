#include "ModbusTCP.hpp"

namespace modbus {

    ModbusTCP::ModbusTCP() : devices(std::vector<ModbusServer*>()), sockfd(-1) {
    }

    ModbusTCP::~ModbusTCP() {
        for (int i = 0; i < devices.size(); i++)
            delete devices[i];
    }

    void ModbusTCP::Start() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create new socket, save file descriptor
        if (sockfd < 0) {
            Util::Error(UNABLE_TO_OPEN_SOCKET);
            return;
        }

        int reusePort = 1; // Disables default "wait time" after port is no longer in use before it is unbound
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));

        bzero((char *) &serv_addr, sizeof(serv_addr)); // Initialize serv_addr to zeros

        serv_addr.sin_family = AF_INET; // Sets the address family
        serv_addr.sin_port = htons(portno); // Converts number from host byte order to network byte order
        serv_addr.sin_addr.s_addr = INADDR_ANY; // Sets the IP address of the machine on which this server is running

        if (bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            Util::Error(UNABLE_TO_BIND);
            return;
        }

        unsigned int backlogSize = 5; // Number of connections that can be waiting while another finishes
        listen(sockfd, backlogSize);
        std::cout << Util::Color(MAGENTA, "C++ Server Opened on Port " + portno) << std::endl;

        threads_stop = false;
        t_server = std::thread(&ModbusTCP::Listen, this);
        t_server.detach();
        CommandPrompt();
    }

    void ModbusTCP::CommandPrompt() {
        std::string input;
        while (!threads_stop) {
            //std::cout << "> $ ";
            std::getline(std::cin, input);
            ProcessCommand(input);
        }
    }

    void ModbusTCP::ProcessCommand(std::string input) {
        if (input.length() == 0 || std::all_of(input.begin(),input.end(),isspace)) return;
        selected_device->Update();
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        Command cmd = Util::TokenizeCommand(input);
        switch (cmd.command)
        {
        case DEVICES:
            if(!regex_match(input, std::regex("(devices)"))) {
                Util::Error(BAD_COMMAND_SYNTAX);
                break;
            }
            for (int i = 0; i < devices.size(); i++) {
                if (devices[i] == selected_device)
                    std::cout << i+1 << ".- [" << *devices[i] << "]" << std::endl;
                else
                    std::cout << i+1 << ".- " << *devices[i] << std::endl;
            }
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
                    Util::Error(INVALID_DATA);
                    break;
                }
                std::cout << "Device " << *selected_device << " << " << (analog ? "Analog" : "Digital") << "[" << position << "] >> = ";
                std::cin >> value;
                if (!analog && value != 0 && value != 1) {
                    Util::Error(INVALID_DATA);
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
                ModbusServer* device = Device(Util::ToByte(cmd.args[0]));
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

    void ModbusTCP::Listen() {
        while (!threads_stop) {
            int newsockfd; // New socket file descriptor
            unsigned int cli_len; // Client address size
            sockaddr_in cli_addr; // Client address

            cli_len = sizeof(sockaddr_in);
            newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len); // Block until a client connects
            if (newsockfd < 0) {
                 Util::Error(UNABLE_TO_ACCEPT);
                 Stop();
            }
            std::string aux(inet_ntoa(cli_addr.sin_addr) + std::string(" : ") + std::to_string(ntohs(cli_addr.sin_port)));
            std::cout << Util::FormatSockAddr(&cli_addr) << " - " << Util::Color(BOLD, Util::Color(GREEN, "Connected")) << std::endl;

            t_clients.push_back(std::thread(&ModbusTCP::HandleRequest, this, newsockfd, &cli_addr));
            t_clients.back().detach();
        }
    }

    // echo -en '\x06\x03\x00\x00\x00\x01\x85\xBD' | nc localhost 1502 | xxd -ps -u | sed 's/.\{2\}/0x& /g'
    void ModbusTCP::HandleRequest(int newsockfd, sockaddr_in* cli_addr) {
        char buffer[1024];
        ssize_t bytes_recieved = 0, bytes_sent;
        do {
            bytes_recieved = recv(newsockfd, buffer, 1024, 0);
            if (bytes_recieved <= 0) break;

            byte* array = (byte*) buffer;
            std::vector<byte> input(array, array + sizeof(array)/sizeof(byte));

            std::cout << Util::FormatSockAddr(cli_addr) + " - {Input}" << Util::ToString(input) << std::endl;
            std::vector<byte> output = ProcessPetition(input);
            if (!output.empty())
                std::cout << "Response: " + Util::ToString(output) << std::endl;

            const char* foo = reinterpret_cast<const char*>(output.data());
            bytes_sent = send(newsockfd, foo, output.size(), 0);
            if (bytes_sent <= 0) break;
            
        } while (bytes_recieved > 0);
        // If no data arrives, the program will just wait here until some data arrives.
        if (bytes_recieved == 0) std::cout << Util::FormatSockAddr(cli_addr) << " - " << Util::Color(BOLD, Util::Color(RED, "Disconnected")) << std::endl;
        if (bytes_recieved == -1)  Util::Error(RECEIVE_ERROR);
        close(newsockfd);
    }

    std::vector<byte> ModbusTCP::ProcessPetition(std::vector<byte> input) {
        ModbusServer* device = Device(input[0]);
        if (device != NULL)
            return device->Petition(input);
        return std::vector<byte>();
    }

    void ModbusTCP::Stop() {
        threads_stop = true;
        sockfd = -1;
        std::cout << Util::Color(RED, "Stopping server...") << std::endl;
        close(sockfd);
    }

    void ModbusTCP::Restart() {
        Stop();
    }

    void ModbusTCP::AddDevice(byte id) {
        AddDevice(new ModbusServer(id));
    }

    void ModbusTCP::AddDevice(ModbusServer* mbs) {
        devices.push_back(mbs);
        if (devices.size() == 1) selected_device = mbs;
    }

    ModbusServer* ModbusTCP::GetDevice(std::vector<byte> input) {
        for (int i = 0; i < devices.size(); i++)
            if (devices[i]->IsRequestForMe(input))
                return devices[i];
        Util::Error(INVALID_DEVICE_ID);
        return NULL;
    }

    ModbusServer* ModbusTCP::Device(byte id) {
        for (int i = 0; i < devices.size(); i++)
            if (devices[i]->GetID() == id)
                return devices[i];
        Util::Error(INVALID_DEVICE_ID);
        return NULL;
    }

}