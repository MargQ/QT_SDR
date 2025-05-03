#include "filter.h"


int syms = 2;       // 16 символов (8 слева + 8 справа)
double beta = 0.9; // 25% roll-off
int P = 10;         // 10 отсчётов на символ для фильтра

// Функция свёртки с формирующим фильтром
std::vector<int> FormingFilter(const std::vector<int>& signal, const std::vector<double>& filter) {
    int signal_size = signal.size();
    int filter_size = filter.size();
    std::vector<int> result(signal_size + filter_size - 1, 0);

    for (int i = 0; i < signal_size; ++i) {
        for (int j = 0; j < filter_size; ++j) {
            result[i + j] += signal[i] * filter[j];
        }
    }
    return result;
}

// Фильтр SRRC
std::vector<double> GenerateSRRC(int syms, double beta, int P, double t_off) {
    int length_SRRC = 2 * P * syms;
    std::vector<double> s(length_SRRC);
    
    // Нормировочный коэффициент
    double norm = 1.0 / sqrt(P);
    
    for (int i = 0; i < length_SRRC; ++i) {
        double t = (i - length_SRRC/2.0 + t_off) / P;
        
        if (abs(t) < 1e-8) {  // t = 0
            s[i] = norm * (1 - beta + (4*beta)/M_PI);
        }
        else if (abs(abs(t) - 1.0/(4*beta)) < 1e-8) {  // t = ±1/(4β)
            s[i] = (beta/sqrt(2*P)) * ((1 + 2/M_PI)*sin(M_PI/(4*beta)) + 
                                      (1 - 2/M_PI)*cos(M_PI/(4*beta)));
        }
        else {
            double pi_t = M_PI * t;
            double beta_t = 4 * beta * t;
            double numerator = sin(pi_t*(1-beta)) + beta_t*cos(pi_t*(1+beta));
            double denominator = pi_t * (1 - beta_t*beta_t);
            s[i] = 4 * beta * norm * numerator / denominator;
        }
    }
    
    // Дополнительная нормировка по энергии
    double energy = 0;
    for (auto v : s) energy += v*v;
    double scale = sqrt(energy);
    for (auto& v : s) v /= scale;
    
    return s;
}

// Функция свертки в режиме same - сохраняется размер входного сигнала на выходе
std::vector<std::complex<double>> ConvolveSame(
    const std::vector<std::complex<double>>& signal,
    const std::vector<double>& filter_kernel)
{
    int signal_size = signal.size();
    int filter_size = filter_kernel.size();
    int half_filter = (filter_size - 1) / 2;  // Центр фильтра

    std::vector<std::complex<double>> result(signal_size + filter_size -1, 0);

    for (int i = 0; i < signal_size; ++i) {
        std::complex<double> acc = 0;
        for (int j = 0; j < filter_size; ++j) {
            int idx = i + j - half_filter;
            if (idx >= 0 && idx < signal_size) {
                acc += signal[idx] * filter_kernel[j];
            }
        }
        result[i] = acc;
    }

    return result;
}

// Функция свертки в режиме full - выходной массив имеет размер N + M - 1
std::vector<std::complex<double>> ConvolveFull(
    const std::vector<std::complex<double>>& signal,
    const std::vector<double>& filter_kernel)
{
    int signal_size = signal.size();
    int filter_size = filter_kernel.size();
    int output_size = signal_size + filter_size - 1;
    
    std::vector<std::complex<double>> result(output_size, 0);

    for (int n = 0; n < output_size; ++n) {
        for (int k = 0; k < filter_size; ++k) {
            int idx = n - k;
            if (idx >= 0 && idx < signal_size) {
                result[n] += signal[idx] * filter_kernel[k];
            }
        }
    }

    return result;
}