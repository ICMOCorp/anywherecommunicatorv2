#include "communicator.hpp"

#include <iostream>

#include <atomic>

int open_socket(struct pollfd* socketfds, struct pollfd* clientfds){
    

}

void socket_job(){
    int socket_status = SOCKET_INIT;

    while(true){
        if(socket_status == SOCKET_INIT
            || socket_status == SOCKET_ERROR){
                //try to open
                //if open fails just keep this state
                //otherwise move to next state
            
        }
        else if(socket_status == SOCKET_OPENED){
            //just keep listening until a connection
            //up
            //when you get a connection move to the socket connected state 
        }
        else if(socket_status == SOCKET_CONNECTED){
            //ch
        }
        else if(socket_status == SOCKET_PROCESSING){

        }
    }

}

