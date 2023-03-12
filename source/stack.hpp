#pragma once

#include <assert.h>

template <typename T>
class Stack {
public:
    Stack() : top(nullptr) {}
    ~Stack() { Clear(); }

    void Push(const T &s) {
        Node *n = new Node();
        n->data = s;
        n->next = top;
        top = n;
    }

    T Pop() {
        assert(top != nullptr);
        Node *tmp = top;
        T r = top->data;
        top = top->next;
        delete tmp;
        return r;
    }

    T &Peek() {
        assert(top != nullptr);
        return top->data;
    }

    bool IsEmpty() {
        return top == nullptr;
    }

    void Clear() {
        while (!IsEmpty())
            Pop();
    }

private:
    struct Node {
        T data;
        Node *next;
    };
    Node *top;
};
