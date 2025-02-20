#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <filesystem>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

namespace testing{

inline const fs::path TESTINGDIR = "./testing";
inline const int SUCCESS = 1;
inline const int FAILURE = 0;

/*
starts the python tester 
assuming: The python tester generates the testing directory
*/
inline int startPython(){
    Py_Initialize();

    char pythonImportCode[] = 
    "import os\n"
    // "print(os.getcwd())\n"
    "import sys\n"
    "sys.path.append('{}/testing/'.format(os.getcwd()))\n"
    //"print(sys.path)\n"
    "import errorPythonPort\n"
    "import socketTester\n"
    ;
    PyRun_SimpleString(pythonImportCode);
    
    char runTestCode[] = 
    "socketTester.runTest()\n"
    ;
    PyRun_SimpleString(runTestCode);

    return 1;
}

/*
closes the python tester 
assuming: the python tester closes the testing directory
*/
inline int endPython(){
    Py_Finalize();
    return 1;
}

/*
Essentially a wrapper for startPython() - endPython()
so that it can be run on a separate thread
*/
inline void runPythonThread(){
    startPython();
    endPython();
}

/*
writes a file with the commandStr so that the python script
can communicate with this tester

makes commandPath more compatible
*/
inline int sendCommand(fs::path commandStr){
    commandStr.replace_extension(".comm");
    
    fs::path commandPath = TESTINGDIR / commandPath;
    std::ofstream commandFile(commandPath);
    commandFile.close();
    return 1;
}

/*
looks for "good" or "bad" file 
    if it exists,
    it deletes it 
    then returns SUCCESS or FAILURE
-1 if it cannot find such file
(only one should exist at a time)
*/
inline int readResult(){
    fs::path goodpath = TESTINGDIR / "good";
    if(fs::exists(goodpath)){
        fs::remove(goodpath);   // shouldnt need to check because 
                                //  it's checked in the if statement
        return SUCCESS;
    }

    fs::path badpath = TESTINGDIR / "bad";
    if(fs::exists(badpath)){
        fs::remove(badpath);
        return FAILURE;
    }
    return -1;
}


}