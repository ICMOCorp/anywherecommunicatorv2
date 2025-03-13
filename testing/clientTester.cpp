#include "errorCPPPort.hpp"
#include "socketLib.hpp"
#include "testingSuite.hpp"


#include <vector>
#include <thread>
#include <chrono>

const int initTime = 3;
const int communicateTime = 1;

const std::string FILENAME = "clientTester.cpp";

void initTest(testing::TestSuite& t, std::thread& pythonJob, const std::string& testName){
    t = testing::TestSuite(testName, FILENAME);
    pythonJob = std::thread(testing::runPythonThread);
    std::cout << "Starting up the python thread\n"
            << "Let it start up for " << initTime << " seconds"
            << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(initTime));
}

void pauseForPython(const std::string& pauseReason, int time){
    std::cout << "STATUS: ---PAUSE--- We are pausing for: " << pauseReason << std::endl;
    std::cout << "\t>>Let's wait for " << time << " seconds" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(time));
}

void finalizeTest(testing::TestSuite& t, std::thread& pythonJob){
    testing::sendCommand("quit");
    pythonJob.join();
    t.printFinalOutput();
}

void clientConnectionTests(){
    testing::TestSuite t;
    std::thread pythonJob;
    initTest(t, pythonJob, "Client Connections Test");

    std::string command;
    int res;
    int pyRes;

    socketstuffs::Socket s;

    std::vector<int> validPorts;
    int ret = socketstuffs::getValidScannedPorts(9000, 9100, validPorts);
    if(ret == socketstuffs::NONEMPTYVECTOR){
        //this shouldn't happen
        //I also shouldn't be consistently putting up vague
        // depressing messages on impossible errors
        //I won't know which is which
        std::cout << "just .... end me now\n" 
                    "Alright fine, this is in clientTester.cpp"
                    << std::endl;
        return;
    }

    // socket open
    std::cout << "STATUS: Attempting socket connect" << std::endl;
    size_t index = 0;
    ret = s.openIt(validPorts[index]);
    while(ret == socketstuffs::INVALIDPORT){
        std::cout << "again, how?!?\n" 
                    << "checked: " << index << std::endl
                    << "in the socket open test of clientTester.cpp"
                    << std::endl;
        index++;
        if(index >= validPorts.size()){
            throw std::runtime_error("IM DONE!!!! (in clientTester.cpp)");
        }
        ret = s.openIt(validPorts[index]);
    }
    std::cout << "STATUS: socket opened on " << s.getPort() << std::endl;

    //client connection
    std::cout << "STATUS: attempting client connection" << std::endl;
    socketstuffs::Client c;
    command = "connect@" + std::to_string(s.getPort());
    testing::sendCommand(command);
    std::cout << "sent \"connect\" command at port " << s.getPort() << "\n"
                << "Let it wait for " << communicateTime << " seconds"
                << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(communicateTime));

    ret = c.connectIt(s);
    if(ret == socketstuffs::POLLTIMEDOUT){
        std::cout << "Poll timed out!" << std::endl; 
        throw std::domain_error("We should not have polled out, client did not connect");
    }
    else if(ret == socketstuffs::UNKNOWNPOLLRESULT){
        std::cout << "Got some negative poll value?" << std::endl;
        std::cout << "Oh. let's check it out: " <<std::endl;
        std::cout << "\t>>" << strerror(errno) << std::endl;
        throw std::runtime_error("Got negative poll value!");
    }
    else{
        std::cout << "STATUS: client connected"  << std::endl;
    }
    pyRes = testing::readResult();

    t.test("simple client connection test to valid socket", ret == 1
                                                        && pyRes == testing::SUCCESS);


    // Python-side disconnect test
    command = "disconnect";
    testing::sendCommand(command);
    std::cout << "sent \"disconnect\" command\n"
                << "Let it wait for " << initTime<< " seconds"
                << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(initTime));
    res = testing::readResult();
    if(res == -1){
        std::cout << "Error in Python-side disconnection test\n"
            << "did not find file for disconnect command"
            << std::endl;
    }
    t.test("Python-side disconnection from client", res == testing::SUCCESS);

    // Python reconnection test
    command = "connect@" + std::to_string(s.getPort());
    testing::sendCommand(command);
    std::cout << "sent \"connect\" command at port " << s.getPort() << "\n"
                << "Let it wait for " << initTime<< " seconds"
                << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(initTime));
    ret = c.connectIt(s);
    if(ret == socketstuffs::POLLTIMEDOUT){
        std::cout << "Poll timed out!" << std::endl; 
        throw std::domain_error("We should not have polled out, client did not connect");
    }
    else if(ret == socketstuffs::UNKNOWNPOLLRESULT){
        std::cout << "Got some negative poll value?" << std::endl;
        std::cout << "Oh. let's check it out: " <<std::endl;
        std::cout << "\t>>" << strerror(errno) << std::endl;
        throw std::runtime_error("Got negative poll value!");
    }
    else{
        std::cout << "STATUS: client connected"  << std::endl;
    }
    res = testing::readResult();
    if(res == -1){
        std::cout << "Error in Python reconnection test\n"
            << "did not find file for connect command"
            << std::endl;
    }
    t.test("Python reconnection to client", res == testing::SUCCESS);

    std::cout << "CLOSING PROGRAM" << std::endl;
    c.closeIt();
    s.closeIt();

    finalizeTest(t, pythonJob);
}

