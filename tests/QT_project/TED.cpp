#include "TED.h"
#include "mainwindow.h"

std::vector<int> TED(const std::vector<std::complex<double>>& data) {
    const double BnTs = 0.01;
    const int Nsps = 10;
    const double C = std::sqrt(2) / 2;
    const double Kp = 0.165;

    double teta = (BnTs / Nsps) / (C + 1 / (4 * C));
    double K1 = (-4 * C * teta) / ((1 + 2 * C * teta + teta * teta) * Kp);
    double K2 = (-4 * teta * teta) / ((1 + 2 * C * teta + teta * teta) * Kp);

    std::vector<double> err(data.size() / 10, 0.0);
    int nsp = 10;
    double p1 = 0.0;
    double p2 = 0.0;
    int n = 0;

    // Изменение типа на int
    std::vector<int> mass_cool_inex;
    std::vector<int> mass_id; 

    for (size_t ns = 0; ns < data.size() - (2 * nsp); ns += nsp) {
        // Расчет ошибки
        double real = (data[ns + nsp].real() - data[ns].real()) * data[ns + (nsp / 2)].real();
        double imag = (data[ns + nsp].imag() - data[ns].imag()) * data[ns + (nsp / 2)].imag();
        // Вычисление среднего значения
        err[ns / nsp] = (real + imag) / 2;

        double error = err[ns / nsp];
        p1 = error * K1;
        p2 += p1 + error * K2;

        // Ограничение p2 в диапазоне [-1, 1]
        if (p2 > 1.0) {
            p2 -= static_cast<int>(p2);
        } else if (p2 < -1.0) {
            p2 += static_cast<int>(-p2);
        }

        n = std::clamp(static_cast<int>(round(p2 * Nsps)), -Nsps, Nsps);
        // Приведение к int
        int n1 = static_cast<int>(n + ns + nsp);
        
        mass_cool_inex.push_back(n1);
        mass_id.push_back(n);
    }

    return mass_cool_inex; // Возвращаем массив индексов типа int
}
