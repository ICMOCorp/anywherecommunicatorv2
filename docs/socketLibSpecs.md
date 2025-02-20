# A Message

a "message" is a Megabyte of bytes (1024 * 1024 = 
1,048,576 bytes) which is split up into 3 parts:
```
- 3 bytes               (size of message)
- 13 bytes              (the username)
- 1,048, 560 bytes      (the message)
```
(i.e. a message of "hi" sent by john will contain the bytes
`33 6A 6F 68 6E 68 69`)

# Socket Class

### Prerequisites

- Built for linux system
- Depends on CircularRing data structure implemented by Min:
    (https://github.com/MiniMinja/CircularBuffer) - as a 
    dynamic library
- The socket is connected to the self IP address (127.0.0.1)

### Expected Behavior


## Constructor

### Prerequisites

### Expected Behavior

Initializes the different socket struct datas as well as the RingBufferS implementation (does not open the socket yet)


## `openSocket(int portstart, int portend)`

### Prerequisites

- `portstart <= portend`
- or `portend < 0` which means we only check 1 port

### Expected Behavior

Tries to open a socket in the range `[portstart, portend]`. 
For example, if it tries to open `portstart` but that port 
is already taken, then it opens in `portstart+1` all 
the way to `portend`

> Error: if it cannot open the port, it will return an error
of `NOGOODPORT` error


## `closeSocket()`

### Prerequisites

- socket should exist (?)

### Expected Behavior

closes the socket that is opened

> Error: if a socket isnt already opened, it will return
a `NOEXISTINGSOCKET_CLOSE` error (but no other side effects will
take place)

> Error: if some other happens, `UNEXPECTED_CLOSE` will
happen

## `getPort()`

### Prerequisites

### Expected Behavior

returns the port the socket is connected to. Gives `-1` if not
connected to a socket

# Free Functions

## `interpretError(int errCode)`

### Prerequisite

- predefined error codes listed in [Socket](#socket-class)

### Expected Behavior

it should print out a more detailed information about 
- What the error is
- Which function caused it
- And a description of what happend
- a stack trace (if possible)

> Error: an undefined error comes in, then print out `"Unexpected"`
and then print out the error value (and a stack trace if possible)
