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
*/
class History{
private:
    std::list<std::string> messages;

public:
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
    const std::list<std::string>& getMessage();
};

/*prints the content of the messages list in history*/
void printHistory(History& h);


}