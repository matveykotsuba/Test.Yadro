#include "FastFourierTransform.hpp"

#include <cmath>
#include <stdexcept>

using namespace std;

FastFourierTransform::ComplexVector FastFourierTransform::forward(
    const ComplexVector& input) const
{
    if (!isSupportedLength(input.size())) {
        throw invalid_argument("N can be divided only by 2, 3 and 5");
    }

    return transform(input, false);
}

FastFourierTransform::ComplexVector FastFourierTransform::inverse(
    const ComplexVector& input) const
{
    if (!isSupportedLength(input.size())) {
        throw invalid_argument("N can be divided only by 2, 3 and 5");
    }

    ComplexVector output = transform(input, true);
    const double scale = static_cast<double>(input.size());

    for (size_t i = 0; i < output.size(); ++i) {
        output[i] /= scale;
    }

    return output;
}

bool FastFourierTransform::isSupportedLength(size_t n) const
{
    if (n == 0) {
        return false;
    }

    while (n % 2 == 0) {
        n /= 2;
    }
    while (n % 3 == 0) {
        n /= 3;
    }
    while (n % 5 == 0) {
        n /= 5;
    }

    return n == 1;
}

FastFourierTransform::ComplexVector FastFourierTransform::transform(
    const ComplexVector& input,
    bool inverse) const
{
    const size_t n = input.size();
    if (n == 1) {
        return input;
    }

    const int radix = chooseRadix(n);
    if (radix == 0) {
        throw invalid_argument("unsupported fft length");
    }

    const size_t part_size = n / radix;
    vector<ComplexVector> parts(radix, ComplexVector(part_size));

    for (int s = 0; s < radix; ++s) {
        // split input into radix parts: s, s + radix, s + 2 * radix, ...
        for (size_t i = 0; i < part_size; ++i) {
            parts[s][i] = input[radix * i + s];
        }

        parts[s] = transform(parts[s], inverse);
    }

    ComplexVector output(n);
    const double pi = acos(-1.0);
    const double sign = inverse ? 1.0 : -1.0;

    for (size_t q = 0; q < part_size; ++q) {
        for (int p = 0; p < radix; ++p) {
            Complex sum(0.0, 0.0);

            for (int s = 0; s < radix; ++s) {
                const double angle_n =
                    sign * 2.0 * pi * static_cast<double>(s) *
                    static_cast<double>(q) / static_cast<double>(n);

                const double angle_r =
                    sign * 2.0 * pi * static_cast<double>(s * p) /
                    static_cast<double>(radix);

                const double angle = angle_n + angle_r;
                const Complex multiplier(cos(angle), sin(angle));
                sum += parts[s][q] * multiplier;
            }

            output[q + part_size * p] = sum;
        }
    }

    return output;
}

int FastFourierTransform::chooseRadix(size_t n) const
{
    if (n % 2 == 0) {
        return 2;
    }
    if (n % 3 == 0) {
        return 3;
    }
    if (n % 5 == 0) {
        return 5;
    }

    return 0;
}
