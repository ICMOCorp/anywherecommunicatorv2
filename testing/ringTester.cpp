#include "ring.hpp"
#include "testingSuite.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

void testBadInit(){
    testing::TestSuite t("Bad init - when maxLength is 0", "ring.hpp");
    bool errorGiven = false;
    try{
        ringbuffer::RingBufferS buffer(0);
    }
    catch(const std::invalid_argument& e){
        std::cout << e.what() << std::endl;
        errorGiven = true;
    }

    t.test("Should be an exception", errorGiven);

    t.printFinalOutput();
}
void testSimpleOperations(){
    testing::TestSuite t("Simple operations", "ring.hpp");

    ringbuffer::RingBufferS buffer(10);

    //simple push case
    std::string simple = "hello";
    size_t simpleSize = simple.size();
    bool pushed = false;
    try{
        buffer.push(simple, simpleSize);
        pushed = true;
    }catch(const std::runtime_error& e){
        std::cout << ">>There was an error!" << std::endl;
    }
    t.test("Simple push case (\"Hello\")", pushed);

    //size test
    std::cout << "Size is " << buffer.size() << std::endl;
    t.test("size test - should be " + std::to_string(simpleSize), 
                        buffer.size() == simpleSize);

    //contents test
    std::string content = buffer.getContents();
    std::cout << "Content is " << content << std::endl;
    t.test("contents test", content == simple);

    //empty test
    t.test("empty test", !buffer.isEmpty());

    //pop test
    const size_t popAmount = 3;
    std::string poppedContent;
    bool popped = false;
    try{
        buffer.pop(poppedContent, popAmount);
        popped = true;
    }catch(const std::runtime_error& e){
        std::cout << ">>Error happend" << std::endl;
    }
    t.test("Simple pop", popped);

    //expectation test
    std::string expectedPoppedContent = "hel";
    std::cout << "popped content is " << poppedContent << std::endl;
    t.test("expectation test", expectedPoppedContent == poppedContent);

    //size test (pop)
    size_t expectedSize = simpleSize - popAmount;
    t.test("size test pt 2 (pop)", buffer.size() == expectedSize);

    //content test (pop)
    content = buffer.getContents();
    std::string expectedContent = "lo";
    std::cout << "current content is " << content << std::endl; 
    t.test("content test pt 2 (pop)", expectedContent == content);

    //empty test (pop)
    t.test("empty test pt 2 (pop) - should be false", !buffer.isEmpty());


    //pop everything case
    popped = false;
    poppedContent.clear();
    try{
        buffer.pop(poppedContent, 2);
        popped = true;
    }catch(const std::runtime_error& e){
        std::cout << ">>There was an error!" << std::endl;
    }
    t.test("pop everything case", popped);

    //expectation test
    expectedPoppedContent = "lo";
    std::cout << "popped content is " << poppedContent << std::endl;
    t.test("expectation test", expectedPoppedContent == poppedContent);

    //size test (pop)
    expectedSize = 0;
    t.test("size test pt 3 (everything gone)", buffer.size() == expectedSize);

    //content test (pop)
    content = buffer.getContents();
    expectedContent = "";
    std::cout << "current content is " << content << std::endl; 
    t.test("content test pt 3 (everything gone)", expectedContent == content);

    //empty test (pop)
    t.test("empty test pt 3 (everything gone) - should be false", buffer.isEmpty());

    t.printFinalOutput();
}
void testBadOperations(){
    testing::TestSuite t("Bad operations", "ring.hpp");

    ringbuffer::RingBufferS buffer(10);

    //start simple
    std::string simple = "hello";
    size_t simpleSize = simple.size();
    bool pushed = false;
    try{
        buffer.push(simple, simpleSize);
        pushed = true;
    }catch(const std::runtime_error& e){
        std::cout << ">>There was an error!" << std::endl;
    }
    t.test("Simple push case (\"Hello\")", pushed);

    //contents test
    std::string content = buffer.getContents();
    std::cout << "Content is " << content << std::endl;
    t.test("contents test", content == simple);
    
    //overpush test
    std::string bigstring = "what the heck, when did we as for this";
    pushed = false;
    try{
        int val = buffer.push(bigstring, bigstring.size());
        std::cout << "return value is " << val << std::endl;
        pushed = val == 1;
    }catch(const std::runtime_error& e){
        //here just cuz we need to catch something
    }
    t.test("overpush test", !pushed);

    //contents test
    content = buffer.getContents();
    std::cout << "Content is " << content << std::endl;
    t.test("contents test (should be " + simple + ")", content == simple);
    
    //push again test
    std::string anotherstring = "bye";
    pushed = false;
    try{
        buffer.push(anotherstring, anotherstring.size());
        pushed = true;
    }catch(const std::runtime_error& e){
        std::cout << ">>There was an error" << std::endl;
    }
    t.test("since we used try-catch, push should succeed", pushed);

    simple += anotherstring;
    //contents test
    content = buffer.getContents();
    std::cout << "Content is " << content << std::endl;
    t.test("contents test (should be " + simple + ")", content == simple);

    //too many pop test
    const size_t popAmount = 100;
    std::string poppedContent;
    bool popped = false;
    int val = 0;
    try{
        val = buffer.pop(poppedContent, popAmount);
        popped = true;
    }catch(const std::runtime_error& e){
        std::cout << ">>Error happend" << std::endl;
    }
    t.test("MANY pop", popped && val == ringbuffer::OUTOFBOUNDS);

    //expectation test
    std::string expectedPoppedContent = "hellobye";
    std::cout << "popped content is " << poppedContent << std::endl;
    t.test("expectation test", expectedPoppedContent == poppedContent);

    //size test (pop)
    size_t expectedSize = 0;
    t.test("size test (everything pop)", buffer.size() == expectedSize);

    //content test (pop)
    content = buffer.getContents();
    std::string expectedContent = "";
    std::cout << "current content is " << content << std::endl; 
    t.test("content test (everything pop)", expectedContent == content);

    //empty test (pop)
    t.test("empty test (everything pop) - should be true", buffer.isEmpty());

    t.printFinalOutput();
}

void testLimits(){

}

int main(){
    testBadInit();
    testSimpleOperations();
    testBadOperations();

    return 0;
}

