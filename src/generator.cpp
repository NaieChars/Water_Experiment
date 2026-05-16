#include "../include/generator.h"

static unsigned int seed = 12345;

// 线性同余随机数生成器
unsigned int lcg()
{
    seed = 1664525 * seed + 1013904223;
    return seed;
}

// 生成随机数据
std::vector<unsigned int> generate_random(int n)
{
    std::vector<unsigned int> data;

    for(int i = 0; i < n; i++)
    {
        data.push_back(lcg());
    }

    return data;
}

// 生成等差数列
std::vector<unsigned int> generate_arithmetic(int n, int d)
{
    std::vector<unsigned int> data;

    for(int i = 0; i < n; i++)
    {
        data.push_back(i * d);
    }

    return data;
}