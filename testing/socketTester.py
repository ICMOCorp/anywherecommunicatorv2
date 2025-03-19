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
        logging.info('Connection: {}'.format(str(connection)))
        return OPENED
    except:
        print("PYTHON:Could not connect in connectIt() function in socketTester.py\nfor some reason. Res value is {}".format(res))
        return CLOSED

'''
    scans a range of ports to connect to
    then connects to a valid port
    it waits for a PING from the port and then 
    and returns PONG back to the port
'''
def autoconnect(portStart, portEnd):
    pass

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
    return True

'''
    reads a bunch (hopefully grabbing the right amount of data)
    and returns whether or not checkMSG was
    matching and checkUserID was matching

    it tries to reread on finding nothing, and if it tries 50 times
    and still nothing, then it quits

    returns True if matching, False if not
'''
def readMessage():
    global connection
    p = select.poll()
    p.register(connection)
    msgCollection = []
    headerGot = 0
    tries = 1
    startTime = time.time()
    while headerGot < MSGSIZEBYTECOUNT:
        if time.time() - startTime > 10:
            print("PYTHON: GOT NOTHING in {}\n".format(
                                "readMessage() in socketTester.py"
                                ))
            print("even after waiting 10 seconds")
            print("PYTHON: Wasn't this supposed to be called after a send?") 
            return
        available = p.poll(10000) # ten second poll
        if len(available) == 0:
            print("PYTHON: GOT NOTHING in {}\n".format(
                                "readMessage() in socketTester.py"
                                ))
            print("even after waiting 10 seconds")
            print("PYTHON: Wasn't this supposed to be called after a send?") 
            return
        if available[0][1] & select.POLLIN == 0:
            continue

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
    startTime = time.time()
    while msgGot < packetDesiredSize:
        if time.time() - startTime > 10:
            print("PYTHON: GOT NOTHING in {}\n".format(
                                "readMessage() in socketTester.py"
                                ))
            print("even after waiting 10 seconds")
            print("PYTHON: Wasn't this supposed to be called after a send?") 
            return
        available = p.poll(10000) # ten second poll
        if len(available) == 0:
            print("PYTHON: GOT NOTHING in {}\n".format(
                                "readMessage() in socketTester.py"
                                ))
            print("even after waiting 10 seconds")
            print("PYTHON: Wasn't this supposed to be called after a send?") 
            return
        if available[0][1] & select.POLLIN == 0:
            continue
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
    return msg, userID


def disconnectIt():
    global connection
    if connection is None:
        print('PYTHON:Connection is {}'.format(str(connection)))
    connection.close()
    connection = None

    return True

