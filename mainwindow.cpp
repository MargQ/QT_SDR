#include "mainwindow.h"
#include <iostream>
#include <QtCharts/QValueAxis>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), chart(new QChart()), realSeries(new QLineSeries()), imagSeries(new QLineSeries()), timer(new QTimer(this)) {
    // Инициализация SDR
    ctx = iio_create_context_from_uri("ip:192.168.3.1");
    if (!ctx) {
        std::cerr << "Unable to create context" << std::endl;
        exit(1);
    }
    rx = iio_context_find_device(ctx, "cf-ad9361-lpc");
    if (!rx) {
        std::cerr << "Unable to find device" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }
    rx0_i = iio_device_find_channel(rx, "voltage0", false);
    rx0_q = iio_device_find_channel(rx, "voltage1", false);
    if (!rx0_i || !rx0_q) {
        std::cerr << "Unable to find channels" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }

    // Настройка графика
    realSeries->setName("Real");
    imagSeries->setName("Imag");
    chart->addSeries(realSeries);
    chart->addSeries(imagSeries);
    chart->createDefaultAxes();
    chart->axisY()->setRange(-2000, 2000);
    chart->axisX()->setRange(0, 1024);
    chart->setTitle("SDR Data Visualization");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Samples");
    chart->setAxisX(axisX, realSeries);
    chart->setAxisX(axisX, imagSeries);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Amplitude");
    chart->setAxisY(axisY, realSeries);
    chart->setAxisY(axisY, imagSeries);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(700, 500);
    setCentralWidget(chartView);
    resize(700, 500);

    // Добавление строки состояния
    statusBar()->showMessage("Ready");

    // Создание меню
    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("&File");
    QMenu *viewMenu = menuBar->addMenu("&View");

    QAction *exitAction = new QAction("&Exit", this);
    fileMenu->addAction(exitAction);

    QAction *toggleDockAction = new QAction("&Toggle Dock", this);
    toggleDockAction->setCheckable(true);
    toggleDockAction->setChecked(true);
    viewMenu->addAction(toggleDockAction);

    // Создание тулбара
    QToolBar *toolBar = new QToolBar("Main Toolbar", this);
    toolBar->addAction(exitAction);
    toolBar->addAction(toggleDockAction);
    addToolBar(toolBar);

    // Создание док виджета
    QDockWidget *dockWidget_l = new QDockWidget("Control Panel", this);
    QDockWidget *dockWidget_r = new QDockWidget("Control Panel", this);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget_l);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget_r);

    // Подключение действий
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Настройка таймера
    connect(timer, &QTimer::timeout, this, &MainWindow::updateData);
    timer->start(10);
}

MainWindow::~MainWindow() {
    iio_context_destroy(ctx);
}

void MainWindow::updateData() {
    auto data = getSdrData(ctx, rx, rx0_i, rx0_q);
    if (data.empty()) {
        return;
    }

    realSeries->clear();
    imagSeries->clear();

    for (size_t i = 0; i < data.size(); ++i) {
        realSeries->append(i, data[i].real());
        imagSeries->append(i, data[i].imag());
    }
}
