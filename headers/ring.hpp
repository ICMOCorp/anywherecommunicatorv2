#include <vector>
/*
basic implementation of a circular buffer of characters
    - behaves like a queue (FIFO)
a simple array of data with two indices that show the bounds of the loop
    - start
    - end
Note that start and end behave independently. That is, even if end loops
around and laps the start, it doesn't change the implementation of the
code. Start and end really are there for programmer's convenience,
push only looks at end and pop only looks at start

lapping (end beats start) is problematic and can lead to many undesired consequences.
This can be avoided by setting a larger maxLength. For example, 
make the size of the buffer double that of a single message.
*/
class RingBufferS{
    private:
        std::vector<char> data; // the data of our ring buffer
        size_t maxLength;   // the fixed length of data
        size_t start, end;  // the two pointers of our ring
    
    public:
        /* initializes internal data to a set size*/
        RingBufferS(size_t maxLength);

        /*Copy-and-swap Idiom
            -> https://medium.com/@amalpp42/idioms-in-c-f6b1c19fa605
        */
        RingBufferS(const RingBufferS& other);
        RingBufferS& operator=(RingBufferS other){
            swap(*this, other);
            return *this;
        }
        friend void swap(RingBufferS& first, RingBufferS& second){
            using std::swap;
            swap(first.data, second.data);
            swap(first.maxLength, second.maxLength);
            swap(first.start, second.start);
            swap(first.end, second.end);
        }


        /*pushes characters toAdd to data*/
        void push(const char* toAdd, size_t size);
        /*pushes characters in our data to destination*/
        void pop(char* dest, size_t popAmount);
};