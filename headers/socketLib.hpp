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

#include <chrono>

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
    RESERROR =                      -26,
    NORES =                         -27,

    NOVALIDPORT =                   -28,
    BADCLIENTCONNECTION =           -29,
    NOCLIENTCONNECTION =            -30,

    //constants
    POLLTIMER =                   10000,
    UNSCANNEDPORT =                 -1,
    BADPORT =                       0,
    GOODPORT =                      1,
    LOWERLIMIT =                     1023,
    NUMPORTS =                      65536,

};

/* The states the connections are in
*/
const int QUIT                          = 0;
const int OPEN                          = 1;
const int IDLE                          = 2;
const int BUSY                          = 3;
const int FINALIZING                    = 4;

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

        if a poll comes out negative it will return 
        UNKNOWNPOLLRESULT

        throws a runtime_exception error if fd is bad
        */
        int getPacket(std::string& id, std::string& message, int timer=POLLTIMER);

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
2. If a client is connected, it alternates between three states:
    IDLE: essentially does nothing on its own (except maybe send PING to
        verify the connection) and awaits for something to tell it to
        send something over the network
    BUSY: the job got told to send something over the network, so it 
        hangs and keeps waiting until the client sends a response. Once
        the response is received from the client, the response in some 
        data and goes to the FINALIZING state
    FINALIZING: the BUSY state finished and stored the response in some 
        storage somewhere. We let the user respond to that and try to
        pull the data stored. We will allow an extra input from the user
        to notify that the data was collected with the "collected" input
        which will reset this state back to INIT mode

This can be viewed like a fsa state:

         (client connected)                                 
[OPEN]-------------------->[IDLE]----->------>---------+ (get a query)  
                             ^                         |
                             |                         V 
              ("collected")  |                       [BUSY]
                             |                         |
                    [COLLECTED]<--------<------<-------+
                                  (got a response)
*/
class Connection : public fsa::FSA{ // inheritence to enforce
                                    // the idea of FSA (what is this
                                    //at its core)
private:
    history::History record;

    //the pair will often store the [ID, MESSAGE BODY]
    // in that order
    std::pair<std::string, std::string> msgStorage;

    Socket s;
    bool socketConnected;
    Client c;
    bool clientConnected;

public:

    Connection();

    /* 
    this simply calls connectSocket()
    
    returns the call to connectSocket
    */
    int start() override;
    /* 
    The format of the input for Connection is 2 parts:
        - COMMAND
        - PARAMS

    if COMMAND is "connect",
        PARAMS is empty

        From OPEN state
        Transforms to IDLE state

    if COMMAND is "query",
        PARAMS should be 2 parts: "ID", "MSG"
            - ID: The id of the query
            - MSG: the message of the query

        From IDLE state
        To BUSY state

    if COMMAND is "response"
        PARAMS should be the message that we got from the 
        query
            - ID: the id of the response
            - MSG: the message of the response

        From BUSY state
        To FINALIZING state

    if COMMAND is "collected"
        PARAMS should be empty

        Now this part is a PROMISE... (not sure if I like that)
        that we have safely extracted the output we wanted
        
        From FINALIZING state
        To IDLE state
    
    if COMMAND is "NONE" (and it shouldn't be. Rather it 
        shouldn't be called in the first place)

        >This does nothing
    
    if COMMAND is "ERROR"
        PARAMS will be the type of error we got
            - "QUIT":               something fatal happen and this fsa
                    should completely exit 

                    From ANY STATE
                    To QUIT

            - "CONNECTIONRESET":    The connection to the client turned 
                    sour so we should probably try to reconnect to client

                    From ANY STATE
                    To OPEN

            - "BADRESPONSE":        The interaction between messages
                    turned bad so just reset to the state where
                    we're awaiting the next message

                    From ANY STATE
                    To IDLE
    if COMMAND is "quit":
        PARAMS will be empty

        From ANY STATE
        To QUIT
    
    does nothing on invalid commands/parameters
    */
    void input(std::vector<std::string>& args) override;
    /*
    This disconnects to a socket
    */
    void exit() override;

    /*The implementation details of job are listed above
        OPEN:       - connectClient()
        IDLE:       - [nothing]
        BUSY:       - sendQuery() then store that data to msgStorage
        FINALIZING: - [nothing]
    */
    int job() override;
    friend int open(Connection& c);
    friend int idle(Connection& c);
    friend int busy(Connection& c);
    friend int finalizing(Connection& c);

    friend void printConnectionHistory(Connection &c);

    /*
    This returns a copy of the messageStorage

    if state isnt correct (called when there wasn't anything
    ready for user, returns <"SYS", "@NOT READY YET">)
    
    so a value is still returned, it's up to the user
    */
    std::pair<std::string, std::string> getMessageStorage();
};

/*
checks if the argument length equals the desired length and writes
a simpl error message

returns whether or not the value is correct
*/
bool paramsCheck(int argLength, int wantedLength, const std::string& command, history::History& record);

/*
checks if the previous state matches what it should be and writes
a simple error message 

returns whether or not the value is correct
*/
bool stateTransitionCheck(int prevState, int prevStateDesired, const std::string& command, history::History& record);

/*
These are the functions the Connection does
and these are pulled out to enforce SRP
This code actually needs two loops. Client connection 
needs to be polled separately, but in order to provide 
a more unified interface, this function
hides this double looped-implementation

The two loops are:
Is Socket Connected?
    if NO:  --------+
            ^       |
            |       |
            |       | Try to connect to a socket
            |       |
            +-------+

Is Client Connected?
    if NO:  --------+
            ^       |
            |       |
            |       | Try to connect to a client
            |       |
            +-------+


*/

/*Tries to connect to a valid Socket

returns 1 on full connect

returns NOVALIDPORT when there is no valid port 
in the range 9000-9100

raises error on impossible cases
*/
int connectSocket(Socket& s, history::History& record);


/*Tries to connect to a valid client

returns 1 on full connect

returns BADCLIENTCONNECTION on the case where
poll() gave a negative value for client connection

returns NOCLIENTCONNECTION when the poll does not  
return within the allotted poll() time for client connection

raises error on impossible case
*/
int connectClient(Client& c, Socket& s, history::History& record);


/*
tries to send to client a body 

returns 1 on successful send  

return SENDERROR when unable to send
*/
int sendQuery(const std::string& id, 
                const std::string& query,  
                Client& c, 
                history::History& record);

/*
tries to get a message from the client
and modifies id and query to be the result
>> NOTE that id and query gets modified
so make sure it's empty

returns 1 on successful response  

returns NORES when there is no response
return RESERROR when response gives error
*/
int getResponse(std::string& id, 
                std::string& response,  
                Client& c, 
                history::History& record);

/*
this sends "PING" and waits for "PONG"

returns 1 on any successful pingpong

returns SENDERROR when unable to send
returns -1 when it is unable to get a PONG
*/
int pingpong(Client& c, history::History& record);

/*
this just tries to pingpong up to 3 times when
poll comes out empty

if any response is weird, it will 
return -1

returns 1 on any successful pingpong

returns -1 when it failed all 3 times
*/
int verifyConnection(Client& c, history::History& record);

/*Here are the functions the state of the Connection is in

they return 1 on successful jobs
they also may call their own inputs at times
*/
int open(Connection& c);
int idle(Connection& c);
int busy(Connection& c);
int finalizing(Connection& c);

/*
simple calls printHistory() of c.history
*/
void printConnectionHistory(Connection& c);

}

