#include "BinarySymmetricChannel.hpp"
#include "ConvolutionalCode.hpp"

#include <climits>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

vector<int> makeRandomBits(size_t count, mt19937& generator)
{
    uniform_int_distribution<int> bit(0, 1);

    vector<int> bits;
    bits.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        bits.push_back(bit(generator));
    }

    return bits;
}

size_t countBitErrors(const vector<int>& first, const vector<int>& second)
{
    size_t errors = 0;

    for (size_t i = 0; i < first.size(); ++i) {
        if (first[i] != second[i]) {
            ++errors;
        }
    }

    return errors;
}

unsigned int makeRandomSeed()
{
    random_device device;
    return device();
}

unsigned int parseSeed(const char* text)
{
    string value(text);
    size_t parsed = 0;

    const unsigned long parsed_seed = stoul(value, &parsed, 10);
    if (parsed != value.size() || parsed_seed > UINT_MAX) {
        throw invalid_argument("bad seed");
    }

    return static_cast<unsigned int>(parsed_seed);
}

int main(int argc, char* argv[])
{
    if (argc != 1 && argc != 3) {
        cerr << "usage: " << argv[0] << " [message_seed channel_seed]\n";
        return 1;
    }

    unsigned int message_seed = 0;
    unsigned int channel_seed = 0;

    try {
        if (argc == 3) {
            message_seed = parseSeed(argv[1]);
            channel_seed = parseSeed(argv[2]);
        } else {
            message_seed = makeRandomSeed();
            channel_seed = makeRandomSeed();
        }
    } catch (const exception& error) {
        cerr << error.what() << '\n';
        cerr << "usage: " << argv[0] << " [message_seed channel_seed]\n";
        return 1;
    }

    const ConvolutionalCodeConfig config{
        7,
        {0171, 0133}
    };

    const size_t message_size = 100000;
    const vector<double> probabilities = {
        0.0, 0.001, 0.002, 0.005, 0.01, 0.02,
        0.03, 0.05, 0.08, 0.10, 0.12, 0.15, 0.20
    };

    mt19937 generator(message_seed);
    BinarySymmetricChannel channel(channel_seed);
    ConvolutionalEncoder encoder(config);
    ViterbiDecoder decoder(config);

    cout << "message_seed=" << message_seed
         << "  channel_seed=" << channel_seed << '\n';

    vector<int> message = makeRandomBits(message_size, generator);
    vector<int> encoded = encoder.encode(message, true);

    ofstream csv("output/viterbi_ber.csv");
    if (!csv) {
        cerr << "cannot open output/viterbi_ber.csv for writing\n";
        return 1;
    }

    csv << "channel_probability,information_bits,encoded_bits,"
           "channel_bit_errors,channel_ber,decoded_bit_errors,decoded_ber\n";
    csv << fixed << setprecision(10);

    for (double probability : probabilities) {
        vector<int> received = channel.transmit(encoded, probability);
        vector<int> decoded = decoder.decode(received, message.size(), true);

        const size_t channel_errors = countBitErrors(encoded, received);
        const size_t decoded_errors = countBitErrors(message, decoded);

        const double channel_ber =
            static_cast<double>(channel_errors) / static_cast<double>(encoded.size());
        const double decoded_ber =
            static_cast<double>(decoded_errors) / static_cast<double>(message.size());

        csv << probability << ','
            << message.size() << ','
            << encoded.size() << ','
            << channel_errors << ','
            << channel_ber << ','
            << decoded_errors << ','
            << decoded_ber << '\n';

        cout << "p=" << probability
             << "  channel_ber=" << channel_ber
             << "  decoded_ber=" << decoded_ber
             << "  decoded_errors=" << decoded_errors
             << '\n';
    }

    cout << "\ncsv file saved to output/viterbi_ber.csv\n";

    return 0;
}
