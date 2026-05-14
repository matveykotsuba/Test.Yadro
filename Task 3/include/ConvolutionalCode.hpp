#pragma once

#include <cstddef>
#include <vector>

struct ConvolutionalCodeConfig
{
    int constraint_length;
    std::vector<unsigned int> generators;
};

class ConvolutionalCode
{
public:
    explicit ConvolutionalCode(const ConvolutionalCodeConfig& config);

    int constraintLength() const;
    int outputBitCount() const;
    int stateCount() const;

    int nextState(int state, int input_bit) const;
    std::vector<int> outputBits(int state, int input_bit) const;

private:
    ConvolutionalCodeConfig config_;
    int state_mask_;
    int register_mask_;

    int parity(unsigned int value) const;
    void checkConfig() const;
};

class ConvolutionalEncoder
{
public:
    explicit ConvolutionalEncoder(const ConvolutionalCodeConfig& config);

    std::vector<int> encode(const std::vector<int>& input_bits, bool terminate = true) const;

private:
    ConvolutionalCode code_;
};

class ViterbiDecoder
{
public:
    explicit ViterbiDecoder(const ConvolutionalCodeConfig& config);

    std::vector<int> decode(
        const std::vector<int>& received_bits,
        std::size_t message_bit_count,
        bool terminated = true) const;

private:
    struct Survivor
    {
        int previous_state;
        int input_bit;
    };

    ConvolutionalCode code_;
    std::vector<std::vector<int>> next_state_;
    std::vector<std::vector<std::vector<int>>> output_bits_;

    int branchMetric(const std::vector<int>& received_bits,
                     std::size_t step,
                     const std::vector<int>& expected_bits) const;
};
