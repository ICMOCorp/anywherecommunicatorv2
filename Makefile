LIBDIRECTORY = ./libraries
HEADERS = ./headers
TESTDIRECTORY = ./testing
GENERALARGS = -I ${HEADERS} -L${LIBDIRECTORY} -std=c++20
PYTHONARGS = -I/usr/include/python3.12 -I/usr/include/python3.12 -fno-strict-overflow -Wsign-compare  -DNDEBUG -g -O2 -Wall -L/usr/lib/python3.12/config-3.12-x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lpython3.12 -ldl  -lm
instructions:
	@echo "Not implemented yet!!!!"
	@echo "Try 'make socketTest'"

socketTest: compileSocketTest runTest cleanTest

socketLib.o: socketLib.cpp
	g++ ${GENERALARGS} -c socketLib.cpp -o socketLib.o

compileSocketTest: socketLib.o ${TESTDIRECTORY}/errorCPPPort.hpp ${TESTDIRECTORY}/socketTester.cpp
	g++ ${TESTDIRECTORY}/socketTester.cpp socketLib.o ${GENERALARGS} -I ${TESTDIRECTORY} ${PYTHONARGS} -o test

runTest: test
	export LD_LIBRARY_PATH=${LIBDIRECTORY}
	./test

cleanTest: test
	rm test