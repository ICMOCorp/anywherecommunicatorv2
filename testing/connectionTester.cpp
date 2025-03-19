#include "socketLib.hpp"
#include "errorCPPPort.hpp"
#include "testingSuite.hpp"
#include "history.hpp"

#include <iostream>
#include <ctime>
#include <atomic>
#include <mutex>

int getUserNumber(const std::string& prompt){
    int x;
    std::cout << prompt;
    std::cin >> x;
    return x;
}

void preConnectionTest(){
    testing::TestSuite t("Testing Necessary Functions", "connectionTester.cpp");

    //==== Starting python thread ====
    std::thread pythonJob(testing::runPythonThread);
    int pythonInitTime = 1;
    int pythonConnectionTime = 3;
    testing::pauseForPython("PYTHON INIT", pythonInitTime);

    // ==== some vars ====
    int res;                // the return value of functions
    int pythonRes;          // the return value of python jobs

    /*
     === [Test 0] Getting the necessary functions working: ===
                connectSocket
    */
    std::cout << "[Test0] Starting the connectSocket() Test" << std::endl;
    socketstuffs::Socket s;
    history::History h;
    res = socketstuffs::connectSocket(s, h);
    if(res == socketstuffs::NOVALIDPORT){
        std::cout << ">>FAILURE: res was not able to find a valid port" << std::endl;
    }
    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;
    t.test("connectSocket() test", res == 1);

    int portNum = s.getPort();
    std::string portStr = std::to_string(portNum);

    /*
     === [Test 1] Getting the necessary functions working: ===
                connectClient
    */
    std::cout << "[Test 1] Starting the connectClient() Test" << std::endl;
    socketstuffs::Client c;
    res = connectClient(c, s, h);
    if(res == socketstuffs::BADCLIENTCONNECTION){
        std::cout << "Bad connection. return value is " << std::to_string(res) << std::endl;
    }
    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;
    t.test("connectClient() test - no client", res == socketstuffs::NOCLIENTCONNECTION);

    std::cout << "Connecting on pythen end" << std::endl;
    testing::sendCommand(std::string("connect@") + portStr);
    testing::pauseForPython("Sent 'connect' command", pythonConnectionTime);
    pythonRes = testing::readResult();
    if(pythonRes != testing::SUCCESS){
        std::cout << "FATAL ERROR: Python was not able to connect to socket" << std::endl;
    }

    std::cout << "[Test 1B] Starting the connectClient() Test" << std::endl;
    res = connectClient(c, s, h);
    if(res == socketstuffs::BADCLIENTCONNECTION){
        std::cout << "Bad connection. return value is " << std::to_string(res) << std::endl;
    }
    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;
    t.test("connectClient() test - with client", res == 1);

    /*
     === [Test 2] Getting the necessary functions working: ===
                 pingpong
    */
    std::cout << "[Test 2] Starting the pingpong() Test" << std::endl;

    std::cout << "pingpong on python first, making it wait" << std::endl;
    testing::sendCommand(std::string("pingpong"));

    std::cout << "STATUS: calling pingpong()" << std::endl;
    res = socketstuffs::pingpong(c, h);
    if(res == socketstuffs::SENDERROR){
        std::cout << "ERROR: Could not send PING" << std::endl;
    }
    testing::pauseForPython("Called pingpong in cpp, python should receive it", pythonConnectionTime);

    pythonRes = testing::readResult();
    if(pythonRes != testing::SUCCESS){
        //testing::pauseForPython("Looking for the output", 100000);
        std::cout << "EH?: Could not find the output file of python test" << std::endl;
        std::cout << "Output is: " << pythonRes << std::endl;
        std::cout << "1 is SUCESS, 0 is FAILURE, -1 is NEITHER" << std::endl;
    }

    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;


    t.test("pingpong() test", res == 1 &&
                                pythonRes == testing::SUCCESS);

    /*
     === [Test 3] Getting the necessary functions working: ===
                verifyConnection 
    */
    
    std::cout << "[Test 3] Starting the verifyConnection() Test" << std::endl;

    std::cout << "Calling python's read first, telling it to wait for server" << std::endl;
    int timeWaitedByPython = rand() % 20000 + 10000;
    std::cout << "Will let python hang for " << timeWaitedByPython/1000 << " secs" << std::endl;
    testing::sendCommand(std::string("pingpongwait@") + std::to_string(timeWaitedByPython));

    std::cout << "STATUS: calling verifyConnection in cpp" << std::endl;
    res = verifyConnection(c, h);
    if(res == socketstuffs::SENDERROR){
        std::cout << "ERROR: Could not send PING" << std::endl;
    }

    testing::pauseForPython("Sent 'pingpongwait' command", pythonConnectionTime);
    pythonRes = testing::readResult();
    if(pythonRes != testing::SUCCESS){
        std::cout << "FATAL ERROR: Python was not able pingpong" << std::endl;
        std::cout << "Output is: " << pythonRes << std::endl;
        std::cout << "1 is SUCESS, 0 is FAILURE, -1 is NEITHER" << std::endl;
    }

    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;

    t.test("verifyConnection() test", res == 1 &&
                                    pythonRes == testing::SUCCESS);

    /*
     === [Test 4] Getting the necessary functions working ===
                sendQuery 
    */
    std::string id = "alex";
    std::string query = "Do you like pasta?";
    std::cout << "[Test 4] Starting the sendQuery() Test" << std::endl;

    std::cout << "Calling python's read first, telling it to wait for server" << std::endl;
    testing::sendCommand(std::string("read@"+id), query);

    std::cout << "STATUS: calling sendQuery() in cpp" << std::endl;
    res = sendQuery(id, query, c, h);
    if(res == socketstuffs::SENDERROR){
        std::cout << "ERROR: Could not send: " << query << std::endl;
    }

    testing::pauseForPython("Sent 'read' command", pythonConnectionTime);
    pythonRes = testing::readResult();
    if(pythonRes != testing::SUCCESS){
        std::cout << "FATAL ERROR: Python was not able read the query" << std::endl;
        std::cout << "Output is: " << pythonRes << std::endl;
        std::cout << "1 is SUCESS, 0 is FAILURE, -1 is NEITHER" << std::endl;
    }

    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;

    t.test("sendQuery() test", res == 1 &&
                                    pythonRes == testing::SUCCESS);
   
    /*
     === [Test 5] Getting the necessary functions working ===
                getResponse
    */
    std::string response = "Yes, I do!";
    std::cout << "[Test 5] Starting the getResponse() Test" << std::endl;

    std::cout << "Calling python's send first, making server read afterwards" << std::endl;
    testing::sendCommand(std::string("send@"+id), response);
    testing::pauseForPython("Sent 'send' command", pythonConnectionTime);
    pythonRes = testing::readResult();
    if(pythonRes != testing::SUCCESS){
        std::cout << "FATAL ERROR: Python was not able send the query" << std::endl;
        std::cout << "Output is: " << pythonRes << std::endl;
        std::cout << "1 is SUCESS, 0 is FAILURE, -1 is NEITHER" << std::endl;
    }

    std::cout << "STATUS: calling getResponse() in cpp" << std::endl;
    std::string idHolder, responseHolder;
    res = getResponse(idHolder, responseHolder, c, h);
    if(res == socketstuffs::NORES){
        std::cout << "ERROR: Got no response on server end" << std::endl;
    }
    else if(res == socketstuffs::RESERROR){
        std::cout << "ERROR: socket gave back an error" << std::endl;
    }

    std::cout << "===============HISTORY================" << std::endl;
    history::printHistory(h);
    std::cout << "======================================" << std::endl;

    t.test("getResponse() test", res == 1 &&
                                    pythonRes == testing::SUCCESS &&
                                    idHolder == id &&
                                    responseHolder == response);
    

    testing::sendCommand("quit");
    pythonJob.join();
    t.printFinalOutput();
}
/*
a simple Connection thread
*/
std::atomic<bool> runCondition(true);
std::string inputstr;
std::mutex inputMutex;
void inputstrload(std::string& output){
    std::lock_guard<std::mutex> guard(inputMutex);
    output = inputstr;
}
void inputstrstore(std::string& toChange){
    std::lock_guard<std::mutex> guard(inputMutex);
    inputstr = toChange;
}
void runConnection(){
    int res;
    std::cout << " | THREAD STATUS: starting connection thread" << std::endl;
    std::cout << " | THREAD STATUS: opening Connection" << std::endl;
    socketstuffs::Connection c;
    while(runCondition.load()){
        int res = c.start();
        if(res != 1){
            std::cout << " | THREAD ERROR: connection could not connect. Value: " << res << std::endl;
        }
        if(res == 1){
            std::cout << " | THREAD STATUS: connected!" << std::endl;
            break;
        }
        std::cout << " | THREAD STATUS: reconnecting" << std::endl;
    }

    std::string lastInput;
    std::vector<std::string> command;
    bool gotCommand = false;
    std::pair<std::string, std::string> lastMsg;
    std::cout << " | THREAD STATUS: Running loop" << std::endl;
    while(runCondition.load()){
        inputstrload(lastInput);
        //std::cout << "{" << lastInput << "}" << std::endl;
        if(lastInput.size() > 0){
            std::cout << " | THREAD STATUS: >>" << lastInput << "<<" << std::endl;
            if(lastInput == "status"){
                std::cout << " | THREAD STATE: " << c.getState() << std::endl;
            }
            else if(c.getState() == socketstuffs::IDLE){
                command.push_back("query");
                command.push_back("alex");
                command.push_back(lastInput);
                gotCommand = true;
            }
            else{
                std::cout << "Connection is not in the right state" << std::endl;
                std::cout << "Skipping this command" << std::endl;
            }
            //unload input
            lastInput = "";
            inputstrstore(lastInput);
        }

        if(gotCommand){
            c.input(command);
            //unload command
            command.clear();
            gotCommand = false;
        }
        int res = c.job();
        if(res != 1){
            int currState = c.getState();
            std::cout << " |  THREAD ERROR: error found in state: " << currState << std::endl;
            socketstuffs::printConnectionHistory(c);
            //we shouldn't throw an error here yet
            // I don't like it either but we have to wait
            //throw std::runtime_error("");
        }
        else if(res == 1 && c.getState() == socketstuffs::FINALIZING){
            lastMsg = c.getMessageStorage();
            std::cout << "I got " << std::endl;
            std::cout << "From " << lastMsg.first << "," << std::endl;
            std::cout << "\t" << lastMsg.second << std::endl;
        }
    }
}

