#include "mainwindow.h"
#include "sdrworker.h"
#include "ascii_art_dft.hpp"
#include <QMetaType>
#include <QDebug>
#include <QDoubleValidator>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), chart(new QChart()), realSeries(new QLineSeries()), imagSeries(new QLineSeries()), spectrumSeries(new QLineSeries()), constellationSeries(new QScatterSeries()) {

// Инициализация SDR
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:");
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");
    }

    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1024");
    SoapySDRKwargs_set(&args, "loopback", "0");

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL)
    {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        exit(1);
    }

    // // Настройка TX и RX
    // double sampleRate_tx = 1e6;
    // double sampleRate_rx = 1e6;
    // double frequency_tx = 800e6;
    // double frequency_rx = 800e6;
    // double txGain = -50.0;
    // double rxGain = 10.0;

    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sampleRate_rx) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, frequency_rx, NULL) != 0)
    {
        printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sampleRate_tx) != 0)
    {
        printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, frequency_tx, NULL) != 0)
    {
        printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
    }
    //printf("SoapySDRDevice_getFrequency tx: %lf\n", SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));
    // Настройка каналов и стримов
    size_t channels[] = {0}; // {0} or {0, 1} 
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);

    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], rxGain) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], txGain) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }
    

    // Создание и настройка DockWidget для параметров SDR
    QDockWidget *dockWidget_p = new QDockWidget("SDR Configuration", this);
    QWidget *dockWidgetContent = new QWidget(dockWidget_p);
    QFormLayout *formLayout = new QFormLayout(dockWidgetContent);
    // Установка валидатора для научной нотации
    QDoubleValidator *validator = new QDoubleValidator(parent);
    validator->setNotation(QDoubleValidator::ScientificNotation);

    // Поля ввода для параметров
    QLineEdit *ipAddressInput = new QLineEdit("192.168.2.1", this); // Поле для IP-адреса

    // Параметры для tx
    QLineEdit *gainTX_Input = new QLineEdit(QString::number(txGain), this); // Поле для усиления
    gainTX_Input->setPlaceholderText("Enter Gain (dB)");

    QLineEdit *frequencyTX_Input = new QLineEdit(QString::number(frequency_tx), this); // Поле для частоты
    frequencyTX_Input->setValidator(validator);
    frequencyTX_Input->setText(QString::number(frequency_tx, 'g', 10)); //'g' формат с точностью 10
    frequencyTX_Input->setPlaceholderText("Enter Frequency (Hz)");

    QLineEdit *sampleRateTX_Input = new QLineEdit(this); // Поле для скорости выборки
    sampleRateTX_Input->setValidator(validator);
    sampleRateTX_Input->setText(QString::number(sampleRate_tx, 'g', 10)); //'g' формат с точностью 10
    sampleRateTX_Input->setPlaceholderText("Enter Sample Rate (S/s)");

    // QLineEdit *bandwidthTX_Input = new QLineEdit(this); // Поле для полосы
    // bandwidthTX_Input->setValidator(validator);
    // bandwidthTX_Input->setText(QString::number(, 'g', 10));
    // bandwidthTX_Input->setPlaceholderText("Enter Bandwidth (Hz)");

    // Параметры для rx
    QLineEdit *gainRX_Input = new QLineEdit(QString::number(rxGain), this); // Поле для усиления
    gainRX_Input->setPlaceholderText("Enter Gain (dB)");

    QLineEdit *frequencyRX_Input = new QLineEdit(QString::number(frequency_rx), this); // Поле для частоты
    frequencyRX_Input->setValidator(validator);
    frequencyRX_Input->setText(QString::number(frequency_rx, 'g', 10)); //'g' формат с точностью 10
    frequencyRX_Input->setPlaceholderText("Enter Frequency (Hz)");

    QLineEdit *sampleRateRX_Input = new QLineEdit(this); // Поле для скорости выборки
    sampleRateRX_Input->setValidator(validator);
    sampleRateRX_Input->setText(QString::number(sampleRate_rx, 'g', 10)); //'g' формат с точностью 10
    sampleRateRX_Input->setPlaceholderText("Enter Sample Rate (S/s)");

    // QLineEdit *bandwidthRX_Input = new QLineEdit(this); // Поле для полосы
    // bandwidthRX_Input->setValidator(validator);
    // bandwidthRX_Input->setText(QString::number(, 'g', 10));
    // bandwidthRX_Input->setPlaceholderText("Enter Bandwidth (Hz)");

    

    // Кнопка "Apply"
    QPushButton *applyButton = new QPushButton("Apply", this);
    
    formLayout->addRow("IP Address:", ipAddressInput);

    //formLayout->addRow("Gain(tx), dB:", gainTX_Input);
    formLayout->addRow("Frequency lo(tx), Hz:", frequencyTX_Input);
    formLayout->addRow("Sample Rate (tx), S/s:", sampleRateTX_Input);
    //formLayout->addRow("Bandwidth (tx):", bandwidthTX_Input);

    //formLayout->addRow("Gain(rx), dB:", gainRX_Input);
    formLayout->addRow("Frequency lo(rx), Hz:", frequencyRX_Input);
    formLayout->addRow("Sample Rate (rx), S/s:", sampleRateRX_Input);
    //formLayout->addRow("Bandwidth (rx):", bandwidthRX_Input);
    formLayout->addRow(applyButton);

    dockWidgetContent->setLayout(formLayout);
    dockWidget_p->setWidget(dockWidgetContent);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget_p);

    // Подключение сигнала нажатия кнопки "Apply"
    connect(applyButton, &QPushButton::clicked, this, [this, 
                                                        sdr,
                                                        ipAddressInput,
                                                        gainTX_Input,
                                                        frequencyTX_Input, 
                                                        sampleRateTX_Input, 
                                                        //bandwidthTX_Input, 
                                                        gainRX_Input, 
                                                        frequencyRX_Input, 
                                                        sampleRateRX_Input,
                                                        &channels,
                                                        channel_count
                                                        //bandwidthRX_Input
                                                        ]() {
        
        QString ipAddress = ipAddressInput->text();
        QString gain_tx = gainTX_Input->text();
        QString frequency_lo_tx = frequencyTX_Input->text();
        QString sampRate_tx = sampleRateTX_Input->text();
        //QString bandwidth_tx = bandwidthTX_Input->text();
        QString gain_rx = gainRX_Input->text();
        QString frequency_lo_rx = frequencyRX_Input->text();
        QString sampRate_rx = sampleRateRX_Input->text();
        //QString bandwidth_rx = bandwidthRX_Input->text();
        
        // // можно добавить логику для использования введенных данных
        // qDebug() << "Applying settings with IP:" << ipAddress << ", Frequency:" << frequency << ", Sample Rate:" << sampleRate;
        // Применяем настройки SDR
        applySdrSettings(sdr, ipAddress.toStdString(), gain_tx.toDouble(),
                        frequency_lo_tx.toDouble(), sampRate_tx.toDouble(),
                        //bandwidth_tx.toDouble(),
                        gain_rx.toDouble(), frequency_lo_rx.toDouble(), sampRate_rx.toDouble(), channels, channel_count
                        //bandwidth_rx.toDouble()
                        );

        // Обновляем диапазон оси X графика спектра
        this->axisX_spectrum->setRange((frequency_rx - sampleRate_rx/2) / 1e6, (frequency_rx + sampleRate_rx/2) / 1e6);

    });

