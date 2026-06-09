#include "server.hpp"
#include "execute.hpp"

void Server::initCmd() {
    this->exCmd["PASS"] = new executePass();
    this->exCmd["NICK"] = new executeNick();
    this->exCmd["USER"] = new executeUser();
    this->exCmd["QUIT"] = new executeQuit();
    this->exCmd["PRIVMSG"] = new executePrivmsg();
    this->exCmd["JOIN"] = new executeJoin();
    this->exCmd["KICK"] = new executeKick();
    this->exCmd["INVITE"] = new executeKick();
}
std::map<int, Client> & Server::get_mapClient(){
    return this->_clients ;
}

std::string &Server::get_pass() {
return this->_pass;
}

int  Server::get_fdeppol() const  {
    return this->_epollFd ;
}
void Server::cretionChanel(std::string name ,std::string pass , Client &client) {
    this->All_chanel[name] = chanel();
    this->All_chanel[name].set_pass(pass);
    this->All_chanel[name].set_name(name);
    this->All_chanel[name].add_CTOchanel(client);
    client.addClientToMaps(name, this->All_chanel[name]);
    std::cout << "Chane a ete  bien creat " << std::endl;
}
std::map<std::string, chanel>& Server::get_Chanel(){
    return this->All_chanel ;
}


/*********************************** */
Server::Server(int port, std::string pas) : _port(port), _pass(pas), _serverFd(-1), _epollFd(-1) {
    initCmd(); /********************************* */
    initSocket();
}
Server::~Server() {
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->first);
    }
    close(_serverFd);
    close(_epollFd);
    
    for (std::map<std::string, execute*>::iterator it = exCmd.begin(); it != exCmd.end(); ++it) {
        delete it->second;
    }
}

void Server::initSocket() {
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0) throw std::runtime_error("Socket creation failed");

    int opt = 1;
    setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0) 
        throw std::runtime_error("Fcntl failed");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
        throw std::runtime_error("Bind failed");
    if (listen(_serverFd, SOMAXCONN) < 0) 
        throw std::runtime_error("Listen failed");

    _epollFd = epoll_create(1024);
    if (_epollFd < 0) 
        throw std::runtime_error("Epoll creation failed");

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _serverFd;
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &ev);

    std::cout << "[+] Server started on port " << _port << " with epoll" << std::endl;
}

void Server::modifyEpollState(int fd, uint32_t state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev);
}
#include <arpa/inet.h> /////
void Server::acceptNewClient() {
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    
    int clientFd = accept(_serverFd, (struct sockaddr*)&cli_addr, &cli_len);
    if (clientFd >= 0) {
        fcntl(clientFd, F_SETFL, O_NONBLOCK); 
        
        struct epoll_event ev;
        ev.events = EPOLLIN; 
        ev.data.fd = clientFd;
        epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &ev);
        std::string client_ip = inet_ntoa(cli_addr.sin_addr); /////////////////////
        _clients[clientFd] = Client(clientFd); 
        _clients[clientFd].set_Id(clientFd);
        _clients[clientFd].set_host(client_ip);
        std::cout << "[NETWORK] New client connected. FD: " << clientFd << std::endl;
    }
}

void Server::handleClientRead(int fd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        
    if (bytes <= 0 ) {
        std::cout << "[NETWORK] Client disconnected. FD: " << fd << std::endl;
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);  
        close(fd);
        _clients.erase(fd);  
        return;
    }

    _clients[fd].getRecvBuffer() += buffer;

    size_t pos;
    while ((pos = _clients[fd].getRecvBuffer().find("\r\n")) != std::string::npos) {
        std::string full_command = _clients[fd].getRecvBuffer().substr(0, pos);
        _clients[fd].getRecvBuffer().erase(0, pos + 2);
        parseInput(fd, full_command);
        if(this->_clients[fd].get_Close()){
            epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);  
            close(fd);
            _clients.erase(fd);  
            return;
        }
    }
}

void Server::parseInput(int clientFd, const std::string& raw_command) {
    std::istringstream iss(raw_command);
    std::string cmd_name;
    
    iss >> cmd_name; 
    if (cmd_name.empty()) return;

    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) { 
            args.push_back(arg); 
    }
    std::cout << "[LOGIC] Executing CMD: " << cmd_name  << " arg" << arg.size() <<std::endl;

    
    if (exCmd.find(cmd_name) != exCmd.end()) {
        exCmd[cmd_name]->executeCmd(*this,this->_clients[clientFd] , args);
        
        if (!_clients[clientFd].getSendBuffer().empty()) {
            modifyEpollState(clientFd, EPOLLIN | EPOLLOUT);
        }
    } else {
        this->_clients[clientFd].getSendBuffer() += "Unknown command.\n" ;
        modifyEpollState(clientFd, EPOLLIN | EPOLLOUT); 
    }
}

void Server::handleClientWrite(int fd) {
    std::string& response = _clients[fd].getSendBuffer();
    if (response.empty()){
        
        return;
    }  
    ssize_t sent = send(fd, response.c_str(), response.length(), 0);
    if (sent > 0) {
        _clients[fd].clearSendBuffer(sent);
        
        if (_clients[fd].getSendBuffer().empty()) {
            modifyEpollState(fd, EPOLLIN);
        }
    }
}

void Server::run() {
    while (true) {
        int nfds = epoll_wait(_epollFd, _events, 64, -1);
        if (nfds < 0) {
            std::cerr << "Epoll wait error" << std::endl;
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int current_fd = _events[i].data.fd;

            if (current_fd == _serverFd) {
                acceptNewClient();
            } else {
                if (_events[i].events & EPOLLIN) {
                    handleClientRead(current_fd);
                }
                if (_events[i].events & EPOLLOUT) {
                    handleClientWrite(current_fd);
                }
            }
        }
    }
}