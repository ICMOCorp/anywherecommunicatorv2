import errorPythonPort as eport

import struct
import socket
import select
import time

import logging
logging.basicConfig(
    filename = 'lastInteraction.log',
    encoding='utf-8',
    filemode = 'w',
    level=logging.DEBUG
)
logging.warning("Please rename this if you want a record")

MEGABYTE = 1024 * 1024
MSGSIZEBYTECOUNT = 3
IDBYTEHEADERBYTES = 13

OPENED = 1
CLOSED = 2

# PACKET ORDER (msg, userID)
# as a reminder so I can type /ORDER if I forget

'''
    gets a string message and converts it into the
    predefined packet format before converting it 
    into a bytes data to send


    if the message turns out to be too big, it will
    not send the message but instead will return message too big
'''
def convertToPacket(msg, userID):
    numBytes = len(msg)
    if numBytes > MEGABYTE - MSGSIZEBYTECOUNT - IDBYTEHEADERBYTES:
        raise ValueError('Message to send over network is too big! (size: {})'.format(numBytes))

    if len(userID) < 13:
        userID = "{:<13s}".format(userID)

    packetData = []
    packetData.append(struct.pack('>I', numBytes)[1:])
    packetData.append(str.encode(userID))
    packetData.append(str.encode(msg))

    packet = b"".join(packetData)
    return packet

'''
    TLDR: >> (msg, userID)
    converts a bytes object into a message string and userID tuple
    where the message string contains the message
    and userID is contains the userID for the message (with whitespace-padding
    stripped away)
'''
def convertFromPacket(buffer):
    userIDAsBytes = buffer[3:16]
    msgAsBytes = buffer[16:]

    userID = userIDAsBytes.decode('utf-8')
    msg = msgAsBytes.decode('utf-8')
    return msg, userID

'''
    we cant just grab a packet so we need to extrapolate
    from a bunch of recv calls. We'll pass in a bytes object
    of size 4 but the first byte is 0x00 (struct unpack needs
    a buffer of size 4)
'''
def convertToSize(buffer):
    return struct.unpack('>I', buffer)[0] # unpack() returns a tuple

global connection
connection = None

'''
    Checks to see the socket port is open
    port is a STR not int

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

'''
    Makes a connection to the socket  and
    saves it to the global variable connection

    here port is a string, NOT an integer
'''
def connectIt(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect(('127.0.0.1', int(port)))
        s.setblocking(0)
        global connection
        connection = s
        logging.info("Connected to '127.0.0.1:{}".format(int(port)))
        return OPENED
    except:
        print("PYTHON:Could not connect in connectIt() function in socketTester.py\nfor some reason. Res value is {}".format(res))
        return CLOSED

'''
    first converts the msg and userID to the
    prefined packet of bytes

    Then sends the packet over to the client

    returns 1 on a successful send
'''
def sendMessage(msg, userID):
    packet = convertToPacket(msg, userID)
    global connection
    logging.info('id:')
    logging.info('\t>>{}'.format(userID))
    logging.info('Sent:')
    logging.info('\t>>{}'.format(msg))
    logging.info('asPacket:')
    logging.info('\t>>{}'.format(str(packet)))
    connection.sendall(packet)
    return 1

'''
    reads a bunch (hopefully grabbing the right amount of data)
    and returns whether or not checkMSG was
    matching and checkUserID was matching

    it tries to reread on finding nothing, and if it tries 50 times
    and still nothing, then it quits

    returns True if matching, False if not
'''
def readMessage(checkMsg, checkUserID):
    global connection
    p = select.poll()
    p.register(connection)
    msgCollection = []
    headerGot = 0
    tries = 1
    while headerGot < MSGSIZEBYTECOUNT:
        available = p.poll(10000) # ten second poll
        if len(available) == 0:
            print("PYTHON: GOT NOTHING in {}\n.".format(
                                "readMessage() in socketTester.py"
                                ))
            print("even after waiting 10 seconds")
            print("PYTHON: Wasn't this supposed to be called after a send?") 
            return
        headerBuffer = connection.recv(MSGSIZEBYTECOUNT)
        if len(headerBuffer) <= 0:
            logging.error('Something went wrong with recv (header part of readMessage)')
            raise ConnectionError('Something when wrong with recv\n in getting header in readMessage() in socketTester.py')
        logging.info('Got buffer: ')
        logging.info('\t>>{}'.format(str(headerBuffer)))
        msgCollection.append(headerBuffer)
        headerGot += len(headerBuffer)
    
    # note that at this point point we dont know if headerGot is the size of 
    # the msg size 
    #   or if its bigger
    # combining the multi part message containing the header into a single part
    msgCollection = [b''.join(msgCollection)]

    #using this combined >= header-sized msg just to grab the size of the message
    msgLength= convertToSize(b'\0' + msgCollection[0][:3])
    packetDesiredSize = msgLength + MSGSIZEBYTECOUNT + IDBYTEHEADERBYTES
    msgGot = headerGot
    while msgGot < packetDesiredSize:
        available = p.poll(10000) # ten second poll
        if len(available) == 0:
            print("PYTHON: GOT NOTHING in {}\n.".format(
                                "readMessage() in socketTester.py"
                                ))
            print("even after waiting 10 seconds")
            print("PYTHON: Wasn't this supposed to be called after a send?") 
            return
        '''
        print('Message: {}'.format(str(msgCollection)))
        print('Available: {}'.format(str(available)))
        print('Got: {}/{}'.format(msgGot, packetDesiredSize))
        '''
        headerBuffer = connection.recv(packetDesiredSize - msgGot)# I don't want to accidentally grab
                                                            # the next message's information
        if len(headerBuffer) <= 0:
            logging.error('Something went wrong with recv (body part of readMessage)')
            raise ConnectionError('Something when wrong with recv\nin getting body in readMessage() in socketTester.py')
        logging.info('Got buffer: ')
        logging.info('\t>>{}'.format(str(headerBuffer)))
        msgCollection.append(headerBuffer)
        msgGot += len(headerBuffer)

    packet = b''.join(msgCollection)

    msg, userID = convertFromPacket(packet)
    userID = userID.strip()
    print('PYTHON: OUR PACKAGE: {} <FROM: {}>'.format(msg, userID))
    return msg == checkMsg and userID == checkUserID


def disconnectIt():
    global connection
    if connection is None:
        print('PYTHON:Connection is {}'.format(str(connection)))
    connection.close()
    connection = None

    return True

# Test for socket
def runTest():
    '''
    Command lists:
        - connect@<port>
        - send@<id> - msg is in file, it should be the second argument
        - read@<id> - check msg is in file, it should be the second argument
        - disconnect
    '''
    tester = eport.generateTester({
        "verifyOpen": lambda args: checkSocket(args[0]) == OPENED, 
        "verifyClose": lambda args: checkSocket(args[0]) == CLOSED,
        "connect": lambda args: connectIt(args[0]) == OPENED,
        "send" : lambda args: sendMessage(args[1], args[0]) == 1,
        "read" : lambda args: readMessage(args[1], args[0]) == True,
        "disconnect": lambda args: disconnectIt()
    })

    eport.run(tester)

