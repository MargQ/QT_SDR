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
    int symbolBits = std::log2(M); // количество битов для одного QAM символа
    std::vector<std::complex<float>> symbols(bits.size() / symbolBits);
    
    for (size_t i = 0; i < bits.size(); i += symbolBits) 
    {
        int symbolIndex = 0;
        for (int j = 0; j < symbolBits; ++j) 
        {
            symbolIndex = (symbolIndex << 1) | (bits[i + j] ? 1 : 0);
        }
        symbols[i / symbolBits] = QAMSymbols[symbolIndex];
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
