#ifndef _BOBHASH32_H
#define _BOBHASH32_H
#include <cstdint>
#include <stdio.h>
using namespace std;

typedef unsigned int uint;
typedef unsigned long long int uint64;

constexpr uint32_t MAX_PRIME32 = 1229;
constexpr uint32_t MAX_BIG_PRIME32 = 50;

class BOBHash32 {
  public:
    BOBHash32();
    ~BOBHash32();
    BOBHash32(uint prime32Num);
    void initialize(uint prime32Num);
    uint run(const char *str, uint len);

  private:
    uint prime32Num;
};

#endif   //_BOBHASH32_H
