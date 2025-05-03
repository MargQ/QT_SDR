#ifndef FILTER_H
#define FILTER_H

#include <algorithm>
#include "mainwindow.h"

extern int syms;
extern double beta;
extern int P;

std::vector<int> FormingFilter(const std::vector<int>& signal, const std::vector<double>& filter);

std::vector<double> GenerateSRRC(int syms, double beta, int P, double t_off = 0);

std::vector<std::complex<double>> ConvolveSame(
    const std::vector<std::complex<double>>& signal,
    const std::vector<double>& filter_kernel);

std::vector<std::complex<double>> ConvolveFull(
    const std::vector<std::complex<double>>& signal,
    const std::vector<double>& filter_kernel);

#endif // FILTER_H