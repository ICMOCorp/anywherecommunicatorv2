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

Hence, the bare loop needs to look as such:
while(true){
    input(some input);
    int ret = job();
}

Note that job() function has as return function. When interacting
with some FSAs, there comes times when we need to know if a job
was finished or not or if the state is awaiting something from
the user's end. Turns out a return value from job is crucial 
to figuring out how to adapt to the ever-changing state of the
FSA
*/
class FSA{
protected:
    int state;

public:
    virtual int job() = 0;
    virtual int start() = 0;
    virtual void input(std::vector<std::string>& msgs) = 0;
    virtual void exit() = 0;

    inline int getState(){
        return state;
    }
};

}