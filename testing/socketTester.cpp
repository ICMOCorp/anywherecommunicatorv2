#include "errorCPPPort.hpp"
#include "socketLib.hpp"
#include "ring.hpp"
#include "testingSuite.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

/*tests getValidScannedPorts() in socketLib.cpp*/
/*wrapper to the TestSuite wrapper*/
void testSinglePortScanCase(std::string caseName, int start, int end, std::vector<int>& ports, testing::TestSuite& t){
    ports.clear();
    socketstuffs::getValidScannedPorts(start, end, ports, true);
    t.test(caseName, testing::testViaUserInput("Does this look good?"));
}
void validPortScanTest(){
    testing::TestSuite t("getValidScannedPorts()", "socketLib.cpp");

    std::vector<int> ports;

    //trivial case - the good case
    testSinglePortScanCase("trivial - normal", 9000, 9010, ports, t);

    //trivial case - the single case
    testSinglePortScanCase("trivial - single", 9011, 9011, ports, t);

    //edge case: low values
    testSinglePortScanCase("edge - low values", -10, 10, ports, t);

    //bad case: possible repeat
    testSinglePortScanCase("bad - possible repeat", 9000, 9010, ports, t);

    t.printFinalOutput();
}

void pythonPortTest(){
    testing::TestSuite t("Python thread check", "errorCPPPort.hpp and errorPythonPort.py");
    std::thread pythonJob(testing::runPythonThread);
    int sec = 3;
    std::cout << "Starting up the python thread\n"
            << "Let it start up for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));

    // on test - checks thread to see if python is open
    // note that this only checks the variables and doesn't check the 
    // actual thread
    t.test("on test", testing::threadRunning.load());

    // test1 - should get true
    testing::sendCommand("test1");
    std::cout << ">Letting python process <test1> command\n"
            << "Let it wait up for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    int val = testing::readResult();
    t.test("test1", val == testing::SUCCESS);
    if(val == -1){
        std::cout << "Error in running readResult in errorCPPPort.hpp - "
                << "neither \"good\" nor \"bad\" found"
                << std::endl;
    }

    // test2 - should get false 
    testing::sendCommand("test2");
    std::cout << ">Letting python process <test2> command\n"
            << "Let it wait up for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    val = testing::readResult();
    t.test("test2", val == testing::FAILURE);
    if(val == -1){
        std::cout << "Error in running readResult in errorCPPPort.hpp"
                << "neither \"good\" nor \"bad\" found"
                << std::endl;
    }
    
    //quit - python should be closed
    testing::sendCommand("quit");
    std::cout << "Letting the python process \"quit\" for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    bool closed = testing::threadRunning.load() == false;
    t.test("quit test", closed);
    if(!closed){
        std::cout << "WARNING: python did not close! " 
                << "There is a background process running" << std::endl;
    }

    pythonJob.join();
    t.printFinalOutput();
}

