#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QTimer>
#include <QToolBar>
#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QThread>
#include "sdr.h"
#include "sdrworker.h"

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

    struct iio_context *ctx;
    struct iio_device *rx;
    struct iio_channel *rx0_i;
    struct iio_channel *rx0_q;

    QThread workerThread;
};

#endif // MAINWINDOW_H

