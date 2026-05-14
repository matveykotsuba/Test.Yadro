#include "BinarySymmetricChannel.hpp"

#include <stdexcept>

using namespace std;

BinarySymmetricChannel::BinarySymmetricChannel(unsigned int seed)
    : generator_(seed)
{
}

vector<int> BinarySymmetricChannel::transmit(
    const vector<int>& bits,
    double error_probability)
{
    if (error_probability < 0.0 || error_probability > 1.0) {
        throw invalid_argument("error probability must be in [0, 1]");
    }

    bernoulli_distribution flip(error_probability);

    vector<int> output;
    output.reserve(bits.size());

    for (int bit : bits) {
        if (bit != 0 && bit != 1) {
            throw invalid_argument("bits must be 0 or 1");
        }
        output.push_back(bit ^ static_cast<int>(flip(generator_)));
    }

    return output;
}
