#include "synchr.h"

void CostasLoop::process(const int16_t* data, size_t size) {
    m_output.resize(size/2);
    
    for (size_t i = 0; i < size/2; ++i) {
        // Получение комплексного отсчета
        std::complex<double> sample(
            data[2*i] / 2048.0,
            data[2*i + 1] / 2048.0
        );
        
        // Коррекция фазы
        m_output[i] = sample * std::exp(std::complex<double>(0, -m_phase));
        
        // Обновление фазы и частоты
        double error = phaseDetector4(m_output[i]);
        m_freq += m_beta * error;
        m_phase += m_freq + m_alpha * error;
        
        // Нормализация фазы
        while (m_phase >= 2 * M_PI) m_phase -= 2 * M_PI;
        while (m_phase < 0) m_phase += 2 * M_PI;
    }
}

double CostasLoop::phaseDetector4(const std::complex<double>& sample) {
    double a = (sample.real() > 0) ? 1.0 : -1.0;
    double b = (sample.imag() > 0) ? 1.0 : -1.0;
    return a * sample.imag() - b * sample.real();
}

const std::vector<std::complex<double>>& CostasLoop::getOutput() const {
    return m_output;
}