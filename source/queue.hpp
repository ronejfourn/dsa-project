#pragma once

#include <assert.h>

template <typename T>
class Queue {
public:
    typedef bool (*CompareFunc)(const T &a, const T &b);

    Queue() : front(nullptr), rear(nullptr) {}
    ~Queue() { Clear(); }

    void Enqueue(const T &s) {
        Node *n = new Node(s, nullptr);
        (!front) ?
            rear = front = n :
            rear->next = n, rear = n;
    }

    T PriorityDequeue() {
        assert(front);
        assert(cmp);

        Node *trav = front->next;
        Node *prev = front;
        Node *dequeued = front;
        Node *prevOfDequeued = nullptr;

        while (trav != nullptr) {
            if (cmp(trav->data, dequeued->data)) {
                dequeued = trav;
                prevOfDequeued = prev;
            }
            prev = trav;
            trav = trav->next;
        }

        auto data = dequeued->data;

        if (front == rear) {
            front = rear = nullptr;
        } else if (dequeued == front) {
            front = front->next;
        } else if (dequeued == rear) {
            rear = prevOfDequeued;
            rear->next = nullptr;
        } else {
            prevOfDequeued->next = dequeued->next;
        }

        delete dequeued;
        return data;
    }

    T Dequeue() {
        assert(front);
        auto r = front->data;
        auto n = front->next;

        delete front;

        (!n) ?
            front = rear = nullptr :
            front = n;

        return r;
    }

    void SetCompareFunc(CompareFunc f) {
        cmp = f;
    }

    bool IsEmpty() {
        return front == nullptr;
    }

    void Clear() {
        cmp = nullptr;
        while (!IsEmpty())
            Dequeue();
    }

private:
    struct Node {
        T data;
        Node *next;

        Node(T d, Node *n) : data(d), next(n) {}
    };
    Node *front;
    Node *rear;
    CompareFunc cmp = nullptr;
};

