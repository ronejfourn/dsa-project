#pragma once

template <typename T>
class stack_t {
public:
    stack_t() : top(nullptr) {}

    void push(const T &s) {
        node_t *n = new node_t();
        n->state = s;
        n->next = top;
        top = n;
    }

    T pop() {
        node_t *tmp = top;
        T r = top->state;
        top = top->next;
        delete tmp;
        return r;
    }

    T peek() {
        return top->state;
    }

    bool is_empty() {
        return top == nullptr;
    }

    void clear() {
        while (!is_empty())
            pop();
    }

private:
    struct node_t {
        T state;
        node_t *next;
    };
    node_t *top;
};
