#include "errorCPPPort.hpp"
#include "socketLib.hpp"
#include "ring.hpp"
#include "testingSuite.hpp"

#include <iostream>
#include <string>
#include <vector>
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

void openSocketTest(){

}

void closeSocketTest(){

}


int main(){

    validPortScanTest();

    //std::thread pythonThread(runPythonThread());

    //pythonThread.join(); // depends on "quit.comm"
}