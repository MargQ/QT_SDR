#include "filter.h"

// Функция свёртки с формирующим фильтром
std::vector<int> FormingFilter(const std::vector<int>& signal, const std::vector<int>& filter) {
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