///////////////////////////////////////////////////////////////////
    //setupUI();
    // Настройка графика
    realSeries->setName("Real");
    imagSeries->setName("Imag");
    chart->addSeries(realSeries);
    chart->addSeries(imagSeries);
    chart->createDefaultAxes();
    chart->axisY()->setRange(-1000, 1000);
    chart->axisX()->setRange(0, 1024);
    chart->setTitle("SDR Data Visualization");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Samples");
    chart->setAxisX(axisX, realSeries);
    chart->setAxisX(axisX, imagSeries);

    QValueAxis *axisY_time = new QValueAxis();
    axisY_time->setTitleText("Amplitude");
    chart->setAxisY(axisY_time, realSeries);
    chart->setAxisY(axisY_time, imagSeries);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing, false);
    chartView->setMinimumSize(400, 300);
    setCentralWidget(chartView);

    // Настройка графика для спектра
    QChart *spectrumChart = new QChart();
    spectrumSeries->setName("Spectrum");
    spectrumChart->addSeries(spectrumSeries);
    // spectrumChart->createDefaultAxes();

    // Создаем и настраиваем ось X для графика спектра
    axisX_spectrum = new QValueAxis();
    axisX_spectrum->setTitleText("Frequency (MHz)");
    axisX_spectrum->setRange((frequency_rx - sampleRate_rx/2) / 1e6, (frequency_rx + sampleRate_rx/2) / 1e6);
    spectrumChart->addAxis(axisX_spectrum, Qt::AlignBottom);
    spectrumSeries->attachAxis(axisX_spectrum);

    // Создаем и настраиваем ось Y для графика спектра
    QValueAxis *axisY_spectrum = new QValueAxis();
    axisY_spectrum->setTitleText("Power (dB)");
    axisY_spectrum->setRange(15, 55);
    spectrumChart->addAxis(axisY_spectrum, Qt::AlignLeft);
    spectrumSeries->attachAxis(axisY_spectrum);

    spectrumChart->setTitle("Frequency Domain Visualization");

    QChartView *spectrumView = new QChartView(spectrumChart);
    spectrumView->setRenderHint(QPainter::Antialiasing, false);
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
    constellationView->setRenderHint(QPainter::Antialiasing, false);
    constellationView->setMinimumSize(400, 400);
    constellationView->setFixedSize(300, 300);

    // Добавим виджет созвездия в layout
    layout->addWidget(constellationView, 0, Qt::AlignLeft);
    //resize(500, 500);
    // Добавление строки состояния
    statusBar()->showMessage("Ready");

    //////////////////////////////////////////
    // QVBoxLayout *l1ayout = new QVBoxLayout;
    // l1ayout->addWidget(chartView);
    // QWidget *centralWidgets = new QWidget(this);
    // centralWidgets->setLayout(l1ayout);
    // setCentralWidget(centralWidgets);
