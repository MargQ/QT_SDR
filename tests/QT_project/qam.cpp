#include "qam.h"


using namespace std;

QAM_Mod::QAM_Mod(int M) : M(M) 
{
    // Проверяем, что M является степенью 2 (например, 4, 16, 64)
    int sqrt_M = std::sqrt(M);
    if (sqrt_M * sqrt_M != M) {
        std::cerr << "M must be a perfect square (4, 16, 64, etc.)" << std::endl;
        exit(1);
    }

    // Инициализируем QAM символы
    QAMSymbols.resize(M);
    for (int i = 0; i < M; ++i) {
        QAMSymbols[i] = mapToQAMSymbol(i, sqrt_M);
    }
}

std::vector<std::complex<float>> QAM_Mod::mod(const std::vector<int>& bits) 
{
    for (int bit : bits) {
    if (bit != 0 && bit != 1) {
        std::cerr << "Error: bits must contain only 0 or 1!" << std::endl;
        exit(1);
    }
}

    int symbolBits = std::log2(M); // количество битов для одного QAM символа

    // Проверяем, что bits.size() кратно symbolBits
    if (bits.size() % symbolBits != 0) {
        std::cerr << "Error: bits.size() must be divisible by symbolBits!" << std::endl;
        exit(1);
    }

    size_t numSymbols = bits.size() / symbolBits;
    std::vector<std::complex<float>> symbols(numSymbols);

    for (size_t i = 0; i + symbolBits <= bits.size(); i += symbolBits) 
    {
        int symbolIndex = 0;
        for (int j = 0; j < symbolBits; ++j) 
        {
            symbolIndex = (symbolIndex << 1) | (bits[i + j] ? 1 : 0);
        }
        symbols.at(i / symbolBits) = QAMSymbols.at(symbolIndex);
    }

    return symbols;
}

std::complex<float> QAM_Mod::mapToQAMSymbol(int index, int sqrt_M) 
{
    // QAM созвездие расположено на сетке
    int x = (index % sqrt_M) * 2 + 1 - sqrt_M;  // Реальная часть (по горизонтали)
    int y = (index / sqrt_M) * 2 + 1 - sqrt_M;  // Мнимая часть (по вертикали)

    return std::complex<float>(x, y);
}
