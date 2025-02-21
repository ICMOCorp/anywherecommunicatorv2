#include "sharedstuff.hpp"
//#include "ring.hpp"
#include <sys/socket.h> // For socket(), bind(), 
                        //  listen(), accept(), and send()
                        // and getaddrinfo()/addrinfo
#include <sys/types.h>  // also for getaddrinfo()/addrinfo (not specified)
#include <netdb.h>      // also for getaddrinfo()/addrinfo

#include <netinet/in.h> // For sockaddr_in structure.
#include <arpa/inet.h>  // For inet_ntoa() and htons() functions.
#include <unistd.h>     // For close().
#include <poll.h>       // For poll and POLLIN
#include <fcntl.h>      // For fcntl, F_GETFL, F_SETFL, O_NONBLOCK

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cerrno>


namespace socketstuffs{

/*
list of possible error codes and other constants
*/
enum ErrorCodes{
    //error codes
    INVALIDPORT =                   -10,
    NOTOPENED =                     -11,
    NONEMPTYVECTOR =                -12,
    ALREADYOPEN =                   -13,

    //constants
    UNSCANNEDPORT =                 -1,
    BADPORT =                       0,
    GOODPORT =                      1,
    LOWERLIMIT =                     1023,
    NUMPORTS =                      65536
};

/*function that takes a return code from the possible 
functions below
returns a user-friendly message that explains what the
error is*/
std::string interpretError(int errCode);

/*when given an empty vector
it will fill that vector with  valid ports
returns true
if vector is not empty, returns NONEMPTYVECTOR error*/
int getValidScannedPorts(int startport, int endport, std::vector<int>& validPorts, bool display=false);

/* A class that acts as a container for variables related to 
the socket created on the system. 
 - Assumes the ip address is self (127.0.0.1)

these variables include:
- addrinfo
- sockaddr structs 
- pollfd for polling

it also separates the low level socket operations 
such as "getaddrinfo" and "socket->bind->listen->accept"

from calls
such as "open socket" and "connect to client" */
class Socket{
    private:
        struct addrinfo hints;              //helper for servinfo (when calling getaddrinfo)
        struct addrinfo* servinfo;          //the result of calling getaddrinfo

        struct pollfd socketfd[1];           //contains socketfd but also used for polling
//deprecated
        //struct pollfd clientfd[];           //contains fd for client socket but also for polling

        int port;                           //port number this socket is connected to
                                            //-1 if not connected

/*>>>DEPRECATED<<<*/
        /*implemented by Min: 
            ->  https://github.com/MiniMinja/CircularBuffer
        */
        //RingBufferS localBuffer;            //This is the buffer that handles the queueing of 
                                            //incoming messages (multiple message are contiguous
                                            //and it's possible to have mixed messages from a 
                                            //single packet)

    public:
        /*initializes a socket to all zeros, 
            assuming that all setting happens at the open*/
        Socket();
        
        //Removed copy constructor because Socket should not be copied
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        //specified functions
        /*keeps a socket open in NONblocking mode for listening at port
            and keeps track of that port in the member value
            returns 1 if port open was sucessful
            if port is not valid, return INVALIDPORT error
            if openIt is called again without a close, return ALREADYOPEN error*/
        int openIt(int port);
        /*closes the opened socket
            close should generally work 
            even if the socket will never opened to begin with*/
        int closeIt();
        /*returns the port value this object is connected to
            (note that the port value is -1 if it is not 
            connected to anything) */
        int getPort();

//>>>>>>>>>>>>>>>DEPRECATED<<<<<<<<<<<<<<<<<<<<<
// The client functions are deprecated and will be moved to the client class
        /*essentially calls accept on incoming connect requests
        and then verifies with ping call 
            if not trusted client, will return the NOTTRUSTWORTHY error
            if something else bad happend, will return appropriate error code */
        //int getTrustedClient();
        /*sends the whole message to the client (hides the loop that keeps
            calling send() to send the whole message)
            if socket disconnected, will return appropriate error
            if whole message not send because of other reasons, 
                will return INCOMPLETESEND error
            */
        //int sendToClient(const std::string msg);
        /*receives a message from the client, expected message packet is 1 Megabyte:
            3 bytes                     - size of message
            13 bytes                    - the username
            1,048,560 (1Megabyte - 16)  - the remaining message
            
            If the promised message isn't received (< 16 bytes or < size of message)
                this will wait upon this message for up to <sharedstuff:MAXWAITSECS>
                in which once the time has run out, the whole message will be dropped
            If we havent received the full message but socket disconnects
                this will also drop everything in the buffer
            */
        //std::string readFromClient();
        /* closes the client, but does not close the socket 
            also returns some error code, though a close should
            still happen despite some wierd shenanigans*/
        //int closeClient();


};

//TODO
class Client{

};
}

