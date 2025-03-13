LIBDIRECTORY = ./libraries
HEADERS = ./headers
TESTDIRECTORY = ./testing
GENERALARGS = -I ${HEADERS} -L${LIBDIRECTORY} -std=c++20
PYTHONARGS = -I/usr/include/python3.12 -I/usr/include/python3.12 -fno-strict-overflow -Wsign-compare  -DNDEBUG -g -O2 -Wall -L/usr/lib/python3.12/config-3.12-x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lpython3.12 -ldl  -lm
instructions:
	@echo "Not implemented yet!!!!"
	@echo "Try 'make socketTest'"

socketTest: compileSocketTest runTest cleanTest

ringTest: compileRingTest runTest cleanTest

clientTest: compileClientTest runTest cleanTest

socketLib.o: socketLib.cpp ring.o
	g++ ${GENERALARGS} -c socketLib.cpp -o socketLib.o

compileSocketTest: socketLib.o ring.o ${TESTDIRECTORY}/errorCPPPort.hpp ${TESTDIRECTORY}/socketTester.cpp
	g++ ${TESTDIRECTORY}/socketTester.cpp socketLib.o ring.o ${GENERALARGS} -I ${TESTDIRECTORY} ${PYTHONARGS} -o test

ring.o: ring.cpp
	g++ ${GENERALARGS} -c ring.cpp -o ring.o

compileRingTest: ring.o ${TESTDIRECTORY}/ringTester.cpp
	g++ ${TESTDIRECTORY}/ringTester.cpp ring.o ${GENERALARGS} -o test

compileClientTest: socketLib.o ring.o ${TESTDIRECTORY}/errorCPPPort.hpp ${TESTDIRECTORY}/socketTester.cpp
	g++ ${TESTDIRECTORY}/clientTester.cpp socketLib.o ring.o ${GENERALARGS} -I ${TESTINGDIRECTORY} ${PYTHONARGS} -o test

runTest: test
	export LD_LIBRARY_PATH=${LIBDIRECTORY}
	./test

cleanTest: test
	rm test