#include "ConvolutionalCode.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

using namespace std;

ConvolutionalCode::ConvolutionalCode(const ConvolutionalCodeConfig& config)
    : config_(config),
      state_mask_(0),
      register_mask_(0)
{
    checkConfig();

    state_mask_ = (1 << (config_.constraint_length - 1)) - 1;
    register_mask_ = (1 << config_.constraint_length) - 1;
}

int ConvolutionalCode::constraintLength() const
{
    return config_.constraint_length;
}

int ConvolutionalCode::outputBitCount() const
{
    return int(config_.generators.size());
}

int ConvolutionalCode::stateCount() const
{
    return 1 << (config_.constraint_length - 1);
}

int ConvolutionalCode::nextState(int state, int input_bit) const
{
    const int reg = ((state << 1) | input_bit) & register_mask_;
    return reg & state_mask_;
}

vector<int> ConvolutionalCode::outputBits(int state, int input_bit) const
{
    const unsigned int reg =
        static_cast<unsigned int>(((state << 1) | input_bit) & register_mask_);

    vector<int> bits;
    bits.reserve(config_.generators.size());

    for (unsigned int generator : config_.generators) {
        bits.push_back(parity(reg & generator));
    }

    return bits;
}

int ConvolutionalCode::parity(unsigned int value) const
{
    int result = 0;
    while (value != 0) {
        result ^= static_cast<int>(value & 1U);
        value >>= 1U;
    }

    return result;
}

void ConvolutionalCode::checkConfig() const
{
    if (config_.constraint_length < 2 || config_.constraint_length > 20) {
        throw invalid_argument("bad constraint length");
    }

    if (config_.generators.empty()) {
        throw invalid_argument("generator list is empty");
    }

    const unsigned int max_mask = (1U << config_.constraint_length) - 1U;
    for (unsigned int generator : config_.generators) {
        if (generator == 0 || (generator & ~max_mask) != 0) {
            throw invalid_argument("generator does not fit constraint length");
        }
    }
}

ConvolutionalEncoder::ConvolutionalEncoder(const ConvolutionalCodeConfig& config)
    : code_(config)
{
}

vector<int> ConvolutionalEncoder::encode(
    const vector<int>& input_bits,
    bool terminate) const
{
    vector<int> bits = input_bits;
    if (terminate) {
        bits.insert(bits.end(), code_.constraintLength() - 1, 0);
    }

    int state = 0;
    vector<int> encoded;
    encoded.reserve(bits.size() * code_.outputBitCount());

    for (int bit : bits) {
        if (bit != 0 && bit != 1) {
            throw invalid_argument("input bits must be 0 or 1");
        }

        vector<int> out = code_.outputBits(state, bit);
        encoded.insert(encoded.end(), out.begin(), out.end());
        state = code_.nextState(state, bit);
    }

    return encoded;
}

ViterbiDecoder::ViterbiDecoder(const ConvolutionalCodeConfig& config)
    : code_(config)
{
    const int states = code_.stateCount();

    next_state_.assign(states, vector<int>(2));
    output_bits_.assign(states, vector<vector<int>>(2));

    for (int state = 0; state < states; ++state) {
        for (int bit = 0; bit <= 1; ++bit) {
            next_state_[state][bit] = code_.nextState(state, bit);
            output_bits_[state][bit] = code_.outputBits(state, bit);
        }
    }
}

vector<int> ViterbiDecoder::decode(
    const vector<int>& received_bits,
    size_t message_bit_count,
    bool terminated) const
{
    const int outputs = code_.outputBitCount();
    if (received_bits.size() % outputs != 0) {
        throw invalid_argument("bad received bit count");
    }

    const size_t steps = received_bits.size() / outputs;
    if (message_bit_count > steps) {
        throw invalid_argument("message bit count is too large");
    }

    const int states = code_.stateCount();
    const int inf = numeric_limits<int>::max() / 4;

    vector<int> metric(states, inf);
    vector<int> next_metric(states, inf);
    metric[0] = 0;

    vector<vector<Survivor>> survivor(
        steps,
        vector<Survivor>(states, {-1, 0}));

    for (size_t step = 0; step < steps; ++step) {
        fill(next_metric.begin(), next_metric.end(), inf);

        for (int state = 0; state < states; ++state) {
            if (metric[state] == inf) {
                continue;
            }

            for (int bit = 0; bit <= 1; ++bit) {
                const int to_state = next_state_[state][bit];
                const vector<int>& expected = output_bits_[state][bit];
                const int candidate =
                    metric[state] +
                    branchMetric(received_bits, step, expected);

                if (candidate < next_metric[to_state]) {
                    next_metric[to_state] = candidate;
                    survivor[step][to_state] = {state, bit};
                }
            }
        }

        metric.swap(next_metric);
    }

    int state = 0;
    if (!terminated) {
        state = static_cast<int>(
            min_element(metric.begin(), metric.end()) - metric.begin());
    }

    vector<int> decoded(steps, 0);
    for (size_t step = steps; step > 0; --step) {
        const Survivor item = survivor[step - 1][state];
        decoded[step - 1] = item.input_bit;
        state = item.previous_state;
    }

    decoded.resize(message_bit_count);
    return decoded;
}

int ViterbiDecoder::branchMetric(
    const vector<int>& received_bits,
    size_t step,
    const vector<int>& expected_bits) const
{
    int distance = 0;
    const size_t offset = step * expected_bits.size();

    for (size_t i = 0; i < expected_bits.size(); ++i) {
        if (received_bits[offset + i] != expected_bits[i]) {
            ++distance;
        }
    }

    return distance;
}
