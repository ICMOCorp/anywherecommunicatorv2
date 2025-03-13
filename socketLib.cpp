#include "socketLib.hpp"

#include <iostream>
#include <bitset>

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
        case NONEMPTYVECTOR:
            ret = "Error: this function only takes an empty vector as argument\n\t- Call to getValidScannedPorts() in socketLib.hpp";
            break;
        case ALREADYOPEN:
            ret = "Error: Tried to call open without closing. Close opened socket first\n\t- Call to openIt() in socketLib.hpp";
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
                    << "\tSkipping port " << i
                    << std::endl;
            continue;
        }

        if(display) std::cout << "VALID PORT" << std::endl;
        validPorts.push_back(i);
        close(sockfd);
    }
    return validPorts.size();
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
    if(this->port != -1){
        return ALREADYOPEN;
    }
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
        //std::cout << ">>>>its this" << std::endl;
        freeaddrinfo(servinfo);
        servinfo = NULL;
        //std::cout << ">>>>or is it?" << std::endl;
    }
    std::memset(socketfd, 0, sizeof(socketfd[0]));  //this is find, so long as it's trivial

    return 1;
}

int socketstuffs::Socket::getPort(){
    return port;
}

int socketstuffs::Socket::getSocketFD(){
    if(port == -1){
        return -1;
    }
    return socketfd[0].fd;
}

/* Client stuff */
socketstuffs::Client::Client(){
    clientfd[0].fd = -1;
}

socketstuffs::Client::~Client(){
    if(clientfd[0].fd != -1){
        close(clientfd[0].fd);
        clientfd[0].fd = -1;
    }
}

int socketstuffs::Client::connectIt(Socket& s){
    std::cout << "socket port is " << s.socketfd[0].fd << std::endl;
    int val = poll(s.socketfd, 1, POLLTIMER);
    if(val == 0){
        return POLLTIMEDOUT;
    }
    else if(val < 0){
        // Not sure what happens when poll comes out negative
        // from the documentation
    }
    else{
        if(s.socketfd[0].revents & POLLIN){
            socklen_t add_size = sizeof(theiraddr);
            clientfd[0].fd = accept(s.socketfd[0].fd,
                                        (struct sockaddr*)&theiraddr,
                                        &add_size);
            clientfd[0].events = POLLIN | POLLOUT;
            buffer = ringbuffer::RingBufferS(sharedstuff::Megabyte * 2);
            return 1;
        }
    }
    return UNKNOWNPOLLRESULT;
}

