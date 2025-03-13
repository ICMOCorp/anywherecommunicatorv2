#pragma once

#include <iostream>
#include <string>
#include <cstdint>

namespace sharedstuff{
const uint32_t Megabyte =        1024 * 1024;
const uint32_t HEADERSIZE =      16;
const int MAXWAITSECS =          3;              //subject to change depending on performance

const int INVALIDMSGCOUNT =      Megabyte + 1;
const int STRINGTOOBIG =         Megabyte + 10;
const int NUMTOOBIG =            Megabyte + 11;
const int STRINGTOOSMALL =       Megabyte + 12;

const int LITTLE =               1;
const int BIG =                  13;
const int MSGSIZEBYTECOUNT =     3;
const int IDSIZEBYTECOUNT =      13;

/*a function that determines the endianness of the system given by
    -> https://stackoverflow.com/questions/4181951/how-to-check-whether-a-system-is-big-endian-or-little-endian
*/
inline int endianness(){
    int n = 1;
    if(*(char *)&n == 1){
        return LITTLE;
    }

    return BIG;
}

/*a function that converts the message count byte as 
bytes of an integer in little Endian
*/
inline uint32_t strToLittleEndian(uint32_t& val, const std::string& s){
    //defensive programming
    if(s.size() > MSGSIZEBYTECOUNT){
        return STRINGTOOBIG;
    }

    val = 0;
    for(size_t i = 0;i<s.size();i++){
        char byte = s[i];
        val |= ((int)byte) << (MSGSIZEBYTECOUNT-i);
    }
    return 1;
}

/* a function that converts an integer in littleEndian mode
to a str
*/
inline uint32_t littleEndianToStr(uint32_t val, std::string& s){
    //defensive programming
    if(val > Megabyte){
        return NUMTOOBIG;
    }

    char numS[3];
    for(size_t i = 0;i<3;i++){
        numS[i] = (val & (0xFF << (i))) >> (i);
    }
    s = std::string(numS);
    return 1;
}

/*a function that converts the message count bytes as 
bytes of an integer in big Endian
*/
inline uint32_t strToBigEndian(uint32_t& val, const std::string& s){
    //defensive programming
    if(s.size() > MSGSIZEBYTECOUNT){
        return STRINGTOOBIG;
    }

    val = 0;
    for(size_t i = 0;i<s.size();i++){
        char byte = s[i];
        val |= ((int)byte) << i;
    }
    return 1;
}

/* a function that converts an integer in bigEndian mode
to a str
*/
inline uint32_t bigEndianToStr(uint32_t val, std::string& str){
    //defensive programming
    if(val > Megabyte){
        return NUMTOOBIG;
    }

    char numS[MSGSIZEBYTECOUNT];
    for(size_t i = 0;i<MSGSIZEBYTECOUNT;i++){
        numS[i] = (val & (0xFF << (MSGSIZEBYTECOUNT-i))) >> (MSGSIZEBYTECOUNT-i);
    }
    str = std::string(numS);
    return 1;
}

/*a function that checks which endianness the system is and
returns the proper integer conversion*/
inline uint32_t strToUint(const std::string& s){
    //endianness doesn't matter except for how it is 
    // layed out in memory. Any bit operations will behave
    // the same way.
    // The code below vvv doesn't matter
    /*
    int sysendianness = endianness();
    uint32_t ret = -1;
    uint32_t val;
    if(sysendianness == LITTLE){
        val = strToLittleEndian(ret, s);
    }
    else{
        val = strToBigEndian(ret, s);
    }
    if(val == STRINGTOOBIG){
        return STRINGTOOBIG;
    }
    return ret;
    */
    if(s.size() > MSGSIZEBYTECOUNT){
        return STRINGTOOBIG;
    }
    if(s.size() < MSGSIZEBYTECOUNT){
        return STRINGTOOSMALL;
    }
    uint32_t val = 0;
    for(int i = 0;i<MSGSIZEBYTECOUNT;i++){
        //std::cout << std::hex << (int)s[i] << " " << ((MSGSIZEBYTECOUNT-i-1)* 8) << std::endl;
        val |= ((uint32_t)s[i]) << ((MSGSIZEBYTECOUNT-i-1) * 8);
    }
    return val;
}

/*a function that checks which endianness the system is and
returns the proper integer conversion*/
inline std::string uintToStr(uint32_t num){
    //endianness doesn't matter except for how it is 
    // layed out in memory. Any bit operations will behave
    // the same way.
    // The code below vvv doesn't matter
    /*
    int sysendianness = endianness();
    std::string ret;
    uint32_t val;
    if(sysendianness == LITTLE){
        val = littleEndianToStr(num, ret);
    }
    else{
        val = strToBigEndian(num, ret);
    }
    if(val == NUMTOOBIG){
        std::cout << "Inputted number was too big" << std::endl
                    << "Num was " << num << std::endl
                    << "in uintToStr() in sharedstuff.hpp" << std::endl;
        return "";
    }
    return ret;
    */
    if(num > Megabyte){
        return "";
    }
    std::string ret;
    ret.resize(MSGSIZEBYTECOUNT, ' ');
    //std::cout << "---converting the integer---" << std::endl;
    for(int i = 0;i<MSGSIZEBYTECOUNT;i++){
        /*
        std::cout << "ret: ";
        for(int j = 0;j<ret.size();j++){
            if(j > 0) std::cout << ", ";
            std::cout << '\'' << ret[i] << '\'';
        }
        std::cout << std::endl;
        */

        //std::cout << "num: " << num << " (" << std::hex << num << ")" << std::endl;
        char c = num & 0xFF;
        ret[MSGSIZEBYTECOUNT-i-1] = c;
        num >>= 8;
    }
    return ret;
}

}