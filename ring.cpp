#include "ring.hpp"

#include <stdexcept>
#include <iostream>

void ringbuffer::bufferError(std::string msg){
    std::cout << msg << std::endl;
    throw std::runtime_error("Quit by error");
}

ringbuffer::RingBufferS::RingBufferS(){
    this->maxLength = 0;
    start = end = -1;
    currSize = 0;
}

ringbuffer::RingBufferS::RingBufferS(size_t maxLength){
    if(maxLength == 0){
        throw std::invalid_argument("FATAL ERROR: the max length cannot be 0.\n"
                                    "RingBuffer could not be created");
    }
    this->maxLength = maxLength;
    start = end = 0;
    data.resize(maxLength, '\0');
    currSize = 0;
}

ringbuffer::RingBufferS::RingBufferS(const RingBufferS& other){
    this->data = std::vector<char>(other.data);
    this->maxLength = other.maxLength;
    this->start = other.start;
    this->end = other.end;
}

int ringbuffer::RingBufferS::push(const std::string& toAdd, size_t size){
    if(size > maxLength - currSize){ //a fancier version to account for 
                                    // overflow error as well
        return ringbuffer::OUTOFBOUNDS;
    }
    //some defensive programming
    if(end >= maxLength){
        ringbuffer::bufferError("FATAL ERROR: end variable is out of bounds\n"
                                "in push() function in RingBufferS in ring.hpp");
    }
    for(size_t i = 0;i<size;i++){
        data[end] = toAdd[i];
        end++;
        
        //loop end if so it goes around
        if(end >= maxLength){
            end = 0;
        }
    }
    currSize += size;
    return 1;
}

int ringbuffer::RingBufferS::pop(std::string& dest, size_t popAmount){
    //some defensive programming
    if(maxLength == 0){
        ringbuffer::bufferError("FATAL ERROR: you're using an NULL ring buffer (a ring buffer of size 0)\n"
                                "in pop() function in RingBufferS in ring.hpp");
    }
    if(start >= maxLength){
        ringbuffer::bufferError("FATAL ERROR: start variable is out of bounds\n"
                                "in pop() function in RingBufferS in ring.hpp");
    }
    //a written out min() function
    size_t limit = popAmount;
    bool limitSet = false;
    if(limit > currSize){
        limit = currSize;
        limitSet = true;
    }
    for(size_t i = 0;i<limit;i++){
        dest += data[start];
        start++;

        //loop the start so it goes around
        if(start >= maxLength){
            start = 0;
        }
    }
    if(limitSet){
        currSize = 0;
        return OUTOFBOUNDS;
    }
    currSize -= popAmount;
    return 1;
}

std::string ringbuffer::RingBufferS::getContents(){
    //some defensive programming
    if(maxLength == 0){
        ringbuffer::bufferError("FATAL ERROR: you're using an NULL ring buffer (a ring buffer of size 0)\n"
                                "in getContents() function in RingBufferS in ring.hpp");
    }
    if(start >= maxLength){
        ringbuffer::bufferError("FATAL ERROR: start variable is out of bounds\n"
                                "start: " + std::to_string(start) + " maxLength: " + std::to_string(maxLength) + "\n"
                                "in getContents() function in RingBufferS in ring.hpp");
    }
    if(end >= maxLength){
        ringbuffer::bufferError("FATAL ERROR: end variable is out of bounds\n"
                                "end: " + std::to_string(end) + " maxLength: " + std::to_string(maxLength) + "\n"
                                "in getContents() function in RingBufferS in ring.hpp");
    }
    if(currSize > maxLength){
        ringbuffer::bufferError("FATAL ERROR: currSize variable is larger than maxLength\n"
                                "currSize: " + std::to_string(currSize) + " maxLength: " + std::to_string(maxLength) + "\n"
                                "in getContents() function in RingBufferS in ring.hpp");
    }
    size_t wrappedIndex = start;
    if(currSize > maxLength - start){
        wrappedIndex = currSize - (maxLength - start);
    }
    else{
        wrappedIndex += currSize;
    }
    if(wrappedIndex != end){
        ringbuffer::bufferError("FATAL ERROR: end variable does not match the index calculated to be the actual end\n"
                                "end: " + std::to_string(end) + " wrappedIndex: " + std::to_string(wrappedIndex) + "\n"
                                "in getContents() function in RingBufferS in ring.hpp");
    }

    size_t accessIndex = start;
    std::string ret = "";
    for(size_t i = 0;i<currSize;i++){
        ret += data[accessIndex];
        accessIndex++;
        if(accessIndex >= maxLength){
            accessIndex = 0;
        }
    }
    return ret;
}

bool ringbuffer::RingBufferS::isEmpty(){
    return currSize == 0;
}

size_t ringbuffer::RingBufferS::size(){
    return currSize;
}