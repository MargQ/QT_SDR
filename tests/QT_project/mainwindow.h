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



QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateData(const std::vector<std::complex<int16_t>>& data);

private:
    QChart *chart;
    QLineSeries *realSeries;
    QLineSeries *imagSeries;
    QLineSeries *spectrumSeries;
    QScatterSeries *constellationSeries; // Объявление серии для созвездия

    struct iio_context* ctx;
    struct iio_device* rx_dev;
    struct iio_device* tx_dev;
    struct iio_channel* rx0_i;
    struct iio_channel *tx0_i;
    struct iio_channel* rx0_q;
    struct iio_channel *tx0_q;
    struct iio_channels_mask* rxmask = NULL;
    struct iio_channels_mask* txmask = NULL;
    struct iio_stream  *rxstream = NULL;
    struct iio_stream  *txstream = NULL;
    
    

    QVector<QPointF> m_realBuffer;
    QVector<QPointF> m_imagBuffer;
    QVector<QPointF> m_spectrumBuffer;
    QThread workerThread;
};

#endif // MAINWINDOW_H

