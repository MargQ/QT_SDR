#include "mainwindow.h"
#include <iostream>
#include <QtCharts/QValueAxis>
#include "sdrworker.h"
#include <QThread>
#include <QMetaType>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), chart(new QChart()), realSeries(new QLineSeries()), imagSeries(new QLineSeries()) {

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

    // Создание и запуск sdr worker
    SdrWorker *worker = new SdrWorker(ctx, rx, rx0_i, rx0_q);
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::destroyed, &workerThread, &QThread::quit);
    connect(this, &MainWindow::destroyed, worker, &QObject::deleteLater);
    connect(worker, &SdrWorker::dataReady, this, &MainWindow::updateData);

    workerThread.start();
}

MainWindow::~MainWindow() {
    iio_context_destroy(ctx);
    workerThread.quit();
    workerThread.wait();
}

void MainWindow::updateData(const std::vector<std::complex<int16_t>>& data) {
    qDebug() << "MainWindow::updateData called from thread:" << QThread::currentThread();

    static const int resolution = 4; // Может быть 1, если data содержит уже комплексные числа
    static const int max_points = 1024;
    static const int sampleCount = data.size();

    // Прооверка на пустоту данных
    if (sampleCount == 0) {
        qWarning() << "No data to process!";
        return;
    }

    // Проверка размера data, чтобы избежать ошибок переполнения
    if (data.size() < static_cast<size_t>(sampleCount)) {
        qWarning() << "Data size is smaller than expected sample count!";
        return;
    }

    // Инициализация буфера, если он пуст
    if (m_realBuffer.isEmpty() || m_realBuffer.size() != sampleCount) {
        m_realBuffer.resize(sampleCount);
        m_imagBuffer.resize(sampleCount);
    }

    int start = 0;
    int availableSamples = std::min(static_cast<int>(data.size()), sampleCount);

    // Перенос старых данных
    if (availableSamples < sampleCount) {
        start = sampleCount - availableSamples;
        for (int s = 0; s < start; ++s) {
            m_realBuffer[s].setY(m_realBuffer.at(s + availableSamples).y());
            m_imagBuffer[s].setY(m_imagBuffer.at(s + availableSamples).y());
        }
    }

    // Нормализацияданных и обновление буфера
    for (int s = start; s < sampleCount; ++s) {
        qreal normalizedReal = qreal(data[s - start].real()) / qreal(32768)*100000;
        qreal normalizedImag = qreal(data[s - start].imag()) / qreal(32768)*100000;

        m_realBuffer[s].setX(s);
        m_realBuffer[s].setY(normalizedReal);

        m_imagBuffer[s].setX(s);
        m_imagBuffer[s].setY(normalizedImag);
    }

    // Обновление данные в графиках
    realSeries->replace(m_realBuffer);
    imagSeries->replace(m_imagBuffer);

    // Дебаг для проверки состояния буфера
    //qDebug() << "First point real:" << m_realBuffer.first().x() << m_realBuffer.first().y();
    //qDebug() << "First point imag:" << m_imagBuffer.first().x() << m_imagBuffer.first().y();
    //qDebug() << "Last point real:" << m_realBuffer.last().x() << m_realBuffer.last().y();
    //qDebug() << "Last point imag:" << m_imagBuffer.last().x() << m_imagBuffer.last().y();
}



