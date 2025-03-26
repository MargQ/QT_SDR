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
#include <QToolButton>
#include <QSettings>

#include <iostream>
#include <cstdint> // Для работы с memcpy
#include <cstring> // Для работы с memcpy
#include <fftw3.h> 
#include "sdr.h"
#include "sdrworker.h"
#include "qam.h"
#include "oversample.h"
#include "filter.h"

QT_CHARTS_USE_NAMESPACE



class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void closeEvent(QCloseEvent *event);
    std::vector<float> fftshift(const std::vector<float>& data);
    void updateSpectrum(const int16_t* data, size_t size);
    void updateData(const int16_t* data, size_t size);
    void applySdrSettings(SoapySDRDevice* sdr,
                            const std::string& ipAddress,
                            double gainTX,
                            double frequencyTX,
                            double sampleRateTX,
                            double bandwidthTX,
                            double gainRX,
                            double frequencyRX,
                            double sampleRateRX,
                            size_t* channels,
                            size_t channel_count,
                            double bandwidthRX
                            );
    void applyTheme(const QString &theme);
    // Слоты для управления функциями док-виджетов
    void toggleSpectrumMovable(bool enabled);
    void toggleTimeMovable(bool enabled);
    void toggleConstellationMovable(bool enabled);
    
    void toggleSpectrumFloatable(bool enabled);
    void toggleTimeFloatable(bool enabled);
    void toggleConstellationFloatable(bool enabled);

    void toggleSpectrumClosable(bool enabled);
    void toggleTimeClosable(bool enabled);
    void toggleConstellationClosable(bool enabled);

    // Функция для установки/снятия флагов функций док-виджета
    void setDockFeatures(QDockWidget *dock, QDockWidget::DockWidgetFeature feature, bool enabled);
    
    // Методы для сохранения и загрузки настроек
    void loadSettings();
    void saveSettings();



private:

    QtCharts::QChartView *chartView; // Объявление chartView

    // Буфер для хранения данных перед вычислением спектра
    std::vector<std::complex<float>> buff;
    // Параметры SDR
    double sampleRate_tx = 1e6;
    double sampleRate_rx = 1e6;
    double frequency_tx = 800e6;
    double frequency_rx = 800e6;
    double txGain = -50.0;
    double rxGain = 10.0;
    double bandwidth_tx = 20e6;
    double bandwidth_rx = 20e6;
    QValueAxis *axisX_spectrum;

    QChart *chart;
    QChart *spectrumChart;       // График спектра
    QChart *constellationChart;  // График созвездия

    // Док-виджеты для графиков
    QDockWidget *chartDock;
    QDockWidget *spectrumDock;
    QDockWidget *constellationDock;

    // Переменные для хранения текущей темы
    QString currentTheme;
    
    QAction *lightThemeAction;
    QAction *darkThemeAction;
    QAction *customThemeAction;

    QLineSeries *realSeries;
    QLineSeries *imagSeries;
    QLineSeries *spectrumSeries;
    QScatterSeries *constellationSeries; // Объявление серии для созвездия

    // Действия для управления графиками
    QAction *SpectrumMovableAction;
    QAction *SpectrumFloatableAction;
    QAction *SpectrumClosableAction;

    QAction *TimeMovableAction;
    QAction *TimeFloatableAction;
    QAction *TimeClosableAction;

    QAction *ConstellationMovableAction;
    QAction *ConstellationFloatableAction;
    QAction *ConstellationClosableAction;
    
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

