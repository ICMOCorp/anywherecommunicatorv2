#include "history.hpp"

#include <iostream>

void history::History::addMessage(std::string message){
    if(messages.size() >= history::MAXHISTORYSIZE){
        messages.pop_back();
    }
    messages.push_front(message);
}

/*
std::list<std::string> history::History::getMessage(){
    return messages;
}
*/

void history::printHistory(history::History& h){
    std::cout << "Message Log:" << std::endl;
    for(auto it = h.messages.begin(); it != h.messages.end();it++){
        std::cout << "\t" << *it << std::endl;
    }
}