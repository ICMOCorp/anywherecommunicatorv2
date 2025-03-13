#include <string>
#include <iostream>

namespace testing{
/*simply a wrapper for writing out "passed" or "failed"
so we can copy and paste this function*/
inline void printCase(std::string nameOfCase, bool passed){
    if(passed){
        std::cout << "  :^) Passed: ";
    }
    else{
        std::cout << " >:^( Failed: ";
    }
    std::cout << nameOfCase << std::endl;
}

/*we can't verify things are working with sockets
we just want to make sure that it looks right when
printed
so we ask the user (the tester) to manually type out yes or no
if things are working or not*/
inline bool testViaUserInput(std::string msg){
    std::string ans = "";

    std::cout << "\t!!!!HEY USER!!!!\n" << msg << " (yes or no) ";
    std::cin >> ans;
    while(ans != "yes" && ans != "no"){
        std::cout << "\t>>Must be yes or no<<" << std::endl;
        std::cout << msg << " (yes or no) ";
        std::cin >> ans;
    }
    return ans == "yes";
}

/*A wrapper for our original method. There was so much repetition
that I wanted to shrink like 4 lines of repeated code to "test"

I think this is as general as I can make it*/
class TestSuite{
private:
    int passed, total;

public:
    inline TestSuite(){};

    inline TestSuite(std::string name, std::string filename){
        std::cout << "\n=====================================================" << std::endl;
        std::cout << "Testing " << name <<  " in " << filename << std::endl;
        std::cout << "=====================================================" << std::endl;
        passed = 0;
        total = 0;
    }
    inline ~TestSuite(){
        std::cout << "=====================================================" << std::endl;
    }
    inline TestSuite(const TestSuite& other){
        this->passed = other.passed;
        this->total = other.total;
    }
    inline friend void swap(TestSuite& a, TestSuite& b){
        std::swap(a.passed, b.passed);
        std::swap(a.total, b.total);
    }
    inline TestSuite& operator=(TestSuite other){
        swap(*this, other);
        return *this;
    }


    inline void test(std::string name, bool val){
        printCase(name, val);
        if(val){
            passed++;
        }
        total++;
    }

    inline void printFinalOutput(){
        std::cout << ">>>Passed: " << passed << "/" << total << std::endl;
    }
};

}