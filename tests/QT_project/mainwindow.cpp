#include "mainwindow.h"
#include "sdrworker.h"
#include <QMetaType>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), chart(new QChart()), realSeries(new QLineSeries()), imagSeries(new QLineSeries()), spectrumSeries(new QLineSeries()), constellationSeries(new QScatterSeries()) {

    // Инициализация SDR
    ctx = iio_create_context(NULL,"ip:192.168.2.1");
    if (!ctx) {
        std::cerr << "Unable to create context" << std::endl;
        exit(1);
    }
    rx_dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
    if (!rx_dev) {
        std::cerr << "Unable to find device" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }
    tx_dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (!tx_dev) {
        std::cerr << "Unable to find device" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }

    //printf("* Инициализация потоков I/Q %s канала AD9361 \n", "TX");
    tx0_i = iio_device_find_channel(tx_dev, "voltage0", true);
    tx0_q = iio_device_find_channel(tx_dev, "voltage1", true);

    //printf("* Инициализация потоков I/Q %s канала AD9361 \n", "RX");
    rx0_i = iio_device_find_channel(rx_dev, "voltage0", false);
    rx0_q = iio_device_find_channel(rx_dev, "voltage1", false);
if (!tx0_i || !tx0_q) {
    std::cerr << "Unable to find TX channels" << std::endl;
    return;
}
    if (!rx0_i || !rx0_q) {
        std::cerr << "Unable to find channels" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }

    if (!tx0_i || !tx0_q) {
        std::cerr << "Unable to find channels tx" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }
    
    rxmask = iio_create_channels_mask(iio_device_get_channels_count(rx_dev));
    if (!rxmask) {
		fprintf(stderr, "Unable to alloc RX channels mask\n");
	}

    txmask = iio_create_channels_mask(iio_device_get_channels_count(tx_dev));
	if (!txmask) {
		fprintf(stderr, "Unable to alloc TX channels mask\n");
	}

    // Создание и настройка DockWidget для параметров SDR
    QDockWidget *dockWidget_p = new QDockWidget("SDR Configuration", this);
    QWidget *dockWidgetContent = new QWidget(dockWidget_p);
    QFormLayout *formLayout = new QFormLayout(dockWidgetContent);

    // Поля ввода для параметров
    QLineEdit *ipAddressInput = new QLineEdit("192.168.2.1", this); // Поле для IP-адреса

    // Параметры для tx
    QLineEdit *frequencyTX_Input = new QLineEdit(this); // Поле для частоты
    frequencyTX_Input->setPlaceholderText("Enter Frequency (GHz)");

    QLineEdit *sampleRateTX_Input = new QLineEdit(this); // Поле для скорости выборки
    sampleRateTX_Input->setPlaceholderText("Enter Sample Rate (MS/s)");

    QLineEdit *bandwidthTX_Input = new QLineEdit(this); // Поле для полосы
    bandwidthTX_Input->setPlaceholderText("Enter Bandwidth (MHz)");

    // Параметры для rx
    QLineEdit *frequencyRX_Input = new QLineEdit(this); // Поле для частоты
    frequencyRX_Input->setPlaceholderText("Enter Frequency (GHz)");

    QLineEdit *sampleRateRX_Input = new QLineEdit(this); // Поле для скорости выборки
    sampleRateRX_Input->setPlaceholderText("Enter Sample Rate (MS/s)");

    QLineEdit *bandwidthRX_Input = new QLineEdit(this); // Поле для полосы
    bandwidthRX_Input->setPlaceholderText("Enter Bandwidth (MHz)");

    // Кнопка "Apply"
    QPushButton *applyButton = new QPushButton("Apply", this);
    
    formLayout->addRow("IP Address:", ipAddressInput);

    formLayout->addRow("Frequency lo(tx):", frequencyTX_Input);
    formLayout->addRow("Sample Rate (tx):", sampleRateTX_Input);
    formLayout->addRow("Bandwidth (tx):", bandwidthTX_Input);

    formLayout->addRow("Frequency lo(rx):", frequencyRX_Input);
    formLayout->addRow("Sample Rate (rx):", sampleRateRX_Input);
    formLayout->addRow("Bandwidth (rx):", bandwidthRX_Input);
    formLayout->addRow(applyButton);

    dockWidgetContent->setLayout(formLayout);
    dockWidget_p->setWidget(dockWidgetContent);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget_p);

    // Подключение сигнала нажатия кнопки "Apply"
    connect(applyButton, &QPushButton::clicked, this, [this, ipAddressInput, frequencyTX_Input, sampleRateTX_Input, bandwidthTX_Input, frequencyRX_Input, sampleRateRX_Input, bandwidthRX_Input]() {
        QString ipAddress = ipAddressInput->text();
        QString frequency_lo_tx = frequencyTX_Input->text();
        QString sampleRate_tx = sampleRateTX_Input->text();
        QString bandwidth_tx = bandwidthTX_Input->text();
        
        // // Здесь вы можете добавить логику для использования введенных данных
        // qDebug() << "Applying settings with IP:" << ipAddress << ", Frequency:" << frequency << ", Sample Rate:" << sampleRate;
        
        // Обновите параметры SDR здесь
        // Например:
        // updateSdrSettings(ipAddress.toStdString(), frequency.toInt(), sampleRate.toInt());
    });

