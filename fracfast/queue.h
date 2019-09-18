
#ifndef QUEUE_H
#define QUEUE_H


struct elem {
    unsigned int value;
    elem* prev;

    elem() {prev = nullptr;};
    elem(unsigned int v) {value = v; prev = nullptr;};
};


class Queue {
    public:
        Queue();
        Queue(unsigned int i);
        ~Queue();

        bool empty() const;
        
        void push(const unsigned int i);
        void pop();

        unsigned int front() const;

    private:
        elem* head;
        elem* tail;
};


#endif  // QUEUE_H