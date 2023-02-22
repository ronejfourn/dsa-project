#pragma once

template <typename T>
class Stack {
public:
    Stack() : top(nullptr) {}
    ~Stack() { Clear(); }

    void Push(const T &s) {
        Node *n = new Node();
        n->state = s;
        n->next = top;
        top = n;
    }

    T Pop() {
        Node *tmp = top;
        T r = top->state;
        top = top->next;
        delete tmp;
        return r;
    }

    T Peek() {
        return top->state;
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
        T state;
        Node *next;
    };
    Node *top;
};
