#include <string>

namespace commands{

/*The very big list of commands*/

/*Undefined Generalized Command*/
class AbstractCommand{
public:
    inline int virtual executeCommand(const std::string& command) = 0;
};

/**/

}