void clientCommunicationTests(){
    testing::TestSuite t;
    std::thread pythonJob;
    initTest(t, pythonJob, "Client Communications Test");

    std::string id1 = "albert";
    std::string id2 = "barbara";
    std::string command;
    int res, pyRes;
    std::string sRes;

    std::cout << "STATUS: Test initialized" << std::endl;

    //set up connection amongst the server (this) and the client 
    // (python port)
    std::vector<int> validPorts; 
    res = socketstuffs::getValidScannedPorts(9000, 9010, validPorts);
    if(res == socketstuffs::NONEMPTYVECTOR){
        std::cout << "How did we even get here\n"
                << "in scanning for valid ports\n"
                << "in the clientConnectionsTests() function in clientTester.cpp"
                << std::endl;
        throw std::runtime_error("MEH");
    }
    std::cout << "STATUS: Found valid ports" << std::endl;
    socketstuffs::Socket s;
    res = s.openIt(validPorts[0]);
    if(res == socketstuffs::INVALIDPORT){
        std::cout << "why are we still here....\n"
                << "in opening the socket\n"
                << "in the clientConnectionsTests() function in clientTester.cpp"
                << std::endl;
        throw std::runtime_error("just to suffer?");
    }

    std::cout << "STATUS: socket connected" << std::endl;
    socketstuffs::Client c;
    command = "connect@" + std::to_string(s.getPort());
    testing::sendCommand(command);
    pauseForPython("connect command", communicateTime);
    res = c.connectIt(s);
    if(res == socketstuffs::POLLTIMEDOUT){
        std::cout << "Poll timed out\n"
                << "in trying to connect to client\n"
                << "in clientCommunicationsTest() in clientTester.cpp"
                << std::endl;
        throw std::runtime_error("Please don't quit on me");
    }
    else if(res < 0){
        throw std::runtime_error(std::string("Poll is negative\n")
                                + "in clientCommunicationsTest() in clientTester.cpp");
    }
    pyRes = testing::readResult(); // obligatory read from sendCommand

    std::cout << "STATUS: client connected" << std::endl;

    //Test 1: simple message [server -> "PING" -> client]
    std::cout << "===TEST: starting test 1===" << std::endl;
    res = c.sendPacket(id1, "PING"); 
    if(res == socketstuffs::MSGTOOBIG){
        throw std::runtime_error(std::string("Again, please stop with this frizzle\n")
                                + "in test 1 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    std::cout << "sent ping" << std::endl;
    command = "read@"+id1;
    testing::sendCommand(command, "PING");
    pauseForPython("read command", communicateTime);
    pyRes = testing::readResult();
    t.test("sending simple message: [server -> \"PING\" -> client]", pyRes == testing::SUCCESS);

    //Test 2: simple messge 2  [server <- "Pong" <- client]
    std::cout << "===TEST: starting test 2===" << std::endl;
    command = "send@"+id1;
    testing::sendCommand(command, "PONG");
    pauseForPython("send command", communicateTime);
    std::string incoming, incomingID;
    res = c.getPacket(incomingID, incoming); 
    std::cout << "res val is " << res << std::endl;
    if(res == socketstuffs::MSGTOOBIG){
        throw std::runtime_error(std::string("Be free my children!\n")
                                + "in test 2 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    else if(res == socketstuffs::POLLTIMEDOUT){
        throw std::runtime_error(std::string("I thought we needed to call run after a send command\n")
                                            + "in test 2 of \n"
                                            + "clientCommunications() in clientTester.cpp");
    }
    else if(res == socketstuffs::UNKNOWNPOLLRESULT){
        throw std::runtime_error(std::string("Crack a few eggs to make an omelette\n")
                                + "in test 2 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    std::cout << "RESULTS: " << std::endl;
    std::cout << "\tgot " << incoming << "(length: " << incoming.size() << ")" << std::endl;
    std::cout << "\tfrom " << incomingID << "(length: " << incomingID.size() << ")" <<  std::endl;
    pyRes = testing::readResult();
    t.test("reading simple message: [server <- \"PONG\" <- client]", pyRes == testing::SUCCESS
                                                                    && incoming == "PONG"
                                                                    && incomingID == id1);

    //Test 3: multi-client [server -> "a:PING", "b:QING" -> client]
    std::cout << "===TEST: starting test 3===" << std::endl;
    res = c.sendPacket(id1, "PING"); 
    if(res == socketstuffs::MSGTOOBIG){
        throw std::runtime_error(std::string("Alright. Whos the big mama?\n")
                                + "in test 3 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    std::cout << ">>OUTGOING:sent ping" << std::endl;
    res = c.sendPacket(id2, "QING"); 
    if(res == socketstuffs::MSGTOOBIG){
        throw std::runtime_error(std::string("What's the big idea?\n")
                                + "in test 3 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    std::cout << ">>OUTGOING::sent qing" << std::endl;
    command = "read@"+id1;
    testing::sendCommand(command, "PING");
    pauseForPython("read command 1", communicateTime);
    pyRes = testing::readResult();
    command = "read@"+id2;
    testing::sendCommand(command, "QING");
    pauseForPython("read command 2", communicateTime);
    int pyRes2 = testing::readResult();
    t.test("sending multi-client message: [server -> \"a:PING\", \"b:QING\" -> client]", 
        pyRes == testing::SUCCESS && pyRes2 == testing::SUCCESS);


    //Test 4: multi-client [server <- "a:PONG", "b:QONG" <- client]
    std::cout << "===TEST: starting test 4===" << std::endl;
    command = "send@"+id1;
    testing::sendCommand(command, "PONG");
    pauseForPython("send command 1", communicateTime);
    command = "send@"+id2;
    testing::sendCommand(command, "QONG");
    pauseForPython("send command 1", communicateTime);
    incoming.clear(); incomingID.clear();
    std::string incoming2, incoming2ID;
    res = c.getPacket(incomingID, incoming); 
    if(res == socketstuffs::MSGTOOBIG){
        throw std::runtime_error(std::string("I'm running out of obscure things\n")
                                + "in test 4 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    else if(res == socketstuffs::POLLTIMEDOUT){
        throw std::runtime_error(std::string("I thought we needed to call run after a send command\n")
                                            + "in test 4 of \n"
                                            + "clientCommunications() in clientTester.cpp");
    }
    else if(res == socketstuffs::UNKNOWNPOLLRESULT){
        throw std::runtime_error(std::string("Crack a few eggs to make an omelette\n")
                                + "in test 4 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    res = c.getPacket(incoming2ID, incoming2); 
    if(res == socketstuffs::MSGTOOBIG){
        throw std::runtime_error(std::string("HUH, can't think rn\n")
                                + "in test 4 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    else if(res == socketstuffs::POLLTIMEDOUT){
        throw std::runtime_error(std::string("pulloololol\n")
                                            + "in test 4 of \n"
                                            + "clientCommunications() in clientTester.cpp");
    }
    else if(res == socketstuffs::UNKNOWNPOLLRESULT){
        throw std::runtime_error(std::string("quote me\n")
                                + "in test 4 of\n"
                                + "clientCommunications() in clientTester.cpp");
    }
    /*
    std::cout << "RESULTS: " << std::endl;
    std::cout << "\tgot " << incoming << "(length: " << incoming.size() << ")" << std::endl;
    std::cout << "\tfrom " << incomingID << "(length: " << incomingID.size() << ")" <<  std::endl;
    */
    pyRes = testing::readResult();
    t.test("sending multiclient message: [server <- \"a:PONG\", \"b:QONG\" <- client]", pyRes == testing::SUCCESS
                                                                    && incoming == "PONG"
                                                                    && incomingID == id1
                                                                    && incoming2 == "QONG"
                                                                    && incoming2ID == id2);

    c.closeIt();
    s.closeIt();

    finalizeTest(t, pythonJob);
}

void clientCommunicationLimitTests(){

}

int main(){
    clientConnectionTests();
    clientCommunicationTests();
    clientCommunicationLimitTests();
    return 0;
}