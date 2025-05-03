#ifndef SYNCHR_H
#define SYNCHR_H

#include <vector>
#include <complex>

class CostasLoop {
    public:
        // Основная функция обработки
        void process(const int16_t* data, size_t size);
        
        // Получение результатов
        const std::vector<std::complex<double>>& getOutput() const;
        
    private:
        // Внутренняя функция детектора фазы
        double phaseDetector4(const std::complex<double>& sample);
        
        // Состояние петли
        double m_phase = 0.0;
        double m_freq = 0.0;
        
        // Параметры петли
        const double m_alpha = 0.132;
        const double m_beta = 0.00932;
        
        // Выходные данные
        std::vector<std::complex<double>> m_output;
    };

#endif // SYNCHR_H