void basicConnectionTest(){
    testing::TestSuite t("Basic Connection Test", "connectionTester.cpp");

    // ==== some vars ====
    int pythonInitTime = 1;
    int pythonConnectionTime = 3;
    //int res;                // the return value of functions
    int pythonRes;          // the return value of python jobs
    std::string id = "alex";

    /*
     === [Test 1] Testing Connection ===
        running the Connection job() on no
        connection
    */
    std::cout << "STATUS: ran runConnectionThread" << std::endl;
    std::thread startThread(runConnection);
    std::string userInput="";
    while(userInput != "quit"){
        std::cout << ">> " << std::endl;
        std::cin >> userInput;

        if(userInput != "quit"){
            inputstrstore(userInput);
        }
    }
    runCondition.store(false);
    startThread.join();

    /*
     === [Test 2] Testing Connection ===
        running the Connection job() on
        python Connection 
    */
    std::cout << "STATUS: starting python and connecting" << std::endl;
    //==== Starting python thread ====
    std::thread pythonJob(testing::runPythonThread);
    testing::pauseForPython("PYTHON INIT", pythonInitTime);
    std::string portnum;
    bool connected = false;

    std::cout << "STATUS: reran runConnectionThread" << std::endl;
    runCondition.store(true);
    std::thread secondStartThread(runConnection);
    userInput="";
    while(userInput != "quit"){
        std::cout << ">> " << std::endl;
        std::getline(std::cin, userInput);

        if(userInput == "connect"){
            std::cout << "What port should python connect to? ";
            std::cin >> portnum;
            testing::sendCommand("verifyconnect@"+portnum);
            testing::pauseForPython("PYTHON CONNECT", pythonConnectionTime);
            pythonRes = testing::readResult();
            if(pythonRes == testing::SUCCESS){
                connected = true;
            }
            else{
                std::cout << "STATUS: python was not able to connect" << std::endl;
                std::cout << "STATUS:   return value is " << pythonRes  <<  "(0 is FAILURE, -1 is no response from python)" << std::endl;
            }
            userInput="";
        }
        else if(userInput != "quit"){
            if(!connected){
                std::cout << "Let's let the client connect first " << std::endl;
            }
            else{
                if(userInput != "status"){
                    testing::sendCommand("processquery");
                }
                inputstrstore(userInput);
            }
        }
    }
    runCondition.store(false);
    secondStartThread.join();


    testing::sendCommand("quit");
    pythonJob.join();
    t.printFinalOutput();
}

void badConnectionTest(){

}

int main(){
    srand(time(0));
    //preConnectionTest();
    basicConnectionTest();
    return 0;
}
