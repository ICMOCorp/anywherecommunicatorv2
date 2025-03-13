import sys
import os

TESTINGDIRECTORY = "{}/testing".format(os.getcwd())
print("PYTHON: Testing at: {}".format(TESTINGDIRECTORY))

PASSED =  'good' 
FAILED =  'bad'

BADCOMMAND =            -10
MULTIPLECOMMANDS =      -11

# prints an error if the tester fails according to the codes
# listed above
# then exits the program
def pError(errorCode):
    # print("Error: ", end='')
    if errorCode == MULTIPLECOMMANDS:
        # print("Too many commands in directory")
        pass
    else:
        # print("Invalid error code {}".format(erroCode))
        pass
    sys.exit()

# prints the directory of the currently running test
#   (to see if the test itself works or not)
def printDirectory(path):
    print("Running Test at {}".format(path))

# gets the file that ends in .comm, the file that is 
# compatible with our tester
# 
# if there is not 1 (2 or more) file,
# then this quits with a MULTIPLECOMMANDS error
#
# if there is none, it will return None
def getNextCommand(path):
    listing = os.listdir(path)
    itemGot = None
    itemCount = 0
    for item in listing:
        if os.path.isfile(path+"/"+item) and item.endswith('.comm'):
            itemGot = item
            itemCount += 1
    if itemCount > 1:
        pError(MULTIPLECOMMANDS)
    else:
        return itemGot
    
# given a path and name of the command (the file)
# it will simply read the file and return its contents
def readContentsOfCommand(path, commandName):
    content = None
    try:
        with open(path + "/" + commandName) as f:
            content = f.read()
    except:
        print("PYTHON>>>Error reading {}/{}. in readContentsOfCommand() function".format(
                                path, commandName
                            ))
    return content

# takes a command string (the name of the file)
#   of the form:
#       <command>(@<arg1>&<arg2>&<arg3>...)
# and returns a tuple where the first index
# is the parsed command and the other
# remaining index are the arguments to the command
# 
# will not check for bad commands here
# in terms of the '@' and '&' formatting because the processing
# is a little bit above "simple"
#
#empty args are returned as an empty list
def parseCommand(commandStr):
    if '@' not in commandStr:
        return commandStr, []
    command, rest = commandStr.split("@") 
    rest = rest.split("&")
    return command, rest

# takes a command listed in the command 
# listing and executes the command
# an invalid command when parsed
# will return BADCOMMAND error and quit
def executeCommand(testFunc, args=()):
    output = testFunc(args) 
    
    if output:
        return PASSED
    else:
        return FAILED

# takes the a result and outputs a file
# where its name is the result ('good' or 'bad')
#
# we'll let the receiver of "good" or "bad" (the
#   C++ tester) be responsible for cleaning up
def outputResult(result, path):
    f = open(path +'/'+ result, 'w')
    f.close()

# used to set up the environment before reading for commands
#  - If the path doesnt exist, it will be made
#  - If any commands exist, it get deleted
#  - If any results exist ('good' or 'bad'), it gets deleted
def setup(path):
    if not os.path.isdir(path):
        os.mkdir(path)
        return

    toRemove = []
    for file in os.listdir(path):
        if file.endswith('.comm') or file == 'good' or file == 'bad':
            toRemove.append(file)
    
    for file in toRemove:
        os.remove(path + "/" + file)



# cleans up the path defined by path
#  - If the command exists, it will be deleted
def cleanCommand(command, path):
    if os.path.isfile(path + "/" + command):
        os.remove(path + "/" + command)

'''
    This function is kind of like a queue (with capacity
    of 1) where it scans for any incoming commands
    and runs it. This is *THE* function you need to run
    when tesing

    once an available file/command is found, it will 
    parse it,
    execute it,
    and produce an output

    All you need to do for any tester is 
        1. Make a Tester object
        2. Run Tester
'''
def run(tester):
    print("PYTHON: Starting Python Tester!")
    setup(TESTINGDIRECTORY)

    running = True
    while running:
        commandFilename = getNextCommand(TESTINGDIRECTORY)
        if commandFilename is None:
            continue
        elif commandFilename == 'quit.comm':
            running = False
        else:
            commandStr = commandFilename[:commandFilename.index('.comm')]
            command, args = parseCommand(commandStr)
            print('PYTHON: Found command {} ({})'.format(command, str(args)))
            if command == "send" or command == "read":
                msg = readContentsOfCommand(TESTINGDIRECTORY, commandFilename) 
                args.append(msg)
                print('PYTHON: It is a {} command. Read contents of file. ({}...)'.format(command, msg[:10]))
            tf = tester.getTestFunction(command)
            if tf is None:
                print('PYTHON: INVALID COMMAND')
                continue
            result = executeCommand(tf, args)
            res = outputResult(result, TESTINGDIRECTORY)
        
        # we should be done with everything here right?
        cleanCommand(commandFilename, TESTINGDIRECTORY)
    print("PYTHON: Closing Tester")


# it's a wrapper for a testFunction collection so that it can
# exist as an object
#
# I'm sure there's a more elegant way, but I wanted to 
# generalize a function call so that this module 
# can be independent on the kind of test we want 
#
# Every Tester gets equipped with two functions for verification
# purposes. Tester gets a "test1" and "test2" command where
# "test1" return true and "test2" returns false. We can test
# to see this works by calling these functions and ensure
# the test is working as well
class Tester:
    def __init__(self):
        self.commands = {
            "test1": lambda args: True,
            "test2": lambda args: False
        }

    # Adds the command->testFunction pair to the 
    # tester
    def addTestFunc(self, command, testFunction):
        self.commands[command] = testFunction

    # given the command, it finds the corresponding
    # test function
    #
    # if the command doesnt exist, it will return
    # None
    def getTestFunction(self, command):
        if command not in self.commands:
            return None
        return self.commands[command]

# generates a Tester from a list of commands and 
# corresponding functions
def generateTester(commandDict):
    retTest = Tester()
    for command in commandDict:
        retTest.addTestFunc(command, commandDict[command])
    return retTest


# basic tester for check
def runBasicTest():
    run(Tester())
