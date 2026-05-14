#pragma once

#include <complex>
#include <cstddef>
#include <vector>

class FastFourierTransform
{
public:
    typedef std::complex<double> Complex;
    typedef std::vector<Complex> ComplexVector;

    ComplexVector forward(const ComplexVector& input) const;
    ComplexVector inverse(const ComplexVector& input) const;

    bool isSupportedLength(std::size_t n) const;

private:
    ComplexVector transform(const ComplexVector& input, bool inverse) const;
    int chooseRadix(std::size_t n) const;
};
