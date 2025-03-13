#include "history.hpp"

#include <iostream>

void history::History::addMessage(std::string message){
    if(messages.size() >= history::MAXHISTORYSIZE){
        messages.pop_back();
    }
    messages.push_front(message);
}

const std::list<std::string>& history::History::getMessage(){
    return messages;
}

void history::printHistory(History& h){
    const std::list<std::string>& messages = h.getMessage();
    std::cout << "Message Log:" << std::endl;
    for(auto it = messages.begin(); it != messages.end();it++){
        std::cout << "\t" << *it << std::endl;
    }
}