#include "testingSuite.hpp"
#include "commandListings.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <map>


namespace fs = std::filesystem;

namespace automator{

/*An object designed to call each line 
and test the output of each line
*/
class TestAutomator{
private:
    testing::TestSuite t;
    std::map<std::string, commands::AbstractCommand> commandRegistry;
    std::vector<std::string> lines;
    
public:
    friend int load(TestAutomator& t, 
                        const fs::path commandsFile,
                        const std::string testName, 
                        const std::string filename,
                        std::map<std::string, commands::AbstractCommand> commandRegistry);
    
    //
    inline int testLines(){

    }
};

/* loads up the TestAutomator by reading the file contents of 
commandsFile and outputs a TestAutomator

requires 
    - uninitialized TestAutomator object that this will dump into
    - name of the the files that contains the list of commands
    - the names of the test and filename that calls this object
    - the table that stores functions (and names associated to the
        functions to be easily referenced)

        Note: these tables are predefined in commandListings.hpp
*/
inline int load(TestAutomator& t,
                    const fs::path commandsFile,
                    const std::string testName, 
                    const std::string filename,
                    std::map<std::string, commands::AbstractCommand> commandRegistry){
    t.t = testing::TestSuite(testName, filename);
    t.commandRegistry = commandRegistry;

}

}