#include "oversample.h"

// Функция оверсемплинга путём повторения каждого значения count раз
std::vector<std::complex<float>> oversample(const std::vector<std::complex<float>>& data, int count) {
    std::vector<std::complex<float>> oversampledData;
    for (const auto& val : data) {
        oversampledData.push_back(val);  // Первое значение оригинала
        for (int i = 0; i < count - 1; ++i) {
            oversampledData.push_back(std::complex<float>(0.0f, 0.0f));  // Остальные - нули
        }
    }
    return oversampledData;
}

