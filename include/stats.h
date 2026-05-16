#ifndef STATS_H
#define STATS_H

#include <vector>

double compute_variance(const std::vector<int>& buckets, int m, int n);

int max_bucket_length(const std::vector<int>& buckets);

#endif