//////////////////////////////////////////

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

    // // Создание док виджета
    // QDockWidget *dockWidget_l = new QDockWidget("Control Panel", this);
    // QDockWidget *dockWidget_r = new QDockWidget("Control Panel", this);
    // addDockWidget(Qt::LeftDockWidgetArea, dockWidget_l);
    // addDockWidget(Qt::RightDockWidgetArea, dockWidget_r);

    // Подключение действий
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Создание и запуск sdr worker
    SdrWorker *worker = new SdrWorker(sdr, channels, channel_count);

 
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::destroyed, &workerThread, &QThread::quit);
    connect(worker, &SdrWorker::dataReady, this, &MainWindow::updateData);

    workerThread.start();
}

MainWindow::~MainWindow() {
    workerThread.quit();
    workerThread.wait();
}

// // Метод создает интерфейс
// void MainWindow::setupUI() {
//     QVBoxLayout *layout = new QVBoxLayout;
//     layout->addWidget(chartView);
//     QWidget *centralWidget = new QWidget(this);
//     centralWidget->setLayout(layout);
//     setCentralWidget(centralWidget);
// }
void MainWindow::updateSpectrum(const int16_t* data, size_t size) {
    // Проверка на пустоту данных
    if (size == 0) {
        qWarning() << "No data to process!";
        return;
    }

    // Очистка буфера
    buff.clear();

    // Заполнение буфера данными
    for (size_t i = 0; i < size; ++i) {
        float i_val = data[2 * i] / 2048.0f;      // Реальная часть
        float q_val = data[2 * i + 1] / 2048.0f;  // Мнимая часть
        buff.push_back(std::complex<float>(i_val, q_val));
    }

    // Вычисление логарифмической мощности спектра
    ascii_art_dft::log_pwr_dft_type lpdft = ascii_art_dft::log_pwr_dft(&buff.front(), buff.size());

    // Очистка текущих данных спектра
    spectrumSeries->clear();

    // Заполнение спектра новыми данными
    for (size_t i = 0; i < lpdft.size(); ++i) {
        spectrumSeries->append(i, lpdft[i]);
    }

    // Обновление графика спектра
    QChart *spectrumChart = spectrumSeries->chart();
    if (spectrumChart) {
        spectrumChart->removeSeries(spectrumSeries);
        spectrumChart->addSeries(spectrumSeries);
        spectrumChart->createDefaultAxes();
    }
}

