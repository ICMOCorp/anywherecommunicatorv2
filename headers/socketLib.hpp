#include "sharedstuff.hpp"
#include "ring.hpp"
#include "history.hpp"
#include "fsa.hpp"
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
#include <stdexcept>
#include <utility>

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
    NORESPONSE =                    -14, 
    POLLTIMEDOUT =                  -15,
    UNKNOWNPOLLRESULT =             -16,
    BADRECV =                       -17,
    READCLOSE =                     -18,
    MSGTOOBIG =                     -19,
    IDTOOBIG =                      -20,
    BADSEND =                       -21,
    SENDCLOSE =                     -22,

    SENDERROR =                     -23,
    BADINPUTERROR =                 -24,
    ALREADYBUSY =                   -25,

    //constants
    POLLTIMER =                   10000,
    UNSCANNEDPORT =                 -1,
    BADPORT =                       0,
    GOODPORT =                      1,
    LOWERLIMIT =                     1023,
    NUMPORTS =                      65536
};

/* The states the connections are in
*/
const int INIT                          = 0;
const int IDLE                          = 2;
const int BUSY                          = 3;

/*function that takes a return code from the possible 
functions below
returns a user-friendly message that explains what the
error is*/
std::string interpretError(int errCode);

/*when given an empty vector
it will fill that vector with  valid ports
returns size of validPorts
if vector is not empty, returns NONEMPTYVECTOR error*/
int getValidScannedPorts(int startport, int endport, 
                        std::vector<int>& validPorts, bool display=false);

class Socket;
/*A middle-level class (that's meant to be hidden)
doing jobs like
 - holding data about file descriptor 
    about the socket connection (the client)
 - low-level calls to send and recv and all the error handling
    so that ONE SINGLE CLEAR packet is handled

receives a message from the client, expected message packet is 1 Megabyte:
    3 bytes                     - size of message
    13 bytes                    - the username
    1,048,560 (1Megabyte - 16)  - the remaining message
    
Note that the verification DOES NOT happen at this level.
    That way we can do verification on a separate function for more
    generality

Client is not one we should generate on their own (I can't think of
more ways to make it inaccessible to the public). The best practice would
be the use of the Connection class, which will properly ensure Socket
connection is made. At the same time, I do leave this open so that 
it can be modified and improved.
*/
class Client{
    private:
        struct sockaddr_storage theiraddr;
        struct pollfd clientfd[1];      // the file descriptor
                                        // of this file contains
                                        // the state of the 
                                        // connection

        ringbuffer::RingBufferS buffer;

    public:
        /* This constructor is just makes everything empty
            and sets fd to be bad (-1)
        */
        Client();
                                                            // the size in case
        //Removed copy constructor because Client should not be copied
        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
                                                            // read too much in one read

        /* The destructor disconnects the client if it is still 
            connected
        */
        ~Client();

        /* given a socket, it accesses its members (Client
            is a friend class) to connect to an actual 
            client that is trying to connect to the socket

            also prepares buffer to a certain size as to get ready
            for read

            returns 1 on success

            there will be a timer to connect so that 
            we can occasionally get an error message 
            if the wait is too long, 
            as opposed to waiting infinitely, and then
            figuring that after some waiting, something 
            is wrong and then force quitting

            A POLLTIMEDOUT will be returned when it times out

            Things that can happen to get negative poll
             - ?
            so return UNKNOWNPOLLRESULT
        */
        int connectIt(Socket& s);

        /*receives a message from the client, expected message packet is 1 Megabyte:
            3 bytes                     - size of message
            13 bytes                    - the username
            1,048,560 (1Megabyte - 16)  - the remaining message
        
        getPacket has a poll() and is willing to wait
        POLLTIMER seconds before returning a POLLTIMEDOUT

        throws a runtime_exception error if fd is bad
        */
        int getPacket(std::string& id, std::string& message);

