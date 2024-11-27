
#include "HeavyGuardian.hpp"

unsigned long long HeavyGuardian::hash(std::string ST) {
    return (bobhash->run(ST.c_str(), ST.size()));
}

void HeavyGuardian::_insert(const std::string &item) {
    this->total += 1;
    unsigned long long H = hash(item);
    unsigned int FP = (H >> 48), Hsh = H % M;
    bool FLAG = false;
    for (int k = 0; k < G; k++) {
        int c = HK[Hsh][k].C;
        if (HK[Hsh][k].FP == FP) {
            HK[Hsh][k].C++;
            FLAG = true;
            break;
        }
        if (FLAG) break;
    }
    if (!FLAG) {
        int X, MIN = 1000000000;
        for (int k = 0; k < G; k++) {
            int c = HK[Hsh][k].C;
            if (c < MIN) {
                MIN = c;
                X = k;
            }
        }
        if (!(rand() % int(pow(HK_b, HK[Hsh][X].C)))) {
            HK[Hsh][X].C--;
            if (HK[Hsh][X].C <= 0) {
                HK[Hsh][X].FP = FP;
                HK[Hsh][X].C = 1;
            } else {
                int p = Hsh % ct;
                ext[Hsh][p]++;
            }
        }
    }
}

void HeavyGuardian::update(const std::string &item, int c) {
    this->_assert_not_implemented(c);
    this->_insert(item.c_str());
}

void HeavyGuardian::update(const int &item, int c) {
    this->_assert_not_implemented(c);
    char buffer[20];
    sprintf(buffer, "%d", item);
    this->_insert(buffer);
}

unsigned int HeavyGuardian::_estimate(const std::string &item) {
    unsigned long long H = hash(item);
    unsigned int FP = (H >> 48), Hsh = H % M;
    for (int k = 0; k < G; k++) {
        int c = HK[Hsh][k].C;
        if (HK[Hsh][k].FP == FP) return max(1, HK[Hsh][k].C);
    }
    int p = Hsh % ct;
    return max(1, ext[Hsh][p]);
}

unsigned int HeavyGuardian::estimate(const std::string &item) { return this->_estimate(item); }

unsigned int HeavyGuardian::estimate(const int &item) {
    char buffer[20];
    sprintf(buffer, "%d", item);
    std::string str(buffer);
    return this->_estimate(str);
}

unsigned int HeavyGuardian::update_and_estimate(const std::string &item, int c) {
    this->_assert_not_implemented(c);
    this->_insert(item);
    return this->_estimate(item);
}

unsigned int HeavyGuardian::update_and_estimate(const int &item, int c) {
    this->_assert_not_implemented(c);
    char buffer[20];
    sprintf(buffer, "%d", item);
    std::string str(buffer);
    this->_insert(str);
    return this->_estimate(str);
}

void HeavyGuardian::_assert_not_implemented(int c) {
    if (c != 1) {
        throw std::runtime_error("Not implemented for cases where the number is not equal to 1");
    }
}

void HeavyGuardian::print_status() {
    std::cout << "Width: " << this->M << std::endl;
    std::cout << "Depth: " << G << std::endl;
    std::cout << "Estimated size in bytes: "
              << this->M * sizeof(node) * G + this->M * sizeof(int) * ct << std::endl;
}
