#pragma once
#include <vector>
#include <string>

namespace ringbuffer{
    enum{
        OUTOFBOUNDS                     = -10
    };

    /*Some error function to ensure something doesn't
    go wrong*/
    void bufferError(std::string msg);

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
    
    THUS, MAKE SURE YOU USE DOUBLE THE AMOUNT OF A SINGLE MESSAGE
        This is to account for the issue of lapping

    lapping (end beats start) is problematic and can lead to many undesired consequences.
    This can be avoided by setting a larger maxLength. For example, 
    make the size of the buffer double that of a single message.


    Ex: 
    MSG: "This was a triumph"
    Buffer:
        T->h->i->s-> ->w->a->s-> ->a-> ->t->r->i->u->m->p->h->
        ^                                                     ^ 
       start                                                 end
    */
    class RingBufferS{
        private:
            std::vector<char> data; // the data of our ring buffer
            size_t maxLength;   // the fixed length of data
            size_t start, end;  // the two pointers of our ring
            size_t currSize;

        
        public:
            /* initializes internal data to be empty
            */
            RingBufferS();

            /* initializes internal data to a set size
                and must be > 0*/
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


            /*pushes characters toAdd to data
            if what we add exceeds the size of the maxlength,
                It wil RETURN an OUTOFBOUNDS error

            Q: is there a way to check for valid toAdd?

            if all is properly done, returns 1
            */
            int push(const std::string& toAdd, size_t size);

            /*pops characters in our data to destination
            if there are too little to pop but the request 
                is too much:
                - Then pop as much as possible and return 
                    OUTOFBOUNDS error
            if there is nothing else to pop and popAmount> 0
                - then (actually this is defined above)

            Q: is there a way to check for valid dest?

            if all is properly done, returns 1
            */
            int pop(std::string& dest, size_t popAmount);

            /*returns a string of the contents of whats currently in the buffer*/
            std::string getContents();

            /*returns whether or not the buffer is empty or not*/
            bool isEmpty();

            /*simple accessor to the size variable*/
            size_t size();
    };
}