///////////////////////////////////////////////////////////////////
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
    chartView->setMinimumSize(400, 300);
    setCentralWidget(chartView);

    // Настройка графика для спектра
    QChart *spectrumChart = new QChart();
    spectrumSeries->setName("Spectrum");
    spectrumChart->addSeries(spectrumSeries);
    spectrumChart->createDefaultAxes();
    spectrumChart->axisX()->setRange(0, 512);  // Ось для частоты
    axisX->setTitleText("Frequency (MHz)");  // Подпись оси X
    spectrumChart->axisY()->setRange(15, 55);  // Ось для мощности (дБ)
    axisY->setTitleText("Power (dB)");
    spectrumChart->setTitle("Frequency Domain Visualization");

    QChartView *spectrumView = new QChartView(spectrumChart);
    spectrumView->setRenderHint(QPainter::Antialiasing);
    spectrumView->setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(chartView);
    layout->addWidget(spectrumView);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Добавление диаграммы созвездия
    constellationSeries = new QScatterSeries();
    constellationSeries->setName("Constellation");
    constellationSeries->setMarkerSize(5.0); // Размер точек

    // Настройка графика для диаграммы созвездия
    QChart *constellationChart = new QChart();
    constellationChart->addSeries(constellationSeries);
    constellationChart->createDefaultAxes();
    constellationChart->setTitle("Constellation Diagram");

    // Настроим оси X и Y для отображения созвездия
    QValueAxis *constAxisX = new QValueAxis();
    constAxisX->setRange(-1, 1); // Диапазон значений по X
    constAxisX->setTitleText("I (In-phase)");
    constellationChart->setAxisX(constAxisX, constellationSeries);

    QValueAxis *constAxisY = new QValueAxis();
    constAxisY->setRange(-1, 1); // Диапазон значений по Y
    constAxisY->setTitleText("Q (Quadrature)");
    constellationChart->setAxisY(constAxisY, constellationSeries);

    // Виджет для отображения диаграммы созвездия
    QChartView *constellationView = new QChartView(constellationChart);
    constellationView->setRenderHint(QPainter::Antialiasing);
    constellationView->setMinimumSize(400, 400);
    constellationView->setFixedSize(400, 400);

    // Добавим виджет созвездия в layout
    layout->addWidget(constellationView, 0, Qt::AlignCenter);
    //resize(500, 500);
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
    SdrWorker *worker = new SdrWorker(ctx, rx_dev, rx0_i, rx0_q, rxmask, rxstream, tx_dev, tx0_i, tx0_q, txmask, txstream);
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::destroyed, &workerThread, &QThread::quit);
    connect(worker, &SdrWorker::dataReady, this, &MainWindow::updateData);

    workerThread.start();
}

