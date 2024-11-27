#include "HeavyKeeper.hpp"

HeavyKeeper::HeavyKeeper(int M2, int K) : M2(M2), K(K) {
    ss = new StreamSummary(K);
    ss->clear();

    // generate random number between 0 and 1228
    srand((unsigned int) clock());
    int random_seed = rand() % 1228;
    bobhash = new BOBHash64(random_seed);
}

HeavyKeeper::HeavyKeeper(int M2) : M2(M2) {
    // generate random number between 0 and 1228
    srand((unsigned int) clock());
    int random_seed = rand() % 1228;
    bobhash = new BOBHash64(random_seed);
}

HeavyKeeper::HeavyKeeper(HeavyKeeperConfig &config) : M2(config.M2), K(config.K) {
    ss = new StreamSummary(K);
    ss->clear();

    // generate random number between 0 and 1228
    srand((unsigned int) clock());
    int random_seed = rand() % 1228;
    bobhash = new BOBHash64(random_seed);
}

void HeavyKeeper::generate_new_seed() {
    // generate random number between 0 and 1228
    srand((unsigned int) clock());
    int random_seed = rand() % 1228;
    bobhash = new BOBHash64(random_seed);
}

void HeavyKeeper::clear() {
    for (int i = 0; i < HK_d; i++)
        for (int j = 0; j <= M2 + 5; j++) HK[i][j].C = HK[i][j].FP = 0;
}

unsigned long long HeavyKeeper::hash(const std::string &ST) { return (bobhash->run(ST.c_str(), ST.size())); }

void HeavyKeeper::_insert_with_StreamSummary(const std::string &x) {
    this->total += 1;
    bool mon = false;
    int p = ss->find(x);
    if (p) mon = true;
    int maxv = 0;
    unsigned long long H = hash(x);
    int FP = (H >> 48);
    for (int j = 0; j < HK_d; j++) {
        int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);
        int c = HK[j][Hsh].C;
        // if item is already in the bucket
        if (HK[j][Hsh].FP == FP) {
            if (mon || c <= ss->getmin()) HK[j][Hsh].C++;
            maxv = std::max(maxv, HK[j][Hsh].C);
        }
        // if item is not in the bucket -> decay with prob
        else {
            if (!(rand() % int(pow(HK_b, HK[j][Hsh].C)))) {
                HK[j][Hsh].C--;
                if (HK[j][Hsh].C <= 0) {
                    HK[j][Hsh].FP = FP;
                    HK[j][Hsh].C = 1;
                    maxv = std::max(maxv, 1);
                }
            }
        }
    }
    if (!mon) {
        if (maxv - (ss->getmin()) == 1 || ss->tot < K) {
            int i = ss->getid();
            ss->add2(ss->location(x), i);
            ss->str[i] = x;
            ss->sum[i] = maxv;
            ss->link(i, 0);
            while (ss->tot > K) {
                int t = ss->Right[0];
                int tmp = ss->head[t];
                ss->cut(ss->head[t]);
                ss->recycling(tmp);
            }
        }
    } else if (maxv > ss->sum[p]) {
        int tmp = ss->Left[ss->sum[p]];
        ss->cut(p);
        if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
        ss->sum[p] = maxv;
        ss->link(p, tmp);
    }
}

void HeavyKeeper::_insert(const std::string &x) {
    this->total += 1;
    unsigned long long H = hash(x);
    int FP = (H >> 48);
    for (int j = 0; j < HK_d; j++) {
        int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);
        int c = HK[j][Hsh].C;
        if (HK[j][Hsh].FP == FP) {
            HK[j][Hsh].C++;
        } else {
            if (!(rand() % int(pow(HK_b, HK[j][Hsh].C)))) {
                HK[j][Hsh].C--;
                if (HK[j][Hsh].C <= 0) {
                    HK[j][Hsh].FP = FP;
                    HK[j][Hsh].C = 1;
                }
            }
        }
    }
}

void HeavyKeeper::work() {
    int CNT = 0;
    for (int i = N; i; i = ss->Left[i])
        for (int j = ss->head[i]; j; j = ss->Next[j]) {
            q[CNT].x = ss->str[j];
            q[CNT].y = ss->sum[j];
            CNT++;
        }
    std::sort(q, q + CNT, cmp);
}

std::pair<std::string, int> HeavyKeeper::query(const int &k) { return std::make_pair(q[k].x, q[k].y); }

void HeavyKeeper::print_status() {
    std::cout << "Width: " << this->M2 << std::endl;
    std::cout << "Depth: " << HK_d << std::endl;
    std::cout << "Estimated size in bytes: " << this->M2 * sizeof(node) * HK_d << std::endl;
}

void HeavyKeeper::update(const std::string &item, int c) {
    this->_assert_not_implemented(c);
    this->_insert(item.c_str());
}

void HeavyKeeper::update(const int &item, int c) {
    this->_assert_not_implemented(c);
    this->_insert(std::to_string(item));
}

unsigned int HeavyKeeper::_estimate(const std::string &item) {
    unsigned long long H = hash(item);
    int FP = (H >> 48);
    int maxv = 0;
    for (int j = 0; j < HK_d; j++) {
        int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);
        int c = HK[j][Hsh].C;
        if (HK[j][Hsh].FP == FP) { maxv = std::max(maxv, HK[j][Hsh].C); }
    }
    return maxv;
}

unsigned int HeavyKeeper::estimate(const std::string &item) { return this->_estimate(item); }

unsigned int HeavyKeeper::estimate(const int &item) { return this->_estimate(std::to_string(item)); }

unsigned int HeavyKeeper::update_and_estimate(const std::string &item, int c) {
    this->_assert_not_implemented(c);
    this->total += 1;
    unsigned long long H = hash(item);
    int FP = (H >> 48);
    int maxv = 0;
    for (int j = 0; j < HK_d; j++) {
        int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);
        int c = HK[j][Hsh].C;
        if (HK[j][Hsh].FP == FP) {
            HK[j][Hsh].C++;
            maxv = std::max(maxv, HK[j][Hsh].C);
        } else {
            if (!(rand() % int(pow(HK_b, HK[j][Hsh].C)))) {
                HK[j][Hsh].C--;
                if (HK[j][Hsh].C <= 0) {
                    HK[j][Hsh].FP = FP;
                    HK[j][Hsh].C = 1;
                    maxv = std::max(maxv, 1);
                }
            }
        }
    }
    return maxv;
}

unsigned int HeavyKeeper::update_and_estimate(const int &item, int c) {
    this->_assert_not_implemented(c);
    this->total += 1;
    unsigned long long H = hash(std::to_string(item));
    int FP = (H >> 48);
    int maxv = 0;
    for (int j = 0; j < HK_d; j++) {
        int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);
        int c = HK[j][Hsh].C;
        if (HK[j][Hsh].FP == FP) {
            HK[j][Hsh].C++;
            maxv = std::max(maxv, HK[j][Hsh].C);
        } else {
            if (!(rand() % int(pow(HK_b, HK[j][Hsh].C)))) {
                HK[j][Hsh].C--;
                if (HK[j][Hsh].C <= 0) {
                    HK[j][Hsh].FP = FP;
                    HK[j][Hsh].C = 1;
                    maxv = std::max(maxv, 1);
                }
            }
        }
    }
    return maxv;
}

void HeavyKeeper::_assert_not_implemented(int c) {
    if (c != 1) { throw std::runtime_error("Not implemented for cases where the number is not equal to 1"); }
}
