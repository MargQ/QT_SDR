#include "oversample.h"

// Функция оверсемплинга путём повторения каждого значения count раз
std::vector<int> oversample(const std::vector<int>& data, int count) {
    std::vector<int> oversampledData;
    for (const auto& val : data) {
        for (int i = 0; i < count; ++i) {
            oversampledData.push_back(val);  // Повторение значения count раз
        }
    }
    return oversampledData;
}
