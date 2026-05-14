#pragma once

#include <random>
#include <vector>

class BinarySymmetricChannel
{
public:
    explicit BinarySymmetricChannel(unsigned int seed = 1);

    std::vector<int> transmit(const std::vector<int>& bits, double error_probability);

private:
    std::mt19937 generator_;
};