void MainWindow::updateData(const int16_t* data, size_t size) {
    // Проверка на пустоту данных
    if (size == 0) {
        qWarning() << "No data to process!";
        return;
    }

    // Инициализация буфера, если он пуст
    if (m_realBuffer.isEmpty() || m_realBuffer.size() != size) {
        m_realBuffer.resize(size);
        m_imagBuffer.resize(size);
    }

    // Очистка буферов перед заполнением
    m_realBuffer.fill(QPointF(0, 0));
    m_imagBuffer.fill(QPointF(0, 0));

    // Очистка старых точек созвездия
    constellationSeries->clear();

    // Заполнение серии новыми точками для диаграммы созвездия
    for (size_t i = 5; i < size; ++i) {
        if (2 * i + 1 >= size * 2) continue; // Защита от выхода за границы
        // Извлекаем реальную и мнимую части
        int16_t real = data[2 * i];      // Реальная часть
        int16_t imag = data[2 * i + 1];  // Мнимая часть
       // QChart *chart = new QChart();
    // chart->addSeries(series);
    // chart->createDefaultAxes();
    // chart->setTitle("DFT Log Power Spectrum");

    // QValueAxis *axisX = new QValueAxis();
    // axisX->setLabelFormat("%d");
    // axisX->setTitleText("Frequency Bin");
    // chart->setAxisX(axisX, series);

    // QValueAxis *axisY = new QValueAxis();
    // axisY->setLabelFormat("%.2f");
    // axisY->setTitleText("Magnitude (dB)");
    // chart->setAxisY(axisY, series);

    // chartView->setChart(chart);
    // chartView->setRenderHint(QPainter::Antialiasing);
    // chartView->setMinimumSize(900, 700);
    // setCentralWidget(chartView);
    // resize(700, 500);
        // Нормализация значений от -2048 до 2048
        double I = real / 2048.0; 
        double Q = imag / 2048.0;

        constellationSeries->append(I*2, Q*2); // Сохраняем значения в серии

        // Заполнение буферов
        m_realBuffer[i].setX(i);
        m_realBuffer[i].setY(I * 1000); // Масштабирование по необходимости
        m_imagBuffer[i].setX(i);
        m_imagBuffer[i].setY(Q * 1000); // Масштабирование по необходимости
    }

    // Обновление данных в графиках
    realSeries->replace(m_realBuffer);
    imagSeries->replace(m_imagBuffer);

    //ascii_art_dft::log_pwr_dft_type lpdft = ascii_art_dft::log_pwr_dft(data, size);
    // // Создание графика
    // QLineSeries *series = new QLineSeries();
    // for (size_t i = 0; i < lpdft.size(); ++i) {
    //     series->append(i, lpdft[i]);
    // }

    // QChart *chart = new QChart();
    // chart->addSeries(series);
    // chart->createDefaultAxes();
    // chart->setTitle("DFT Log Power Spectrum");

    // QValueAxis *axisX = new QValueAxis();
    // axisX->setLabelFormat("%d");
    // axisX->setTitleText("Frequency Bin");
    // chart->setAxisX(axisX, series);

    // QValueAxis *axisY = new QValueAxis();
    // axisY->setLabelFormat("%.2f");
    // axisY->setTitleText("Magnitude (dB)");
    // chart->setAxisY(axisY, series);

    // chartView->setChart(chart);
    // chartView->setRenderHint(QPainter::Antialiasing);
    // chartView->setMinimumSize(900, 700);
    // setCentralWidget(chartView);
    // resize(700, 500);

    // Обновление спектра
    //updateSpectrum(data, size);
    // Преобразование Фурье
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * size);
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * size);
    fftw_plan p = fftw_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Заполнение входного массива
    for (size_t i = 0; i < size; i++) {
        in[i][0] = data[2 * i];      // Реальная часть
        in[i][1] = data[2 * i + 1];  // Мнимая часть
    }

    // Выполнение FFT
    fftw_execute(p);

    // Обновление спектра
    QVector<QPointF> spectrumPoints(size);
    double freqStart = (frequency_rx - sampleRate_rx / 2) / 1e6; // Левая граница (МГц)
    double freqStep = sampleRate_rx / (size * 1e6);             // Шаг между точками (МГц)

    // FFT Shift
    for (size_t i = 0; i < size; ++i) {
        size_t shiftedIndex = (i + size / 2) % size; // Центрируем спектр
        double magnitude = sqrt(out[shiftedIndex][0] * out[shiftedIndex][0] +
                                out[shiftedIndex][1] * out[shiftedIndex][1]);

        if (magnitude < 1e-10) { // Защита от log10(0)
            magnitude = 1e-10;
        }

        double dbValue = 10 * log10(magnitude);

        double frequency = freqStart + i * freqStep; // Масштабируем частоту

        spectrumPoints[i] = QPointF(frequency, dbValue);
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

void MainWindow::applySdrSettings(SoapySDRDevice* sdr,
                                const std::string& ipAddress,
                                double gainTX,
                                double frequencyTX,
                                double sampleRateTX,
                                double gainRX,
                                double frequencyRX,
                                double sampleRateRX,
                                size_t* channels,
                                size_t channel_count) {
    
    // // Применение настроек TX
    // if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], gainRX) !=0 ){
    //     printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    // }

    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, frequencyTX, NULL) != 0) {
        qCritical() << "Ошибка установки частоты TX:" << SoapySDRDevice_lastError();
        return;
    }
    
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sampleRateTX) != 0) {
        qCritical() << "Ошибка установки скорости выборки TX:" << SoapySDRDevice_lastError();
        return;
    }
    
    // if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_TX, 0, bandwidthTX) != 0) {
    //     qCritical() << "Ошибка установки полосы пропускания TX:" << SoapySDRDevice_lastError();
    //     return;
    // }

   // Применение настроек RX
   
    // if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], gainRX) !=0 ){
    //     printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    // }

    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, frequencyRX, NULL) != 0) {
        //qCritical() << "Ошибка установки частоты RX:" << SoapySDRDevice_lastError();
        return;
    }
   
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sampleRateRX) != 0) {
        qCritical() << "Ошибка установки скорости выборки RX:" << SoapySDRDevice_lastError();
        return;
    }
   
    qDebug() << "Настройки SDR успешно применены.";

    // Обновление параметров SDR в MainWindow
        this->sampleRate_tx = sampleRateTX;
        this->sampleRate_rx = sampleRateRX;
        this->frequency_tx = frequencyTX;
        this->frequency_rx = frequencyRX;
        //this->txGain = gainTX;
        //this->rxGain = gainRX;
}