void socketTest(){
    testing::TestSuite t("Socket Test", "socketLib.cpp");
    std::thread pythonJob(testing::runPythonThread);
    int sec = 3;
    std::cout << "Starting up the python thread\n"
            << "Let it start up for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));

    //port check init
    socketstuffs::Socket s;
    t.test("initialize check (port should be -1)", s.getPort() == -1);

    //simple connection
    std::string testname ="simple connection";
    std::vector<int> validPorts;
    socketstuffs::getValidScannedPorts(9000, 9010, validPorts);
    std::cout << "----VALID PORTS: [";
    for(size_t i = 0;i<validPorts.size();i++){
        if(i > 0){
            std::cout << ", ";
        }
        std::cout << validPorts[i];
    }
    std::cout << "]----" << std::endl;
    int retVal = s.openIt(validPorts[0]);
    std::string command = "verifyOpen@";
    command += std::to_string(s.getPort());
    testing::sendCommand(command);
    std::cout << "sent \"verifyOpen\" command\n"
            << "Let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    int res = testing::readResult();
    if(res == -1){
        std::cout << "Error in " << testname << " test\n"
            << "did not find file for verifyOpen command"
            << std::endl;
    }
    t.test(testname, retVal == 1 
                    && res == testing::SUCCESS 
                    && s.getPort() == validPorts[0]);

    // reopen - attempting to connect without closing original socket
    // expect to fail - connection is still good and port did not change
    testname = "reopen (open without a closeIt call)";
    retVal = s.openIt(validPorts[0]);
    command = "verifyOpen@";
    command += std::to_string(s.getPort());
    testing::sendCommand(command);
    std::cout << "sent \"verifyOpen\" command\n"
            << "Let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    res = testing::readResult();
    if(res == -1){
        std::cout << "Error in " << testname << " test\n"
            << "did not find file for verifyOpen command"
            << std::endl;
    }
    t.test(testname, retVal == socketstuffs::ALREADYOPEN
                    && res == testing::SUCCESS 
                    && s.getPort() == validPorts[0]);

    // close connection
    testname = "close connection";
    int oldPortVal = s.getPort();
    retVal = s.closeIt();
    command = "verifyClose@";
    command += std::to_string(oldPortVal);
    testing::sendCommand(command);
    std::cout << "sent \"verifyClose\" command\n"
            << "Let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    res = testing::readResult();
    if(res == -1){
        std::cout << "Error in " << testname << " test\n"
            << "did not find file for verifyClose command"
            << std::endl;
    }
    t.test(testname, retVal == 1
                    && res == testing::SUCCESS 
                    && s.getPort() == -1);

    // reclose - attempting to close when a socket isnt open
    // close is still successful - socket should remained closed
    // no other side effects
    testname = "reclose connection";
    retVal = s.closeIt();
    command = "verifyClose@";
    command += std::to_string(oldPortVal);
    testing::sendCommand(command);
    std::cout << "sent \"verifyClose\" command\n"
            << "let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    res = testing::readResult();
    if(res == -1){
        std::cout << "error in " << testname << " test\n"
            << "did not find file for verifyClose command"
            << std::endl;
    }
    t.test(testname, retVal == 1
                    && res == testing::SUCCESS
                    && s.getPort() == -1);

    // reconnect - attempting to reconnect to a closed socket
    testname = "reconnect";
    retVal = s.openIt(oldPortVal);
    command = "verifyOpen@";
    command += std::to_string(oldPortVal);
    testing::sendCommand(command);
    std::cout << "sent \"verifyOpen\" command\n"
            << "let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    res = testing::readResult();
    if(res == -1){
        std::cout << "error in " << testname << " test\n"
            << "did not find file for verifyOpen command"
            << std::endl;
    }
    t.test(testname, retVal == 1
                    && res == testing::SUCCESS
                    && s.getPort() == oldPortVal);


    // reopen v2 - attempting to connect to ANOTHER PORT without closing original socket
    // should result same as first reopen
    testname = "reponv2 - reopen on another port";
    retVal = s.openIt(validPorts[1]);
    command = "verifyOpen@";
    command += std::to_string(oldPortVal);
    testing::sendCommand(command);
    std::cout << "sent \"verifyOpen\" command\n"
            << "let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    int res1 = testing::readResult();
    if(res1 == -1){
        std::cout << "error in " << testname << " test\n"
            << "did not find file for verifyOpen on old port command"
            << std::endl;
    }
    command = "verifyOpen@";
    command += std::to_string(validPorts[1]);
    testing::sendCommand(command);
    std::cout << "sent \"verifyOpen\" command\n"
            << "let it wait for " << sec << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    int res2 = testing::readResult();
    if(res2 == -1){
        std::cout << "error in " << testname << " test\n"
            << "did not find file for verifyOpen on new port command"
            << std::endl;
    }
    t.test(testname, retVal == socketstuffs::ALREADYOPEN
                    && res1 == testing::SUCCESS
                    && res2 == testing::FAILURE
                    && s.getPort() == oldPortVal);


    testing::sendCommand("quit");
    pythonJob.join();
    t.printFinalOutput();
}

int main(){

    //validPortScanTest();
    //pythonPortTest();
    socketTest();

    return 0;
}