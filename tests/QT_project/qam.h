#ifndef QAM_H
#define QAM_H

#include <vector>
#include <complex>
#include <cmath>
#include "mainwindow.h"

class QAM_Mod {
public:
    QAM_Mod(int M);  // Конструктор

    // Функция модуляции
    std::vector<std::complex<float>> mod(const std::vector<int>& bits);

private:
    int M;  // мощность модуляции (QAM16: M=16)
    std::vector<std::complex<float>> QAMSymbols;

    // Функция для отображения индекса на QAM символ
    std::complex<float> mapToQAMSymbol(int index, int sqrt_M);
};

#endif //QAM_H