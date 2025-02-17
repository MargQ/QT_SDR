#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QScatterSeries>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QThread>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDockWidget>

#include <iostream>
#include <cstdint> // Для работы с memcpy
#include <cstring> // Для работы с memcpy
#include <fftw3.h> 
#include "sdr.h"
#include "sdrworker.h"
#include "qam.h"
#include "oversample.h"
#include "filter.h"

//extern std::vector<std::complex<int16_t>> shared_buffer; // Разделяемый буфер
//extern std::mutex buffer_mutex; // Мьютекс для синхронизации доступа к буферу

QT_CHARTS_USE_NAMESPACE



class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateSpectrum(const int16_t* data, size_t size);
    void updateData(const int16_t* data, size_t size);
    void applySdrSettings(SoapySDRDevice* sdr,
                            const std::string& ipAddress,
                            double gainTX,
                            double frequencyTX,
                            double sampleRateTX,
                            //double bandwidthTX,
                            double gainRX,
                            double frequencyRX,
                            double sampleRateRX,
                            size_t* channels,
                            size_t channel_count
                            //double bandwidthRX
                            );

private:

    void setupUI();

    QtCharts::QChartView *chartView; // Объявление chartView

    // Буфер для хранения данных перед вычислением спектра
    std::vector<std::complex<float>> buff;
    double sampleRate_tx = 1e6;
    double sampleRate_rx = 1e6;
    double frequency_tx = 800e6;
    double frequency_rx = 800e6;
    double txGain = -50.0;
    double rxGain = 10.0;
    QValueAxis *axisX_spectrum;
    QChart *chart;
    QLineSeries *realSeries;
    QLineSeries *imagSeries;
    QLineSeries *spectrumSeries;
    QScatterSeries *constellationSeries; // Объявление серии для созвездия
    
    SoapySDRStream *rxStream;
    SoapySDRStream *txStream;

    QVector<QPointF> m_realBuffer;
    QVector<QPointF> m_imagBuffer;
    QVector<QPointF> m_spectrumBuffer;
    QThread workerThread;
    size_t channels[1] = {0}; // {0} or {0, 1}
    QTimer *timer;
};

#endif // MAINWINDOW_H

