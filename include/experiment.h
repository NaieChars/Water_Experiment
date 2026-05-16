#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>
#include <string>

void run_experiment(
    const std::vector<unsigned int>& data,
    int (*hash_func)(unsigned int, int),
    int m,
    const std::string& experiment_name
);

#endif