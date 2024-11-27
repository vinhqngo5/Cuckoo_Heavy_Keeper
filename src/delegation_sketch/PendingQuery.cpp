#include "PendingQuery.hpp"

// PendingQuery implementation
PendingQuery::PendingQuery() : key(0), count(0), flag(false) {}

PendingQuery::PendingQuery(PendingQuery &&other) : key(other.key), count(other.count), flag(other.flag) {}

void PendingQuery::add_query(int key) {
    this->key = key;
    this->flag = true;
}
