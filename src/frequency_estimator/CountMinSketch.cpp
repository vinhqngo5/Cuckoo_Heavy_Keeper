#include "CountMinSketch.hpp"

#define LONG_PRIME 2147483647

CountMinSketch::CountMinSketch() {}

CountMinSketch::CountMinSketch(unsigned int width, unsigned int depth) {
    this->depth = depth;
    this->width = width;
    epsilon = exp(1) / width;
    delta = 1 / pow(exp(1), depth);
    this->init_countmin_sketch();
}

CountMinSketch::CountMinSketch(float epsilon, float delta) {
    this->epsilon = epsilon;
    this->delta = delta;
    this->width = ceil(exp(1) / epsilon);
    this->depth = ceil(log(1 / delta));
    this->init_countmin_sketch();
}

CountMinSketch::CountMinSketch(const CountMinConfig &countmin_configs) {
    if (countmin_configs.CALCULATE_FROM == "WIDTH_DEPTH") {
        this->width = countmin_configs.WIDTH;
        this->depth = countmin_configs.DEPTH;
        this->epsilon = exp(1) / this->width;
        this->delta = 1 / pow(exp(1), this->depth);
    } else if (countmin_configs.CALCULATE_FROM == "EPSILON_DELTA") {
        this->epsilon = countmin_configs.EPSILON;
        this->delta = countmin_configs.DELTA;
        this->width = ceil(exp(1) / this->epsilon);
        this->depth = ceil(log(1 / delta));
    } else {
        cout << "Init CountMinSketch: Invalid calculate from" << endl;
        exit(1);
    }
    this->init_countmin_sketch();
}

void CountMinSketch::init_countmin_sketch() {
    // Initialize counter array of arrays, C
    this->C = new int *[this->depth];
    for (int i = 0; i < this->depth; i++) {
        this->C[i] = new int[this->width];
        for (int j = 0; j < this->width; j++) { this->C[i][j] = 0; }
    }

    // Initialize d pairwise independent hashes
    srand((unsigned int) clock());
    this->hashes = new int *[this->depth];
    for (int i = 0; i < this->depth; i++) {
        this->hashes[i] = new int[2];
        genajbj(this->hashes, i);
    }
}

void CountMinSketch::genajbj(int **hashes, int i) {
    hashes[i][0] = int(float(rand()) * float(LONG_PRIME) / float(RAND_MAX) + 1);
    hashes[i][1] = int(float(rand()) * float(LONG_PRIME) / float(RAND_MAX) + 1);
}

unsigned int CountMinSketch::_get_hashitem(const string &item) {
    unsigned long hash = 5381;
    for (char c : item) { hash = (((hash << 5) + hash) + c) % LONG_PRIME; /* hash * 33 + c */ }
    return hash;
}

void CountMinSketch::update(const int &item, int c) { this->_update(item, c); }

void CountMinSketch::update(const string &item, int c) {
    unsigned int hashitem = this->_get_hashitem(item);
    this->_update(hashitem, c);
}

void CountMinSketch::_update(const int &item, int c) {
    this->total = this->total + c;
    unsigned int hashval = 0;
    for (int i = 0; i < this->depth; i++) {
        hashval = ((long) hashes[i][0] * item + hashes[i][1]) % LONG_PRIME % this->width;
        this->C[i][hashval] = this->C[i][hashval] + c;
    }
}

unsigned int CountMinSketch::estimate(const int &item) { return this->_estimate(item); }

unsigned int CountMinSketch::estimate(const string &item) {
    unsigned int hashitem = this->_get_hashitem(item);
    return this->_estimate(hashitem);
}

unsigned int CountMinSketch::_estimate(const int &item) {
    int minval = numeric_limits<int>::max();
    unsigned int hashval = 0;
    for (unsigned int j = 0; j < this->depth; j++) {
        hashval = ((long) hashes[j][0] * item + hashes[j][1]) % LONG_PRIME % this->width;
        minval = min(minval, C[j][hashval]);
    }
    return minval;
}

unsigned int CountMinSketch::update_and_estimate(const int &item, int c) {
    int minval = numeric_limits<int>::max();
    this->total = this->total + c;
    unsigned int hashval = 0;
    for (int i = 0; i < this->depth; i++) {
        hashval = ((long) hashes[i][0] * item + hashes[i][1]) % LONG_PRIME % this->width;
        this->C[i][hashval] = this->C[i][hashval] + c;
        minval = min(minval, C[i][hashval]);
    }
    return minval;
}

unsigned int CountMinSketch::update_and_estimate(const string &item, int c) {
    int minval = numeric_limits<int>::max();
    unsigned int hashitem = this->_get_hashitem(item);
    this->total = this->total + c;
    unsigned int hashval = 0;
    for (int i = 0; i < this->depth; i++) {
        hashval = ((long) hashes[i][0] * hashitem + hashes[i][1]) % LONG_PRIME % this->width;
        this->C[i][hashval] = this->C[i][hashval] + c;
        minval = min(minval, C[i][hashval]);
    }
}

void CountMinSketch::print_status() {
    cout << "Width: " << this->width << endl;
    cout << "Depth: " << this->depth << endl;
    cout << "Epsilon: " << this->epsilon << endl;
    cout << "Delta: " << this->delta << endl;
    cout << "Total: " << this->total << endl;
    cout << "Estimated size in bytes: " << this->width * this->depth * sizeof(int) << endl;
}