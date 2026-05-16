#include "../include/stats.h"

double compute_variance(const std::vector<int>& buckets, int m, int n)
{
    double alpha = (double)n / m;

    double s2 = 0.0;

    for(int x : buckets)
    {
        s2 += (x - alpha) * (x - alpha);
    }

    return s2 / m;
}

int max_bucket_length(const std::vector<int>& buckets)
{
    int mx = 0;

    for(int x : buckets)
    {
        if(x > mx)
        {
            mx = x;
        }
    }

    return mx;
}