int socketstuffs::Client::getPacket(std::string& id, std::string& message){
    if(clientfd[0].fd == -1){
        throw std::runtime_error("ERROR: client fd is bad (client is not connected)\n"
                                 "in getPacket() in Client in socketLib.hpp");
    }

    const int chunkSize = 100;
    char chunk[chunkSize];
    std::memset(chunk, '\0', chunkSize);

    uint32_t messageSize = sharedstuff::INVALIDMSGCOUNT;
    while(buffer.size() < sharedstuff::MSGSIZEBYTECOUNT){
        //grab a couple bytes until we can recognize the size of the message
        int val = poll(clientfd, 1, POLLTIMER);
        if(val == 0){
            return POLLTIMEDOUT;
        }
        else if(val < 0){
            // Not sure what happens when poll comes out negative
            // from the documentation
            throw std::runtime_error(std::string("Oh lord please help me\n") + 
                                     "error value: " + std::strerror(errno) + "\n" + 
                                     "in reading the message size bytes\n" + 
                                     "when polling\n" + 
                                     "int getPacket() of socketLib.hpp");
        }
        else{
            if(clientfd[0].revents & POLLIN){
                ssize_t bytesRead = recv(clientfd[0].fd, chunk, chunkSize, 0); 
                if(bytesRead < 0){
                    //socket gives error
                    return BADRECV;
                }
                else if(bytesRead == 0){
                    //socket disconnected
                    return READCLOSE;
                }
                std::string toAdd(chunk, bytesRead);
                buffer.push(toAdd, toAdd.size());

                continue;
            }
            else{
                break;
            }
            /*
            std::cout << "What did we get? " << std::endl;
            std::cout << clientfd[0].revents << std::endl;
            std::cout << "as bits: " << std::bitset<8>(clientfd[0].revents) << std::endl;
            std::cout << (clientfd[0].revents & POLLIN) << std::endl;
            std::cout << (clientfd[0].revents & POLLOUT) << std::endl;
            std::cout << "In buffer:";
            for(size_t i = 0;i<chunkSize;i++){
                std::cout << std::hex << chunk[i] << " ";
            }
            std::cout << std::endl;
            */
        }

        //return UNKNOWNPOLLRESULT;
    }

    /*
    std::cout << "chunk: ";
    for(size_t i = 0;i<chunkSize;i++){
        if(i > 0) std::cout << ", ";
        std::cout << std::hex << (int)chunk[i];
    }
    std::cout << std::endl;
    */
        
    /*>>The message size part of the message<<*/
    //I don't like the memory usage of this... a whole string object?
    std::string msgSizeStr;
    int val = buffer.pop(msgSizeStr, sharedstuff::MSGSIZEBYTECOUNT);
    if(val != 1){       // some defensive programming
        //This shouldnt be possible because we just made this
        // check in the if statement above
        // something else is touching this so 
        //throw a runtime_error
        throw std::runtime_error(std::string("FATAL ERROR: This shouldn't have happened\n") + 
                                    "in after we check if buffer holds the right amount of bytes\n" +
                                    "for finding the message size\n" +
                                    "in getPacket() of Client class in socketLib.hpp");
    }
    /*
    std::cout << "msgSizeStr: ";
    for(size_t i = 0;i<msgSizeStr.size();i++){
        if(i > 0) std::cout << ", ";
        std::cout << std::hex << (int)msgSizeStr[i];
    }
    std::cout << std::endl;
    */

    /*>>The ID part of the message*/
    while(buffer.size() < sharedstuff::IDSIZEBYTECOUNT){
        //grab a couple bytes until we can recognize the size of the message
        int val = poll(clientfd, 1, POLLTIMER);
        if(val == 0){
            return POLLTIMEDOUT;
        }
        else if(val < 0){
            // Not sure what happens when poll comes out negative
            // from the documentation
            throw std::runtime_error(std::string("Oh lord please help me\n") +
                                     "error value: " + std::strerror(errno) + "\n" +
                                     "in reading the message size bytes\n" +
                                     "when polling\n" +
                                     "int getPacket() of socketLib.hpp");
        }
        else{
            if(clientfd[0].revents & POLLIN){
                ssize_t bytesRead = recv(clientfd[0].fd, chunk, chunkSize, 0); 
                if(bytesRead < 0){
                    //socket gives error
                    return BADRECV;
                }
                else if(bytesRead == 0){
                    //socket disconnected
                    return READCLOSE;
                }
                std::string toAdd(chunk, bytesRead);
                buffer.push(toAdd, toAdd.size());

                continue;
            }
            else{
                break;
            }
        }

        //return UNKNOWNPOLLRESULT;
    }
        
    /*>>The message size part of the message<<*/
    val = buffer.pop(id, sharedstuff::IDSIZEBYTECOUNT);
    if(val != 1){       // some defensive programming
        //This shouldnt be possible because we just made this
        // check in the if statement above
        // something else is touching this so 
        //throw a runtime_error
        throw std::runtime_error(std::string("FATAL ERROR: This shouldn't have happened\n") + 
                                    "in after we check if buffer holds the right amount of bytes\n" +
                                    "for finding the message id\n" +
                                    "in getPacket() of Client class in socketLib.hpp");
    }
    size_t nonspaceIndex = 0; // go to zero if we have an empty id
    for(size_t i = id.size() - 1;i > 0;i--){
        if(id[i] != ' '){
            nonspaceIndex = i;
            break;
        } 
    }
    id.erase(nonspaceIndex+1);

    /*>>The message part of the message*/
    messageSize = sharedstuff::strToUint(msgSizeStr);
    //std::cout << "message size is " << messageSize << std::endl;
    while(buffer.size() < messageSize){
        //grab the remaining message floating in the internet
        int val = poll(clientfd, 1, POLLTIMER);
        if(val == 0){
            return POLLTIMEDOUT;
        }
        else if(val < 0){
            // Not sure what happens when poll comes out negative
            // from the documentation
            throw std::runtime_error(std::string("Oh lord please help me\n") +
                                     "error value: " + std::strerror(errno) + "\n"
                                     "in reading the message size bytes\n" +
                                     "when polling\n" +
                                     "int getPacket() of socketLib.hpp");
        }
        else{
            if(clientfd[0].revents & POLLIN){
                ssize_t bytesRead = recv(clientfd[0].fd, chunk, chunkSize, 0); 
                if(bytesRead < 0){
                    //socket gives error
                    return BADRECV;
                }
                else if(bytesRead == 0){
                    //socket disconnected
                    return READCLOSE;
                }
                std::string toAdd(chunk, bytesRead);
                buffer.push(toAdd, toAdd.size());

                continue;
            }
            else{
                break;
            }
        }

        //return UNKNOWNPOLLRESULT;

    }
    val = buffer.pop(message, messageSize);
    if(val != 1){
        //This shouldnt be possible because we just made this
        // check in the if statement above
        // something else is touching this so 
        //throw a runtime_error
        throw std::runtime_error("FATAL ERROR: This shouldn't have happened\n"
                                    "in after we check if buffer holds the right amount of bytes\n"
                                    "for grabbing the whole message\n"
                                    "in getPacket() of Client class in socketLib.hpp");
    }
    //std::cout << "Got: " << message << std::endl;
    //std::cout << "From: " << id << std::endl;
    return 1;
}

