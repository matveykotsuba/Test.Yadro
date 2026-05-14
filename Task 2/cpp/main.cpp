#include "FastFourierTransform.hpp"

#include <climits>
#include <cmath>
#include <complex>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

typedef complex<double> Complex;
typedef vector<Complex> ComplexVector;

struct ErrorInfo
{
    double max_abs_error;
    double rmse;
    double relative_error;
};

ComplexVector makeRandomInput(
    size_t n,
    mt19937& generator)
{
    uniform_real_distribution<double> value(-1.0, 1.0);

    ComplexVector result;
    result.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        result.push_back(Complex(value(generator), value(generator)));
    }

    return result;
}

ErrorInfo compareVectors(
    const ComplexVector& original,
    const ComplexVector& recovered)
{
    if (original.size() != recovered.size()) {
        throw invalid_argument("vectors must have the same size");
    }

    if (original.empty()) {
        ErrorInfo empty_result;
        empty_result.max_abs_error = 0.0;
        empty_result.rmse = 0.0;
        empty_result.relative_error = 0.0;
        return empty_result;
    }

    double max_abs_error = 0.0;
    double diff_energy = 0.0;
    double original_energy = 0.0;

    for (size_t i = 0; i < original.size(); ++i) {
        const Complex diff = original[i] - recovered[i];
        const double abs_error = abs(diff);

        if (abs_error > max_abs_error) {
            max_abs_error = abs_error;
        }
        diff_energy += norm(diff);
        original_energy += norm(original[i]);
    }

    ErrorInfo info;
    info.max_abs_error = max_abs_error;
    info.rmse = sqrt(diff_energy / static_cast<double>(original.size()));
    info.relative_error =
        original_energy == 0.0 ? 0.0 : sqrt(diff_energy / original_energy);

    return info;
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

    const unsigned long seed = stoul(value, &parsed, 10);
    if (parsed != value.size() || seed > UINT_MAX) {
        throw invalid_argument("bad seed");
    }

    return static_cast<unsigned int>(seed);
}

int main(int argc, char* argv[])
{
    if (argc != 1 && argc != 2) {
        cerr << "usage: " << argv[0] << " [seed]\n";
        return 1;
    }

    unsigned int seed = 0;
    try {
        if (argc == 2) {
            seed = parseSeed(argv[1]);
        } else {
            seed = makeRandomSeed();
        }
    } catch (const exception& error) {
        cerr << error.what() << '\n';
        cerr << "usage: " << argv[0] << " [seed]\n";
        return 1;
    }

    const vector<size_t> lengths = {
        2, 3, 5, 6, 10, 12, 15, 30, 45, 60, 75, 90, 125, 150, 300, 1024
    };

    FastFourierTransform fft;
    mt19937 generator(seed);

    cout << "seed=" << seed << '\n';

    ofstream csv("output/fft_errors.csv");
    if (!csv) {
        cerr << "cannot open output/fft_errors.csv for writing\n";
        return 1;
    }

    ofstream report("output/fft_report.txt");
    if (!report) {
        cerr << "cannot open output/fft_report.txt for writing\n";
        return 1;
    }

    csv << "length,maximum_absolute_error,root_mean_square_error,relative_error\n";
    csv << scientific << setprecision(12);

    // UTF-8 BOM helps Windows text editors read Russian text correctly.
    report << "\xEF\xBB\xBF";
    report << "Отчет о точности FFT\n\n";
    report << "Для каждой длины N программа делает следующее:\n";
    report << "1. Генерирует случайный комплексный входной вектор x.\n";
    report << "2. Считает spectrum = forward(x).\n";
    report << "3. Считает recovered = inverse(spectrum).\n";
    report << "4. Сравнивает x и recovered поэлементно.\n\n";

    report << "Столбцы с ошибками:\n";
    report << "length - длина преобразования N.\n";
    report << "maximum_absolute_error - максимальное значение |x[i] - recovered[i]|.\n";
    report << "root_mean_square_error - sqrt(sum |x[i] - recovered[i]|^2 / N).\n";
    report << "Это типичная средняя ошибка восстановления одного элемента.\n";
    report << "relative_error - относительная ошибка всего восстановленного вектора.\n";
    report << "Она показывает, насколько ошибка мала по сравнению с масштабом исходных данных:\n";
    report << "sqrt(sum |x[i] - recovered[i]|^2 / sum |x[i]|^2).\n\n";

    report << left
           << setw(8) << "length"
           << setw(26) << "maximum_absolute_error"
           << setw(28) << "root_mean_square_error"
           << "relative_error"
           << '\n';
    report << scientific << setprecision(6);

    cout << scientific << setprecision(6);

    for (size_t index = 0; index < lengths.size(); ++index) {
        const size_t n = lengths[index];
        ComplexVector input = makeRandomInput(n, generator);

        ComplexVector spectrum = fft.forward(input);
        ComplexVector recovered = fft.inverse(spectrum);

        const ErrorInfo error = compareVectors(input, recovered);

        csv << n << ','
            << error.max_abs_error << ','
            << error.rmse << ','
            << error.relative_error << '\n';

        report << left
               << setw(8) << n
               << setw(26) << error.max_abs_error
               << setw(28) << error.rmse
               << error.relative_error
               << '\n';

        cout << "N=" << n
             << "  max_abs_error=" << error.max_abs_error
             << "  rmse=" << error.rmse
             << "  relative_error=" << error.relative_error
             << '\n';
    }

    cout << "\nreport file saved to output/fft_report.txt\n";
    cout << "csv file saved to output/fft_errors.csv\n";

    return 0;
}
