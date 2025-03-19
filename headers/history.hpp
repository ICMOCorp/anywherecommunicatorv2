#pragma once
#include <list>
#include <string>

namespace history{

const int MAXHISTORYSIZE            = 50;

/*A debug-convenience object that's just a wrapper for 
a list of strings

essentially, holds up to 50 messages (up to the design of the user)
that may indicate information such as, 
    - what happened
    - where it happened
    - what state it happened

stores in the order of newest -> latest
[newest] -> [2nd newest] -> ... -> [oldest]
*/
class History{
private:
    std::list<std::string> messages;

public:
    History() = default;
    inline History(const History& other){
        this->messages = other.messages;
    }
    inline friend void swap(History& a, History& b){
        std::swap(a.messages, b.messages);
    }
    inline History& operator=(History other){
        swap(*this, other);
        return *this;
    }


    void addMessage(std::string message);
    /*
        I would like to make this list more "safe" to access
        but I don't know what safe is....
        I'll refer to the member list as "history"
        
        Consider the following scenarios:
            - I returned history, and someone said push_front()
                or push_back()/pop_back() in a print function
                -> is this safe? Will I have permanently
                    affected history everywhere?
            - const by reference is bad for some reason and 
                const by pointer is also bad
            - Is this efficient?
        
        The safest is to get rid of getMessage entirely
    */
    //std::list<std::string> getMessage();
    void clear();

    friend void printHistory(History& h);
};

/*prints the content of the messages list in history*/
void printHistory(History& h);


}