int socketstuffs::Client::sendPacket(const std::string& id, const std::string& message){
    if(clientfd[0].fd == -1){
        throw std::runtime_error("ERROR: client fd is bad (client is not connected)\n"
                                 "in sendPacket() in Client in socketLib.hpp");
    }

    if(message.size() > sharedstuff::Megabyte - sharedstuff::HEADERSIZE){
        return MSGTOOBIG;
    }

    std::string idPadded = id;
    if(id.size() > sharedstuff::IDSIZEBYTECOUNT){
        return IDTOOBIG;
    }
    else if(id.size() < sharedstuff::IDSIZEBYTECOUNT){
        idPadded.append((size_t)sharedstuff::IDSIZEBYTECOUNT - id.size(), ' ');
    }

    uint32_t messageSize = message.size();
    std::string msgSizeHeader = sharedstuff::uintToStr(messageSize);
    std::string packetStr = msgSizeHeader + idPadded + message;

    /*
    std::cout << "Parts:" << std::endl;
    std::cout << "Message Header: " << msgSizeHeader << std::endl;
    std::cout << "idPadded: " << idPadded << std::endl; 
    std::cout << "message: " << message << std::endl;
    */

    const char* packet = packetStr.c_str();
    size_t packetSize = packetStr.size();
    /*
    std::cout << "Sending>>";
    for(size_t i = 0;i<packetSize;i++){
        std::cout << std::hex << (int)packet[i] << " ";
    }
    std::cout << std::endl;
    */
    uint32_t totalBytes = 0;

    while(totalBytes < packetSize){
        int val = poll(clientfd, 1, POLLTIMER);
        if(val == 0){
            return POLLTIMEDOUT;
        }
        else if(val < 0){
            // Not sure what happens when poll comes out negative
            // from the documentation
            throw std::runtime_error(std::string("Oh lord please help me\n") + 
                                     "error value: " + std::strerror(errno) + "\n"
                                     "in sending the packet\n"
                                     "when polling\n"
                                     "int sendPacket() of socketLib.hpp");
        }
        else{
            if(clientfd[0].revents & POLLOUT){
                ssize_t bytesSent = send(clientfd[0].fd, packet + totalBytes, packetSize - totalBytes, 0);
                if(bytesSent == 0){
                    return SENDCLOSE;
                }
                else if(bytesSent == -1){
                    return BADSEND;
                }

                totalBytes += bytesSent;

                continue;
            }
        }
    }

    return 1;
}

int socketstuffs::Client::closeIt(){

    if(clientfd[0].fd != -1){
        close(clientfd[0].fd);
        clientfd[0].fd = -1;
    }

    return 1;
}

/* Connection stuff */
int socketstuffs::connectClient(Socket& s, Client& c, history::History& record){
    std::vector<int> validPorts;
    int res;
    res = getValidScannedPorts(9000, 9100, validPorts);
    if(res == socketstuffs::NONEMPTYVECTOR) { // defensive programming
        throw std::logic_error(std::string("Please don't do this to me\n") + 
                            "in getting validScannedPorts()\n" +
                            "in the state INIT in job() in socketLib.cpp");
    }
    int index = 0;
    record.addMessage(std::string("Attempting to open a socket at port ") + std::to_string(validPorts[index]));
    res = s.openIt(validPorts[index]);
    while(res != 1){
        record.addMessage(">>Failed at this port:");
        if(res == socketstuffs::INVALIDPORT){ // defensive programming
            record.addMessage("\t- This port was invalid");
        }
        else if(res == socketstuffs::ALREADYOPEN){
            record.addMessage("\t- This port is already used");
        }
        index++;
        res = s.openIt(validPorts[index]);
    }
    if(res != 1){
        throw std::runtime_error(std::string("Failed to open a socket in the port range (9000-9100)"));
    }
    record.addMessage("Attempting to connect to a client");
    res = c.connectIt(s);
    if(res == socketstuffs::UNKNOWNPOLLRESULT){
        record.addMessage("Failed to connect to a client");
        record.addMessage("The errno message is:");
        record.addMessage(std:string("\t") + std::strerror(errno));
        throw std::runtime_error(std::string("Oh I'm a gummy bear\n") + 
                                "When I wanted to connect to client\n" + 
                                "in the state OPEN in job in socketLib.cpp");
    }
    record.addMessage(">>>>Client connected successfully");
    return 1;
}