        /*sends a message from the client, expected message packet is 1 Megabyte:
            3 bytes                     - size of message
            13 bytes                    - the username
            1,048,560 (1Megabyte - 16)  - the remaining message

        sendPacket has a poll(), and is willing to wait
        POLLTIMER seconds before returning a POLLTIMEDOUT

        throws a runtime_exception error if fd is bad
        
        if message is too big, returns MSGTOOBIG
        */
        int sendPacket(const std::string& id, 
                        const std::string& message);

        /*Closes the client socket
        (Similar to the destructor)
        */
        int closeIt();

};

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
            if openIt is called again without a close, 
            return ALREADYOPEN error*/
        int openIt(int port);
        /*closes the opened socket
            close should generally work 
            even if the socket will never opened to begin with*/
        int closeIt();
        /*returns the port value this object is connected to
            (note that the port value is -1 if it is not 
            connected to anything) */
        int getPort();
        /*returns the socket file descriptor id
            if the socket isn't connected, returns -1*/
        int getSocketFD();

        friend class Client;

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

/*This is the convenient interface that provides ways to send and receive 
messages from services that connect to this server via the port 

In the most biggest, general, and simplest terms:

1. Creates a port available to connect and awaits a client to connect
2. If a client is connected, it alternates between two states:
    IDLE: essentially does nothing on its own (except maybe send PING to
        verify the connection) and awaits for something to tell it to
        send something over the network
    BUSY: the job got told to send something over the network, so it 
        hangs and keeps waiting until the client sends a response. Once
        the response is received from the client, this goes back to the 
        IDLE state

This can be viewed like a fsa state:

         (client connected)                                 
OPEN ---------------------->IDLE---->---->-----+
                             ^                 |
                             |                 | (get a query)  
           (got a response)  |                 |
                             |                 |
                           BUSY<----<----<-----+


In reality, this is achieved by means of a (almost over-abstractified)
semaphore. There isn't really a variable, but instead a function call
will restart the function (thus restarting the while loop) 
while any hanging states will return (or exit) the function
(thus pause the while loop) 
*/
class Connection : public fsa::FSA{ // inheritence to enforce
                                    // the idea of FSA (what is this
                                    //at its core)
private:
    history::History record;

    std::pair<std::string, std::string> msgQueue;

    Socket s;
    Client c;

    std::string lastOutput;

protected:
    /*The implementation details of job are listed above
    */
    override void job();

public:
    Connection();

    /* this simple connects to a socket
    */
    override void start();
    /* The format of the input for Connection is 2 parts:
        - ID
        - MESSAGE
    This gets added to the msgQueue pair which is the 
    two data listed above

    This input should block when it tries to input while the 
    state is in BUSY mode (already processing) 
    and return ALREADYBUSY

    if the size of the vector is not 2, 
        let the user know it's an invalid size
    if the size of the id is greater than 13
    if the size of the second argument is bigger than a Megabyte
        and return BADINPUTERROR 
    */
    override void input(std::vector<std::string>& args);
    /*This disconnects to a socket
    */
    override void exit();

    /*This function does get the last output
    but if no output was outputted last, it will
    return ">@EMPTY@<"
    */
    std::string getLastOutput();

    const std::list<std::string>& getRecord();
};

//These are the functions the Connection does
//and these are pulled out to enforce SRP

/*Tries to connect to a valid socket and client
returns 1 on connect

raises an error on unable to connect
*/
int connectClient(Socket& s, Client& c, history::History& record);


/*sends query to this job. 

Then it calls poll to see if a response is given

returns 1 on successful send and response

return SENDERROR when unable to send
return -1 when the response is nothing
*/
int sendQuery(const std::string& id, 
                const std::string& query,  
                Client& c, 
                history::History& record);

/*Not sure if this function is needed
but this sends "PING" and waits for "PONG"

a difference response or a too long of a wait response
leads to a disconnect

returns 1 on successful send and response

returns SENDERROR when unable to send
returns -1 when the response is no PONG
*/
int verifyConnection(Client& c, history::History& record);

}

