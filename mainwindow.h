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
#include "sdr.h"

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateData();

private:
    QChart *chart;
    QLineSeries *realSeries;
    QLineSeries *imagSeries;
    QTimer *timer;
    struct iio_context *ctx;
    struct iio_device *rx;
    struct iio_channel *rx0_i;
    struct iio_channel *rx0_q;
};

#endif // MAINWINDOW_H
