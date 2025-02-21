import errorPythonPort as eport

import socket

OPENED = 1
CLOSED = 2

'''
    Checks to see the socket port is open

    Note:
        This program verifies this by CONNECTING
        to the port. It's not like netstat that 
        scans which ports are open
'''
def checkSocket(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = s.connect_ex(('127.0.0.1',int(port)))   # this is connect() but without 
                                                # without raising an exception
    s.close()
    if result == 0:
        return OPENED
    else:
        return CLOSED

# Test for socket
def runTest():
    tester = eport.generateTester({
        "verifyOpen": lambda args: checkSocket(args[0]) == OPENED, 
        "verifyClose": lambda args: checkSocket(args[0]) == CLOSED
    })

    eport.run(tester)

