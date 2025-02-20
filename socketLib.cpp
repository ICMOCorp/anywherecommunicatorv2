#include "socketLib.hpp"

#include <iostream>

using enum socketstuffs::ErrorCodes;

std::string socketstuffs::interpretError(int errCode){
    //only implemented up to what's implemented
    if(errCode >= 0){
        return std::string("This isn't an error (errCode > 0)");
    }
    std::string ret;
    switch(errCode){
        case INVALIDPORT:
            ret = "Error: Bad port number\n\t- Call to openIt() in socketLib.hpp";
            break;
        case NOTOPENED:
            ret = "Error: Socket wasn't even opened\n\t- Call to closeIt() in socketLib.hpp";
            break;
        default: 
            ret = "Undefined error or not yet implemented yet: " + std::to_string(errCode);
    }
    return ret;
}

int socketstuffs::getValidScannedPorts(int portstart, int portend, std::vector<int>& validPorts, bool display){
    if(validPorts.size() != 0){
        return NONEMPTYVECTOR;
    }
    // look at page 23 of book
    struct addrinfo h;
    struct addrinfo* si;
    memset(&h, 0, sizeof(h));
    h.ai_family = AF_INET;
    h.ai_socktype = SOCK_STREAM;
    h.ai_flags = AI_PASSIVE;

    for(int i = portstart; i <= portend; i++){
        if(display) std::cout << "Scanning Port " << i << ": ";

        if(i < LOWERLIMIT) {
            if(display) std::cout << "Port below "<< LOWERLIMIT << "! Ports are (" << LOWERLIMIT << "-" << NUMPORTS<< ")" << std::endl;;
            continue;
        }
        if(i > NUMPORTS) {
            if(display) std::cout << "Port number out of range! Ports are (" << LOWERLIMIT << "-" << NUMPORTS<< ")" << std::endl;;
            continue;
        }

        int status = getaddrinfo(NULL, std::to_string(i).c_str(), &h, &si);
        if(status != 0){
            std::cout << "Tried to call getaddrinfo()\n"
                    << "\t at function call getValidScannedPorts() in socketLib.cpp\n"
                    << "\tVal is " << status << "\n"
                    // << "\tPort is " << i
                    << std::endl;
            continue;
        }

        //look at page 25 of book
        int sockfd = socket(si->ai_family, 
                            si->ai_socktype, 
                            si->ai_protocol
                           );
        if(sockfd == -1){
            //not sure how to error check this
            // we should print out just in case
            std::cout << "Tried creating a socket\n"
                    << "\t at function call getValidScannedPorts() in socketLib.cpp\n"
                    << "\tVal is " << sockfd << "\n"
                    //<< "\tPort is " << i
                    << std::endl;
            continue;
        }

        int res = bind(sockfd, si->ai_addr, si->ai_addrlen);
        if(res == -1){
            std::cout << "Tried calling bind()\n"
                    << "\t at function call getValidScannedPorts() in socketLib.cpp\n"
                    << "\tVal is " << sockfd << "\n"
                    // << "\tPort is " << i
                    << std::endl;
            continue;
        }

        if(display) std::cout << "VALID PORT" << std::endl;
        validPorts.push_back(i);
        close(sockfd);
    }
    return 1;
}

/*
For references of "book" this is going to refer to
    "Beej's Guide to Network Programming" 
        by Brian "Beej Jogensen" Hall
*/

socketstuffs::Socket::Socket(){
    std::memset(&hints, 0, sizeof(hints));
    servinfo = 0;
    std::memset(socketfd, 0, sizeof(socketfd[0]));
    port = -1;

}

int socketstuffs::Socket::openIt(int port){
    int status;
    //copy from "Beej's Guide to Network Programming" by
    //      Brian "Beej Jorgensen" Hall, pg 22, Chapter 5
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo( NULL, std::to_string(port).c_str(), 
                                &hints, &servinfo)
        ) != 0){
            std::cout << "error in socket::checkports.cpp\n" 
                        << "\t>> in open()"
                        << "\t>> call to getaddrinfo()\n" 
                        << "\t>> port = " << port << "\n"
                        << gai_strerror(status) << std::endl;
        return INVALIDPORT;
    }

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sockfd == -1){
        std::cout << "error in socket::checkports.cpp\n" 
                    << "\t>> in openIt()"
                    << "\t>> call to socket()\n" 
                    << "\t>> port = " << port << "\n"
                    << std::strerror(errno) << std::endl;
        return INVALIDPORT;
    }
    //set it to nonblocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    int res = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if(res == -1){
        std::cout << "error in socket::checkports.cpp\n" 
                    << "\t>> in openIt()"
                    << "\t>> call to bind()\n" 
                    << "\t>> port = " << port << "\n"
                    << std::strerror(errno) << std::endl;
        return INVALIDPORT;
    }

    res = listen(sockfd, 5); // I want to set this to 1 but I don't
                             // know if that will be an issue
    if(res == -1){
        std::cout << "error in socket::checkports.cpp\n" 
                    << "\t>> in openIt()"
                    << "\t>> call to listen()\n" 
                    << "\t>> port = " << port << "\n"
                    << std::strerror(errno) << std::endl;
        close(sockfd);
        return INVALIDPORT;
    }

    // page 46 of book
    socketfd[0].fd = sockfd;
    socketfd[0].events = POLLIN;

    this->port = port;
    return 1;
}

int socketstuffs::Socket::closeIt(){
    // Some defensive programming
    // 1) if port is not connected, socketfd should not be a socket 
    //      in the first place, so we don't call close
    //      and there's no need to reset port at that point
    // 2) memset should be fine even if hints is empty
    // 3) servinfo might be null
    // 4) reset an array of struct (with only 1 element)
    if(port != -1){
        close(socketfd[0].fd);
        port = -1;
    }

    std::memset(&hints, 0, sizeof(hints));
    if(servinfo != NULL){
        freeaddrinfo(servinfo);
    }
    std::memset(socketfd, 0, sizeof(socketfd[0]));  //ChatGPT says it's fine...

    return 1;
}

int socketstuffs::Socket::getPort(){
    return port;
}