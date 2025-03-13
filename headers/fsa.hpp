#include <vector>
#include <string>

namespace fsa{

/*This class is more of a formal DEFINITION of FSA than
it is an actual useful object (hence all the abstract functions)

The idea is the FSAs are able to run on their own, and so its 
primary function is in job() 

What changes the behavior/state of the FSA is in start(), exit()
and input(). start() and exit() help with initialization and freeing
of memory and resources while input() is designed to change the 
FSA state while it is the middle of the loop.

Hence, the loop needs to look as such:
while(true){
    input(some input);
    job();
}
*/
class FSA{
protected:
    int state;

    virtual void job() = 0;

public:
    virtual void start() = 0;
    virtual void input(std::vector<std::string>& msgs) = 0;
    virtual void exit() = 0;
};

}