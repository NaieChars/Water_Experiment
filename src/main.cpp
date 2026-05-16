#include "../include/generator.h"
#include "../include/hash.h"
#include "../include/experiment.h"

#include <fstream>

int main()
{
    // 清空旧结果文件
    std::ofstream clear_file("../results/result.txt");
    clear_file.close();

    // 数据集
    auto random_data = generate_random(500);

    auto arithmetic_data = generate_arithmetic(500, 16);

    // 随机数据实验

    run_experiment(
        random_data,
        hash_mod,
        1009,
        "Random Data + Mod Hash + m = 1009"
    );

    run_experiment(
        random_data,
        hash_mod,
        1024,
        "Random Data + Mod Hash + m = 1024"
    );

    run_experiment(
        random_data,
        hash_multiply,
        1024,
        "Random Data + Multiply Hash + m = 1024"
    );

    // 等差数列实验

    run_experiment(
        arithmetic_data,
        hash_mod,
        1009,
        "Arithmetic Data + Mod Hash + m = 1009"
    );

    run_experiment(
        arithmetic_data,
        hash_mod,
        1024,
        "Arithmetic Data + Mod Hash + m = 1024"
    );

    run_experiment(
        arithmetic_data,
        hash_multiply,
        1024,
        "Arithmetic Data + Multiply Hash + m = 1024"
    );

    return 0;
}