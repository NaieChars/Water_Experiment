#include "../include/experiment.h"
#include "../include/stats.h"

#include <iostream>
#include <fstream>
#include <iomanip>

void run_experiment(
    const std::vector<unsigned int>& data,
    int (*hash_func)(unsigned int, int),
    int m,
    const std::string& experiment_name
)
{
    std::vector<int> buckets(m, 0);

    // 哈希映射
    for(unsigned int key : data)
    {
        int idx = hash_func(key, m);

        buckets[idx]++;
    }

    // 计算指标
    double s2 = compute_variance(buckets, m, data.size());

    int max_len = max_bucket_length(buckets);

    // 输出到终端
    std::cout << "==============================" << std::endl;
    std::cout << experiment_name << std::endl;
    std::cout << "m = " << m << std::endl;
    std::cout << "S^2 = " << s2 << std::endl;
    std::cout << "Max Bucket Length = " << max_len << std::endl;

    // 输出到文件
    std::ofstream fout("../results/result.txt", std::ios::app);

    fout << "==============================" << std::endl;
    fout << experiment_name << std::endl;
    fout << "m = " << m << std::endl;
    fout << std::fixed << std::setprecision(6);
    fout << "S^2 = " << s2 << std::endl;
    fout << "Max Bucket Length = " << max_len << std::endl;
    fout << std::endl;

    fout.close();
}