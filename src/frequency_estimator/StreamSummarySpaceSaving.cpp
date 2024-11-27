#include "StreamSummarySpaceSaving.hpp"

StreamSummarySpaceSaving::StreamSummarySpaceSaving(int M2, int K) : M2(M2), K(K) {
    ss = new StreamSummary(M2);
    ss->clear();
}

StreamSummarySpaceSaving::StreamSummarySpaceSaving(SpaceSavingConfig &config) : M2(config.K) {
    ss = new StreamSummary(M2);
    ss->clear();
}

void StreamSummarySpaceSaving::update(const std::string &x, int c) {
    _assert_not_implemented(c);
    this->total++;
    bool mon = false;
    int p = ss->find(x);
    if (p) mon = true;
    if (!mon) {
        int q;
        if (ss->tot < M2)
            q = 1;
        else
            q = ss->getmin() + 1;
        int i = ss->getid();
        ss->add2(ss->location(x), i);
        ss->str[i] = x;
        ss->sum[i] = q;
        ss->link(i, 0);
        while (ss->tot > M2) {
            int t = ss->Right[0];
            int tmp = ss->head[t];
            ss->cut(ss->head[t]);
            ss->recycling(tmp);
        }
    } else {
        int tmp = ss->Left[ss->sum[p]];
        ss->cut(p);
        if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
        ss->sum[p]++;
        ss->link(p, tmp);
    }
}

void StreamSummarySpaceSaving::update(const int &x, int c) {
    _assert_not_implemented(c);
    update(std::to_string(x));
}

unsigned int StreamSummarySpaceSaving::estimate(const std::string &item) {
    int p = ss->find(item);
    if (p) return ss->sum[p];
    return 0;
}

unsigned int StreamSummarySpaceSaving::estimate(const int &item) {
    return estimate(std::to_string(item));
}

unsigned int StreamSummarySpaceSaving::update_and_estimate(const std::string &item, int c) {
    update(item, c);
    return estimate(item);
}

unsigned int StreamSummarySpaceSaving::update_and_estimate(const int &item, int c) {
    return update_and_estimate(std::to_string(item), c);
}

void StreamSummarySpaceSaving::_assert_not_implemented(int c) {
    if (c != 1) {
        throw std::runtime_error("Not implemented for cases where the number is not equal to 1");
    }
}

void StreamSummarySpaceSaving::print_status() {
    std::cout << "StreamSummarySpaceSaving: " << std::endl;
    std::cout << "M2: " << M2 << std::endl;
    std::cout << "K: " << K << std::endl;
    std::cout << "Estimated size in bytes: " << sizeof(*this) << std::endl;
}