'''
    This function will try to wait for "PING" which then it will
        respond with "PONG"
    This will hang forever in this function UNLESS the quit.comm 
        is found
'''
def pingpong():
    try: # Just copied from readMessage (trying to avoid calling it directly
        # because of SRP)
        global connection
        p = select.poll()
        p.register(connection)
        msgCollection = []
        headerGot = 0
        tries = 1

        d = {
            'POLLIN':select.POLLIN,
            'POLLPRI':select.POLLPRI,
            'POLLOUT':select.POLLOUT,
            'POLLERR':select.POLLERR,
            'POLLHUP':select.POLLHUP,
            'POLLRDHUP':select.POLLRDHUP,
            'POLLNVAL':select.POLLNVAL
        }
        '''
        logging.info('Table:')
        for key in d:
            logging.info('{:20s}{:20d}'.format(key, d[key]))
        '''

        startTime = time.time()
        while headerGot < MSGSIZEBYTECOUNT:
            if time.time() - startTime > 10:
                print("PYTHON: GOT NOTHING in {}\n.".format(
                                    "readMessage() in socketTester.py"
                                    ))
                print("even after waiting 10 seconds")
                print("PYTHON: Wasn't this supposed to be called after a send?") 
                return
            logging.info('Polling for message')
            available = p.poll(10000) # ten second poll
            logging.info('Got: {}'.format(str(available)))

            if len(available) == 0:
                print("PYTHON: GOT NOTHING in {}\n.".format(
                                    "readMessage() in socketTester.py"
                                    ))
                print("even after waiting 10 seconds")
                print("PYTHON: Wasn't this supposed to be called after a send?") 
                return
            if available[0][1] & select.POLLIN == 0:
                continue
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
        startTime = time.time()
        while msgGot < packetDesiredSize:
            if time.time() - startTime > 10:
                print("PYTHON: GOT NOTHING in {}\n.".format(
                                    "readMessage() in socketTester.py"
                                    ))
                print("even after waiting 10 seconds")
                print("PYTHON: Wasn't this supposed to be called after a send?") 
                return
            available = p.poll(10000) # ten second poll
            if len(available) == 0:
                print("PYTHON: GOT NOTHING in {}\n.".format(
                                    "readMessage() in socketTester.py"
                                    ))
                print("even after waiting 10 seconds")
                print("PYTHON: Wasn't this supposed to be called after a send?") 
                return
            if available[0][1] & select.POLLIN == 0:
                continue
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
        if msg != "PING" and userID != "SYS":
            return False
        print('PYTHON: Successfully got PING from SYS')
    except Exception as e:
        print('PYTHON: Something went wrong with PING in pingpong() function')
        print(e)
        return False

    try: #Just copied from sendMessage
        packet = convertToPacket("PONG", "client")
        # already declared? > global connection
        logging.info('id:')
        logging.info('\t>>{}'.format(userID))
        logging.info('Sent:')
        logging.info('\t>>{}'.format(msg))
        logging.info('asPacket:')
        logging.info('\t>>{}'.format(str(packet)))
        connection.sendall(packet)
    except Exception as e:
        print('PYTHON: Something went wrong with PONG in pingpong() function')
        print(e)
        return False
    print('PYTHON: SEND PONG from client')
    return True

'''
    This function does the same thing as PINGPONG but once it receives PING
    it not only notices it got PING
    but it deliberately doesn't respond
'''
def pingpongwait(numMSecs):
    numSecs = numMSecs / 1000

    global connection
    print('PYTHON: now we wait {}'.format(numSecs))
    start = time.time()
    while time.time() - start < numSecs:
        print("PYTHON: gathering next PING. Time elapsed: {:0.3f}".format(time.time() - start))
        try: # Just copied from readMessage (trying to avoid calling it directly
            # because of SRP)
            p = select.poll()
            p.register(connection)
            msgCollection = []
            headerGot = 0
            tries = 1

            d = {
                'POLLIN':select.POLLIN,
                'POLLPRI':select.POLLPRI,
                'POLLOUT':select.POLLOUT,
                'POLLERR':select.POLLERR,
                'POLLHUP':select.POLLHUP,
                'POLLRDHUP':select.POLLRDHUP,
                'POLLNVAL':select.POLLNVAL
            }
            #logging.info('Table:')
            #for key in d:
            #    logging.info('{:20s}{:20d}'.format(key, d[key]))
            restart = False
            startTime = time.time()
            while headerGot < MSGSIZEBYTECOUNT:
                if time.time() - startTime > 10:
                    print("PYTHON: GOT NOTHING in {}\n".format(
                                        "pingpongwait() in socketTester.py"
                                        ))
                    print("even after waiting 10 seconds")
                    restart = True
                    break
                logging.info('Polling for message')
                available = p.poll(1000)
                logging.info('Got: {}'.format(str(available)))

                if len(available) == 0:
                    print("PYTHON: GOT NOTHING in {}\n".format(
                                        "pingpongwait() in socketTester.py"
                                        ))
                    print("even after waiting 10 seconds")
                    print("PYTHON: Wasn't this supposed to be called after a send?") 
                    break
                if available[0][1] & select.POLLIN == 0:
                    continue
                headerBuffer = connection.recv(MSGSIZEBYTECOUNT)
                if len(headerBuffer) <= 0:
                    logging.error('Something went wrong with recv (header part of readMessage)')
                    raise ConnectionError('Something when wrong with recv\n in getting header in readMessage() in socketTester.py')
                logging.info('Got buffer: ')
                logging.info('\t>>{}'.format(str(headerBuffer)))
                msgCollection.append(headerBuffer)
                headerGot += len(headerBuffer)
            
            if restart:
                continue
            
            # note that at this point point we dont know if headerGot is the size of 
            # the msg size 
            #   or if its bigger
            # combining the multi part message containing the header into a single part
            msgCollection = [b''.join(msgCollection)]

            #using this combined >= header-sized msg just to grab the size of the message
            msgLength= convertToSize(b'\0' + msgCollection[0][:3])
            packetDesiredSize = msgLength + MSGSIZEBYTECOUNT + IDBYTEHEADERBYTES
            msgGot = headerGot
            startTime = time.time()
            while msgGot < packetDesiredSize:
                if time.time() - startTime > 10:
                    print("PYTHON: GOT NOTHING in {}\n".format(
                                        "readMessage() in socketTester.py"
                                        ))
                    print("even after waiting 10 seconds")
                    print("PYTHON: Wasn't this supposed to be called after a send?") 
                    return
                available = p.poll(1000)
                if available[0][1] & select.POLLIN == 0:
                    continue
                if len(available) == 0 or available[0][1] & select.POLLIN == 0:
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
            if msg != "PING" and userID != "SYS":
                return False
            print('PYTHON: Successfully got PING from SYS')
        except Exception as e:
            print('PYTHON: Something went wrong with PING in pingpongwait() function')
            print(e)
            return False
        print("PYTHON: FOUND PING")
    print("PYTHON: Time's up! Time elapsed: {:0.3f}".format(time.time() - start))
    if numSecs - time.time() + start > 0:
        print("PYTHON: SLEEP: zzzz ({} seconds left)".format(numSecs - time.time() + start))
        time.sleep(numSecs - time.time() + start)

    try: #Just copied from sendMessage
        packet = convertToPacket("PONG", "client")
        # already declared? > global connection
        logging.info('id:')
        logging.info('\t>>{}'.format(userID))
        logging.info('Sent:')
        logging.info('\t>>{}'.format(msg))
        logging.info('asPacket:')
        logging.info('\t>>{}'.format(str(packet)))
        connection.sendall(packet)
    except Exception as e:
        print('PYTHON: Something went wrong with PONG in pingpong() function')
        print(e)
        return False
    print('PYTHON: Sent PONG from client')
    return True