int socketstuffs::sendQuery(const std::string& id, 
                            const std::string& query, 
                            const Client& c, 
                            history::History& record){
    //first we send
    record.addMessage(std::string("Trying to send message: ") + query + "\n" + 
                    "\t> From " + id + "\n");
    int res = c.sendPacket(id, query);
    if(res == socketstuffs::MSGTOOBIG){
        /*
        throw std::runtime_error(std::string("You gave a message that's too big\n")+
                                "when trying to send a query\n" + 
                                "in sendQuery() function of socketLib.cpp");
        */
        //Not sure if crashing on bad message is a good idea
        //The input is wrong, so we should be continuing 
        record.addMessage(std::string("SEND ERROR: Message to big! Query size is ") + std::to_string(query) + "\n" + 
                        "In sendQuery() function in socketLib.cpp");
        return socketstuffs::SENDERROR;
    }
    else if(res == socketstuffs::POLLTIMEDOUT){
        /*
        //not sure why this happens
        //awaiting until someone encounters this
        throw std::runtime_error(std::string("Somehow got that the socket poll() lagged out\n")+
                                "while sending a message over the internet\n" + 
                                "in sendQuery() function of socketLib.cpp");
        */    
        record.addMessage(std::string("SEND ERROR: Poll timed out in sending\n") + 
                        "In sendQuery() function in socketLib.cpp");
        return socketstuffs::SENDERROR;
    }
    record.addMessage(std::string(">>>>Message sent successfully\n"));

    //now we wait
    record.addMessage(std::string("Awaiting a response from the client\n") + 
                    "I'm willing to wait " + std::to_string(socketstuffs::POLLTIMER/1000) + " secs\n");
    std::string response, responseID;
    res = c.getPacket(responseID, response);
    if(res == socketstuffs::POLLTIMEDOUT){
        /*
        //not sure why this happens
        //awaiting until someone encounters this
        throw std::runtime_error(std::string("got poll lag again\n"
                                            "while trying to get a message over the internet\n"
                                            "in sendQuery() function of socketLib.cpp"))
        */
        //I'm not sure if crashing on an empty poll is a good idea
        // we should instead return a different value
        record.addMessage(std::string(">>>>Got a bad poll response"));
        return -1;
    }
    record.addMessage(std::string(">>>>Response received successfully: ") + response);

    lastOutput = response;
    return 1;
}

int socketstuffs::verifyConnection(Client& c, history::History& record){
    //first we send
    record.addMessage(std::string("Trying to send message: ") + "PING\n" + 
                    "\t> From SYS \n");
    int res = c.sendPacket("SYS", "PING");
    if(res == socketstuffs::MSGTOOBIG){
        /*
        throw std::runtime_error(std::string("You're telling me PING is too big?\n")+
                                "when trying to send a query\n" + 
                                "in verifyConnection() function of socketLib.cpp");
        */
        //Not sure if crashing on bad message is a good idea
        record.addMessage(std::string("SEND ERROR: Message to big! Query size is ") + std::to_string(query) + "\n" + 
                        "In verifyConnection() function in socketLib.cpp");
        return socketstuffs::SENDERROR;
    }
    else if(res == socketstuffs::POLLTIMEDOUT){
        /* 
        //not sure why this happens
        //awaiting until someone encounters this
        throw std::runtime_error(std::string("Somehow got that the socket poll() lagged out\n")+
                                "while sending \"PING\" over the internet\n" + 
                                "in verifyConnection() function of socketLib.cpp");
        */
        //Don't want to crash on a bad poll but the user should know
        // the connection may be buggin out
        record.addMessage(std::string("SEND ERROR: Poll timed out in sending\n") + 
                        "In verifyConnection() function in socketLib.cpp");
        return socketstuffs::SENDERROR;
    }
    record.addMessage(std::string("Sent PING successfully\n"));

    //now we wait
    record.addMessage(std::string("Awaiting a response from the client\n") + 
                    "I'm willing to wait " + socketstuffs::POLLTIMER + " secs\n");
    std::string response, responseID;
    res = getPacket(responseID, response);
    if(res == socketstuffs::POLLTIMEDOUT){
        /*
        //not sure why this happens
        //awaiting until someone encounters this
        throw std::runtime_error(std::string("got poll lag again\n"
                                            "while trying to get a message over the internet\n"
                                            "in sendQuery() function of socketLib.cpp"))
        */
        //Don't want to crash on a bad poll but the user should know
        // the connection may be buggin out
        record.addMessage(std::string(">>>>Got a bad poll response"));
        return -1;
    }
    if(response != "PONG"){
        record.addMessage(std::string("You did not get PONG!. Instead, I got: \n") + 
                                    "\t>> " + response);
        return -1;
    }
    
    return 1;
}

