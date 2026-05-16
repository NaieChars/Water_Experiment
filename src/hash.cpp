#include "../include/hash.h"

// 除留余数法
int hash_mod(unsigned int key, int m)
{
    return key % m;
}

// 乘法散列法
int hash_multiply(unsigned int key, int m)
{
    const double A = 0.6180339887;

    double val = key * A;

    // 取小数部分
    val = val - (int)val;

    return (int)(m * val);
}