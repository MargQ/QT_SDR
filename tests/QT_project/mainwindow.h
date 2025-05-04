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
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QtCharts/QSplineSeries>


#include <iostream>
#include <cstdint> // Для работы с memcpy
#include <cstring> // Для работы с memcpy
#include <fftw3.h>
#include <complex>

#include "sdr.h"
#include "sdrworker.h"
#include "qam.h"
#include "oversample.h"
#include "filter.h"
#include "synchr.h"
#include "TED.h" 

QT_CHARTS_USE_NAMESPACE



class MainWindow : public QMainWindow {
    Q_OBJECT

public:
explicit MainWindow( 
                double sampleRate_tx,
                double sampleRate_rx,
                double frequency_tx,
                double frequency_rx,
                double txGain,
                double rxGain,
                double bandwidth_tx,
        double bandwidth_rx,
        SoapySDRStream *rxStream,
        SoapySDRStream *txStream, 
        QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void closeEvent(QCloseEvent *event);
    std::vector<float> fftshift(const std::vector<float>& data);
    void updateSpectrum(const int16_t* data, size_t size);
    void updateData(const int16_t* data, size_t size);
    void updateEyeDiagram(const std::vector<std::complex<double>>& signal, const std::vector<int>& tedIndices);

    void applySdrSettings(
                            const std::string& ipAddress,
                            double gainTX,
                            double frequencyTX,
                            double sampleRateTX,
                            double bandwidthTX,
                            double gainRX,
                            double frequencyRX,
                            double sampleRateRX,
                            double bandwidthRX,
                            SoapySDRStream *rxStream,
                            SoapySDRStream *txStream
                            );
    void applyTheme(const QString &theme);
    // Слоты для управления функциями док-виджетов
    void toggleSpectrumMovable(bool enabled);
    void toggleTimeMovable(bool enabled);
    void toggleConstellationMovable(bool enabled);
    void toggleConstellationCLMovable(bool enabled);
    void toggleEyeDiagramMovable(bool enabled);
    
    void toggleSpectrumFloatable(bool enabled);
    void toggleTimeFloatable(bool enabled);
    void toggleConstellationFloatable(bool enabled);
    void toggleConstellationCLFloatable(bool enabled);
    void toggleEyeDiagramFloatable(bool enabled);

    void toggleSpectrumClosable(bool enabled);
    void toggleTimeClosable(bool enabled);
    void toggleConstellationClosable(bool enabled);
    void toggleConstellationCLClosable(bool enabled);
    void toggleEyeDiagramClosable(bool enabled);

    // Функция для установки/снятия флагов функций док-виджета
    void setDockFeatures(QDockWidget *dock, QDockWidget::DockWidgetFeature feature, bool enabled);
    
    // Методы для сохранения и загрузки настроек
    void loadSettings();
    void saveSettings();




private:

    std::vector<size_t> channelList;

    CostasLoop m_costasLoop;
    QtCharts::QChartView *chartView; // Объявление chartView

    // Буфер для хранения данных перед вычислением спектра
    std::vector<std::complex<float>> buff;
    std::vector<std::complex<double>> convertToComplex(const int16_t* data, size_t size);


    
    QValueAxis *axisX_spectrum;

    QChart *chart;
    QChart *spectrumChart;       // График спектра
    QChart *constellationChart;  // График созвездия
    QChart *constellationCLChart;  // График созвездия после Costas Loop
    QChart *eyeDiagramChart;


    // Док-виджеты для графиков
    QDockWidget *chartDock;
    QDockWidget *spectrumDock;
    QDockWidget *constellationDock;
    QDockWidget *constellationCLDock;
    QDockWidget *eyeDiagramDock;

    // Переменные для хранения текущей темы
    QString currentTheme;
    
    QAction *lightThemeAction;
    QAction *darkThemeAction;
    QAction *customThemeAction;

    QLineSeries *realSeries;
    QLineSeries *imagSeries;
    QLineSeries *spectrumSeries;
    QLineSeries *eyeDiagramSeries;
    QScatterSeries *constellationSeries; // Объявление серии для созвездия
    QScatterSeries *constellationCLSeries; // Объявление серии для созвездия

    QChartView *eyeDiagramView;

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

    QAction *ConstellationCLMovableAction;
    QAction *ConstellationCLFloatableAction;
    QAction *ConstellationCLClosableAction;

    QAction *EyeDiagramMovableAction;
    QAction *EyeDiagramFloatableAction;
    QAction *EyeDiagramClosableAction;

    
    // SoapySDRStream *rxStream;
    // SoapySDRStream *txStream;

    QVector<QPointF> m_realBuffer;
    QVector<QPointF> m_imagBuffer;
    QVector<QPointF> m_spectrumBuffer;
    QThread workerThread;

    QTimer *timer;
};

#endif // MAINWINDOW_H