socketstuffs::Connection::Connection(){
    state = socketstuffs::INIT;
    lastOutput = "";
}

socketstuffs::Connection::start(){
    connectClient(s, c, record);
}

socketstuffs::Connection::input(std::vector<std::string>& args){
    if(args.size() != 2){
        /*
        //Since it is the USER's fault, instead of a crash
        //we need to DISPLAY to the user that the input was bad
        throw invalid_argument(std::string("invalid argument size. number of arguments must be 2.\n") + 
                                "Got a vector of size: " + std::to_string(args.size()) + "\n" + 
                                "for input() function in socketLib.hpp");
        */
        record.addMessage(std::string("Invalid argument size. Number of arguments must be 2.\n") + 
                        + "Got a vector f size: " + std::to_string(args.size()) + "\n" + 
                        + "for input() function in socketLib.hpp");
        return socketstuffs::BADINPUTERROR;
    }
    if(args[0].size() > 13){
        record.addMessage(std::string("The first argument (ID) is longer than 13 characters\n") + 
                            "> it is " + std::to_string(args[0].size()) + " characters long\n" +
                            "in input() function in socketLib.hpp");
        return socketstuffs::BADINPUTERROR;
    }
    if(args[1].size() > sharedstuff::Megabyte) {
        record.addMessage(std::string("The message part of the argument exceeds the maximum message length (") + sharedstuff::Megabyte + ")\n" + 
                                    "the message size is " + args[1].size() + "\n" + 
                                    "in input function in socketLib.hpp");
        return socketstuffs::BADINPUTERROR;
    }
    if(args[1].size() == 0){
        record.addMessage(std::string("Don't send an empty body\n") +
                            "in input function in socketLib.hpp");
        return socketstuffs::BADINPUTERROR;
    }
    if(state == socketstuffs::BUSY){
        record.addMessage(std::string("The connection is already busy. Wait until it is finished\n") + 
                            "in input function in socketLib.hpp");
        return socketstuffs::ALREADYBUSY;
    }

    record.addMessage(std::string("Got message! From: ") + args[0] + "\n" +
                        ">>" + args[1]);
    msgQueue = std::make_pair(args[0], args[1]);
    state = socketstuffs::BUSY;
}

socketstuffs::Connection::exit(){
    c.closeIt();
    s.closeIt();
}

void socketstuffs::Connection::job() {
    if(state == socketstuffs::IDLE){
        //I think it mostly does nothing here
        //This part is mostly just doing idle things until we get an input
        // maybe we try verifyConnection() in order to not let it die?
    }
    else if(state == socketstuffs::BUSY){
        int res = sendQuery(msgQueue.first, msgQueue.second, this, record);
        if(res == -1){
            record.addMessage(std::string("Didn't get a response from sendQuery\n") +
                                "in job() in socketLib.hpp");
        }
        else if(res == socketstuffs::SENDERROR){
            record.addMessaeg(std::string("Wasn't able to send in sendQuery\n") +
                                "in job() in socketLib.cpp");
        }
        state = socketstuffs::IDLE;
    }
}

std::string socketstuffs::Connection::getLastOutput(){
    if(lastOutput == ""){
        return ">@EMPTY@<";
    }

    return lastOutput;
}

const std::list<std::string>& socketstuffs::Connection::getRecord(){
    return record.getMessage();
}