#include "ring.hpp"

RingBufferS::RingBufferS(size_t maxLength){
    this->maxLength = maxLength;
    start = end = 0;
    data.resize(maxLength, '\0');
}

/*Copy and swap idiom*/
RingBufferS::RingBufferS(const RingBufferS& other){
    this->data = std::vector<char>(other.data);
    this->maxLength = other.maxLength;
    this->start = other.start;
    this->end = other.end;
}

/*pushes characters toAdd to data*/
void RingBufferS::push(const char* buffer, size_t size){
    for(size_t i = 0;i<size;i++){
        data[end] = buffer[i];
        end++;
        
        //loop end if so it goes around
        if(end >= maxLength){
            end = 0;
        }
    }
}

/*pushes characters in our data to destination*/
void RingBufferS::pop(char* dest, size_t popAmount){
    for(size_t i = 0;i<popAmount;i++){
        dest[i] = data[start];
        start++;

        //loop the start so it goes around
        if(start >= maxLength){
            start = 0;
        }
    }
}