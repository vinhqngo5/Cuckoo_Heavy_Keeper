// only include this file in the cpp file to avoid poisoning the macro
// macro scope is this compilation unit
#define COUNTER_PER_BUCKET 8
#define MAX_VALID_COUNTER  7

#define ALIGNMENT 64

#define COUNTER_PER_WORD         8
#define BIT_TO_DETERMINE_COUNTER 3
#define K_HASH_WORD              1

#define KEY_LENGTH_4  4
#define KEY_LENGTH_13 13

#define CONSTANT_NUMBER        2654435761u
#define CalculateBucketPos(fp) (((fp) * CONSTANT_NUMBER) >> 15)

#define GetCounterVal(val) ((uint32_t) ((val) & 0x7FFFFFFF))

#define JUDGE_IF_SWAP(min_val, guard_val) ((guard_val) > ((min_val) << 3))

#define UPDATE_GUARD_VAL(guard_val) ((guard_val) + 1)

#define SWAP_MIN_VAL_THRESHOLD 5

#define HIGHEST_BIT_IS_1(val) ((val) & 0x80000000)