MainWindow::~MainWindow() {
    iio_context_destroy(ctx);
    workerThread.quit();
    workerThread.wait();
}

void MainWindow::updateData(const std::vector<std::complex<int16_t>>& data) {
    //qDebug() << "MainWindow::updateData called from thread:" << QThread::currentThread(); //log

    static const int resolution = 4; // Может быть 1, если data содержит уже комплексные числа
    static const int max_points = 1024;
    static const int sampleCount = data.size();

    // Проверка на пустоту данных
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

    // Очистка старых точек созвездия
    constellationSeries->clear();

    // Заполнение серии новыми точками для диаграммы созвездия с использованием memcpy
    for (int s = 0; s < sampleCount; ++s) {
        // Делим на 2048.0 для масштабирования значений созвездия от -1 до 1
        double I = data[s].real() / 2048.0; 
        double Q = data[s].imag() / 2048.0;

        // // Отладочная информация о значениях I и Q
        // if (s % 100 == 0) {
        //     qDebug() << "Data point" << s << ": I =" << I << ", Q =" << Q;
        // }

        // Используем memcpy для копирования значений I и Q в массивы
        memcpy(&m_realBuffer[s], &I, sizeof(double));
        memcpy(&m_imagBuffer[s], &Q, sizeof(double));

        constellationSeries->append(I, Q); // Сохраняем значения в серии
    }

    imagSeries->replace(m_imagBuffer);

    // Нормализация данных и обновление буфера
    for (int s = start; s < sampleCount; ++s) {
        qreal normalizedReal = qreal(data[s - start].real()) / qreal(16384) * 10000;
        qreal normalizedImag = qreal(data[s - start].imag()) / qreal(16384) * 10000;

        m_realBuffer[s].setX(s);
        m_realBuffer[s].setY(normalizedReal);

        m_imagBuffer[s].setX(s);
        m_imagBuffer[s].setY(normalizedImag);
    }

    // Обновление данных в графиках
    realSeries->replace(m_realBuffer);
    imagSeries->replace(m_imagBuffer);

    // Преобразование Фурье
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sampleCount);
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sampleCount);
    fftw_plan p = fftw_plan_dft_1d(sampleCount, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < sampleCount; i++) {
        in[i][0] = data[i].real();
        in[i][1] = data[i].imag();
    }

    fftw_execute(p);

    // Обновление спектра
    QVector<QPointF> spectrumPoints(sampleCount / 2); // Создаем вектор нужного размера

    for (int i = 0; i < sampleCount / 2; ++i) {
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        double dbValue = 10 * log10(magnitude);
        
        // Создаем точку и копируем ее в массив
        QPointF point(i, dbValue);
        std::memcpy(&spectrumPoints[i], &point, sizeof(QPointF));
        
        // Отладочная информация о значениях спектра
        // if (i % 100 == 0) { 
        //     qDebug() << "Frequency bin" << i << "dB Value:" << dbValue;
        // }
    }

    // Установка цвета графика спектра
    spectrumSeries->setColor(Qt::red); 
    spectrumSeries->replace(spectrumPoints); 

    // Очистка памяти
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);    
}




    // Дебаг для проверки состояния буфера
    //qDebug() << "First point real:" << m_realBuffer.first().x() << m_realBuffer.first().y();
    //qDebug() << "First point imag:" << m_imagBuffer.first().x() << m_imagBuffer.first().y();
    //qDebug() << "Last point real:" << m_realBuffer.last().x() << m_realBuffer.last().y();
    //qDebug() << "Last point imag:" << m_imagBuffer.last().x() << m_imagBuffer.last().y();