def verifyconnect(port):
    res = connectIt(port)
    if res == CLOSED:
        print("PYTHON: Could not verify")
        return False
    
    res = pingpong()
    if not res:
        print("PYTHON Could not pingpong!")
        return False
    
    return True

def processquery():
    msg, userID = readMessage()

    msgL = msg.split(" ")
    while len(str(msgL)) > MEGABYTE:
        msgL.pop()
    msgS = str(msgL)
    
    sendMessage(msgS, userID)
    return True

# Test for socket
def runTest():
    '''
    Command lists:
        - connect@<port>
        - autoconnect@<portstart, portend>
        - send@<id> - msg is in file, it should be the second argument
        - read@<id> - check msg is in file, it should be the second argument
        - disconnect
        - pingpong
            > Tries to read "PING" and returns a "PONG" on return.
            > This will sit and wait on this command unless it ends by "quit"
        - pingpongwait@<numMSecs>
            > Tries to read "PING" but does NOT return "PONG" on return
            > UNTIL 'numMSecs' seconds passes
            > again sits and waits for input unless it ends by "quit"
        - verifyconnect@<port>
            just calls connect@<port>
            then pingpong()
        - processquery
            just calls read
            and then send@<id>
                where the content is simply the same thing, but everything is turned into a list
    '''
    tester = eport.generateTester({
        "verifyOpen": lambda args: checkSocket(args[0]) == OPENED, 
        "verifyClose": lambda args: checkSocket(args[0]) == CLOSED,
        "connect": lambda args: connectIt(args[0]) == OPENED,
        "pingpong": lambda args: pingpong(),
        "pingpongwait": lambda args:pingpongwait(int(args[0])),
        "verifyconnect": lambda args:verifyconnect(args[0]) == OPENED,
        "send" : lambda args: sendMessage(args[1], args[0]),
        "read" : lambda args: readMessage() == (args[1], args[0]),
        "processquery": lambda args:processquery(),
        "disconnect": lambda args: disconnectIt()
    })

    eport.run(tester)

