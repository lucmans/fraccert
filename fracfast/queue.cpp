
#include "queue.h"


Queue::Queue() {
    head = nullptr;
    tail = nullptr;
}

Queue::Queue(unsigned int i) {
    head = new elem(i);

    tail = head;
}

Queue::~Queue() {
    elem* iter;

    elem* save = nullptr;
    for(iter = head; iter != tail; iter = save) {
        save = iter->prev;

        delete iter;
    }

    delete iter;
}


bool Queue::empty() const {
    return head == nullptr;
}


void Queue::push(unsigned int i) {
    if(head == nullptr) {
        head = new elem(i);

        tail = head;
    }
    else {
        tail->prev = new elem(i);
        tail = tail->prev;
    }
}

void Queue::pop() {
    if(head == nullptr)
        return;

    elem* save = head;

    head = head->prev;
    if(head == nullptr)
        tail = nullptr;

    delete save;
}


unsigned int Queue::front() const {
    return head->value;
}
