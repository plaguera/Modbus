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
        if (sockfd < 0)
            std::cerr << "ERROR opening socket" << std::endl;

        int reusePort = 1; // Disables default "wait time" after port is no longer in use before it is unbound
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));

        bzero((char *) &serv_addr, sizeof(serv_addr)); // Initialize serv_addr to zeros

        serv_addr.sin_family = AF_INET; // Sets the address family
        serv_addr.sin_port = htons(portno); // Converts number from host byte order to network byte order
        serv_addr.sin_addr.s_addr = INADDR_ANY; // Sets the IP address of the machine on which this server is running

        if (bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // Bind the socket to the address
            std::cerr << "ERROR on binding" << std::endl;

        unsigned int backlogSize = 5; // Number of connections that can be waiting while another finishes
        listen(sockfd, backlogSize);
        std::cout << "C++ server opened on port " << portno << std::endl;

        threads_stop = false;
        t_server = std::thread(&ModbusTCP::Listen, this);
        t_server.detach();
        CommandPrompt();
    }

    void ModbusTCP::CommandPrompt() {
        std::string input;
        while (!threads_stop) {
            std::cout << "> $ ";
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
            for (int i = 0; i < devices.size(); i++) {
                if (devices[i] == selected_device)
                    std::cout << i+1 << ".- [" << *devices[i] << "]" << std::endl;
                else
                    std::cout << i+1 << ".- " << *devices[i] << std::endl;
            }
            break;

        case STOP:
            Stop();
            break;

        case GET:
            {
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
                bool analog = cmd.args[0] == "analog";
                int position = stoi(cmd.args[1]), value;
                if (position < 15 || position > 19) {
                    std::cerr << "Invalid Position !!" << std::endl;
                    break;
                }
                std::cout << "Device " << *selected_device << " << " << (analog ? "Analog" : "Digital") << "[" << position << "] >> = ";
                std::cin >> value;
                if (!analog && value != 0 && value != 1) {
                    std::cerr << "Invalid Digital Register Value !!" << std::endl;
                    break;
                }
                if (analog) selected_device->SetAnalogInput(position, value);
                else selected_device->SetDigitalInput(position, value);
            }
            break;

        case SELECT:
            {
                if (cmd.args.empty()) {
                    std::cout << *selected_device << std::endl;
                    break;
                }
                ModbusServer* device = Device(Util::ToByte(cmd.args[0]));
                if (device != NULL) selected_device = device;
            }
            break;
        
        case HELP:
            std::cout << "Commands:" << std::endl;
            std::cout << "\tDEVICES" << std::endl;
            std::cout << "\tGET  [Analog | Digital | Input | Output]*" << std::endl;
            std::cout << "\tHELP" << std::endl;
            std::cout << "\tSELECT [Device ID]?" << std::endl;
            std::cout << "\tSET [Analog | Digital] [position]" << std::endl;
            std::cout << "\tSTOP" << std::endl;
            break;

        case UNKNOWN:
        default: std::cerr << "Invalid Command !!" << std::endl; break;
        }
        return;
        /*
            std::regex regex("([a]|[d]|(analog)|(digital))[\\[][[:digit:]]{2}[\\]][=][[:digit:]]+");
            std::cout << "Enter Expression: ";
            std::getline(std::cin, input);
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            input.erase(remove(input.begin(), input.end(), ' '), input.end());
            std::cout << input << "\n";
            if(!regex_match(input, regex)) std::cerr << "Expression Error !!" << std::endl;

            std::regex regex_word("[a-z]+");
            std::smatch match;
            std::string mode, tmp = input;
            while (std::regex_search(tmp, match, regex_word)) {
                mode = match[0];
                tmp = match.suffix();
            }

            std::regex regex_int("[[:digit:]]+");
            int position, value, i = 0;
            tmp = input;
            while (std::regex_search(tmp, match, regex_int)) {
                if (i == 0) position = std::stoi(match[0]);
                else if (i == 1) value = std::stoi(match[0]);
                else break;
                tmp = match.suffix();
                i++;
            }
        } */
    }

    void ModbusTCP::Listen() {
        while (!threads_stop) {
            int newsockfd; // New socket file descriptor
            unsigned int cli_len; // Client address size
            sockaddr_in cli_addr; // Client address

            cli_len = sizeof(sockaddr_in);
            newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len); // Block until a client connects
            if (newsockfd < 0)
                std::cerr << "ERROR on accept" << std::endl;
            std::cout << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << " - Connected" << std::endl;

            t_clients.push_back(std::thread(&ModbusTCP::HandleRequest, this, newsockfd, &cli_addr));
            t_clients.back().detach();
        }
    }

    void ModbusTCP::HandleRequest(int newsockfd, sockaddr_in* cli_addr) {
        std::vector<byte> input, output;
        char msg[1000], buffer[1024];
        ssize_t bytes_recieved = recv(newsockfd, buffer, 1024, 0);
        // If no data arrives, the program will just wait here until some data arrives.
        if (bytes_recieved == 0) std::cerr << "host shut down." << std::endl;
        if (bytes_recieved == -1)std::cerr << "recieve error!" << std::endl ;
        bzero(buffer, 1024);
        //std::cout << "[ " << cli_addr->sin_addr << " : " << cli_addr->sin_port << " ] - " << buffer << " - { " << bytes_recieved << " bytes }" << std::endl;
        std::cout << "[("  << bytes_recieved << ") " << std::setfill('0') << std::hex ;
        for (unsigned int i = 0; i < bytes_recieved; i++){
            std::cout << std::setw(2) << (int)buffer[i] << ' ';
            if (i <= 5) msg[i] = buffer[i];
            else if (i > 5) input.push_back((byte)buffer[i]);
        }
        std::cout << "] " << std::dec;
        std::cout << std::endl;

        ModbusServer* mbs = GetDevice(input);
        //if (mbs == 0) return;
        output = mbs->Petition(input);

        if (!output.empty()) {

        }
      /* AQUÍ DEBERÍAMOS INVOCAR A peticion( ) y devolver el valor vector generado
       En primer lugar hay que convertir del array de char a vector de bytes,
       para ello se puede usar: vector<byte>( v, v + sizeof(v)/sizeof(byte) )

       Lo devuelto por peticion( ) hay que convertirlo a array,
       para ello se puede usar: std::copy(v.begin(), v.end(), arr);

       */
    }

    void ModbusTCP::Stop() {
        threads_stop = true;
        sockfd = -1;
        std::cout << "Stopping server..." << std::endl;
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
        std::cerr << "Invalid Device ID !!" << std::endl;
        return NULL;
    }

    ModbusServer* ModbusTCP::Device(byte id) {
        for (int i = 0; i < devices.size(); i++)
            if (devices[i]->GetID() == id)
                return devices[i];
        std::cerr << "Invalid Device ID !!" << std::endl;
        return NULL;
    }

}