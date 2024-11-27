#pragma once

// Pending Query
struct PendingQuery {
    int key;
    volatile int count;
    volatile bool flag;

    PendingQuery();
    PendingQuery(PendingQuery &&other);
    void add_query(int key);
};