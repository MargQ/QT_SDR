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
    if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_TX, 0, bandwidth_rx) != 0) {
        qCritical() << "Ошибка установки полосы пропускания TX:" << SoapySDRDevice_lastError();
        return;
    }

    if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_TX, 0, bandwidth_tx) != 0) {
        qCritical() << "Ошибка установки полосы пропускания TX:" << SoapySDRDevice_lastError();
        return;
    }

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

        // Фиксируем док-виджет слева
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget_p);

    // Запрещаем открепление и перемещение
    dockWidget_p->setFeatures(QDockWidget::DockWidgetClosable);  // Только закрытие

    // Фиксируем ширину док-виджета
    dockWidget_p->setFixedWidth(300);  // Ширина 300 пикселей
    
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

    QLineEdit *bandwidthTX_Input = new QLineEdit(this); // Поле для полосы
    bandwidthTX_Input->setValidator(validator);
    bandwidthTX_Input->setText(QString::number(bandwidth_tx, 'g', 10));
    bandwidthTX_Input->setPlaceholderText("Enter Bandwidth (Hz)");

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

    QLineEdit *bandwidthRX_Input = new QLineEdit(this); // Поле для полосы
    bandwidthRX_Input->setValidator(validator);
    bandwidthRX_Input->setText(QString::number(bandwidth_rx, 'g', 10));
    bandwidthRX_Input->setPlaceholderText("Enter Bandwidth (Hz)");

    

    // Кнопка "Apply"
    QPushButton *applyButton = new QPushButton("Apply", this);
    
    formLayout->addRow("IP Address:", ipAddressInput);

    formLayout->addRow("Gain(tx), dB:", gainTX_Input);
    formLayout->addRow("Frequency lo(tx), Hz:", frequencyTX_Input);
    formLayout->addRow("Sample Rate (tx), S/s:", sampleRateTX_Input);
    formLayout->addRow("Bandwidth (tx):", bandwidthTX_Input);

    formLayout->addRow("Gain(rx), dB:", gainRX_Input);
    formLayout->addRow("Frequency lo(rx), Hz:", frequencyRX_Input);
    formLayout->addRow("Sample Rate (rx), S/s:", sampleRateRX_Input);
    formLayout->addRow("Bandwidth (rx):", bandwidthRX_Input);
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
                                                        bandwidthTX_Input, 
                                                        gainRX_Input, 
                                                        frequencyRX_Input, 
                                                        sampleRateRX_Input,
                                                        &channels,
                                                        channel_count,
                                                        bandwidthRX_Input
                                                        ]() {
        
        QString ipAddress = ipAddressInput->text();
        QString gain_tx = gainTX_Input->text();
        QString frequency_lo_tx = frequencyTX_Input->text();
        QString sampRate_tx = sampleRateTX_Input->text();
        QString bandwidth_tx = bandwidthTX_Input->text();
        QString gain_rx = gainRX_Input->text();
        QString frequency_lo_rx = frequencyRX_Input->text();
        QString sampRate_rx = sampleRateRX_Input->text();
        QString bandwidth_rx = bandwidthRX_Input->text();
        
        // // можно добавить логику для использования введенных данных
        // qDebug() << "Applying settings with IP:" << ipAddress << ", Frequency:" << frequency << ", Sample Rate:" << sampleRate;
        // Применяем настройки SDR
        applySdrSettings(sdr, ipAddress.toStdString(), gain_tx.toDouble(),
                        frequency_lo_tx.toDouble(), sampRate_tx.toDouble(),
                        bandwidth_tx.toDouble(),
                        gain_rx.toDouble(), frequency_lo_rx.toDouble(), sampRate_rx.toDouble(), channels, channel_count,
                        bandwidth_rx.toDouble()
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

    QChartView *chartView = new QChartView(chart); //////
    chartView->setRenderHint(QPainter::Antialiasing, false);
    chartView->setMinimumSize(400, 300);
    setCentralWidget(chartView);

    // Настройка графика для спектра
    spectrumChart = new QChart();
    spectrumSeries->setName("Spectrum");
    spectrumChart->addSeries(spectrumSeries);
    // spectrumChart->createDefaultAxes();

    // Создаем и настраиваем ось X для графика спектра
    // В конструкторе MainWindow
    axisX_spectrum = new QValueAxis();
    axisX_spectrum->setTitleText("Frequency (MHz)");
    //axisX_spectrum->setRange(0, 1023); // num_bins — количество бинов FFT
    axisX_spectrum->setRange((frequency_rx - sampleRate_rx/2) / 1e6, (frequency_rx + sampleRate_rx/2) / 1e6);

    spectrumChart->addAxis(axisX_spectrum, Qt::AlignBottom);
    spectrumSeries->attachAxis(axisX_spectrum);

    QValueAxis *axisY_spectrum = new QValueAxis();
    axisY_spectrum->setTitleText("Power (dB)");
    axisY_spectrum->setRange(-100, 0); // Диапазон мощности в dB
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
    constellationChart = new QChart();
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

    ////ВЗАИМОДЕЙСТВИЕ С ГРАФИКАМИ//////////////



    // Создаем док-виджеты для каждого графика
    QDockWidget *chartDock = new QDockWidget("Time Chart", this);
    QDockWidget *spectrumDock = new QDockWidget("Spectrum Chart", this);
    QDockWidget *constellationDock = new QDockWidget("Constellation Chart", this);

    // Добавляем QChartView в док-виджеты
    chartDock->setWidget(chartView);
    spectrumDock->setWidget(spectrumView);
    constellationDock->setWidget(constellationView);

    // Задаем начальное расположение графиков
    addDockWidget(Qt::RightDockWidgetArea, spectrumDock);          // Временной график сверху
    splitDockWidget(spectrumDock, chartDock, Qt::Vertical);   // Спектр под временным графиком
    splitDockWidget(chartDock, constellationDock, Qt::Horizontal);  // Созвездие справа от спектра

    // Разрешаем открепление, перемещение и сворачивание
    chartDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    spectrumDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    constellationDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);

    // Настройка пропорций
    QList<QDockWidget*> leftDocks;
    leftDocks << dockWidget_p;
    resizeDocks(leftDocks, {100}, Qt::Horizontal);  // Ширина левой области

    QList<QDockWidget*> rightDocks;
    rightDocks << chartDock << spectrumDock << constellationDock;
    resizeDocks(rightDocks, {700, 400, 300}, Qt::Vertical);  // Настраиваем пропорции графиков
    
    // Для каждого док-виджета
    QAction* toggleFullscreen = new QAction("Развернуть", this);
    connect(toggleFullscreen, &QAction::triggered, [chartDock]() {
        if (chartDock->isFloating()) {
            chartDock->showFullScreen();
        } else {
            chartDock->showNormal();
        }
    });

    chartDock->addAction(toggleFullscreen);
    // Настройка главного окна
    setWindowTitle("SDR Application with Fixed Layout");
    /////////////////////////////////////////

    ///////////////////////ТЕМА//////////////
    QMenu *themeMenu = menuBar->addMenu("&Theme");

    QAction *lightThemeAction = new QAction("Light", this);
    QAction *darkThemeAction = new QAction("Dark", this);
    QAction *customThemeAction = new QAction("Custom", this);

    lightThemeAction->setCheckable(true);
    darkThemeAction->setCheckable(true);
    customThemeAction->setCheckable(true);

    themeMenu->addAction(lightThemeAction);
    themeMenu->addAction(darkThemeAction);
    themeMenu->addAction(customThemeAction);

    QActionGroup *themeGroup = new QActionGroup(this);
    themeGroup->addAction(lightThemeAction);
    themeGroup->addAction(darkThemeAction);
    themeGroup->addAction(customThemeAction);
    themeGroup->setExclusive(true);

    lightThemeAction->setChecked(true);
    applyTheme("Light");

    connect(lightThemeAction, &QAction::triggered, this, [this]() { applyTheme("Light"); });
    connect(darkThemeAction, &QAction::triggered, this, [this]() { applyTheme("Dark"); });
    connect(customThemeAction, &QAction::triggered, this, [this]() { applyTheme("Custom"); });

    ///////////////////

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

std::vector<float> MainWindow::fftshift(const std::vector<float>& data) {
    size_t N = data.size();
    std::vector<float> shifted_data(N);
    for (size_t i = 0; i < N; ++i) {
        shifted_data[i] = data[(i + N / 2) % N];
    }
    return shifted_data;
}

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

    std::vector<float> lpdft_std(lpdft.begin(), lpdft.end());
    // Применение fftshift
    std::vector<float> shifted_lpdft = fftshift(lpdft_std);
     
     // Очистка текущих данных спектра
     spectrumSeries->clear();

    // Заполнение спектра новыми данными
    double freq_step = sampleRate_rx / shifted_lpdft.size(); // Шаг по частоте
    double start_freq = frequency_rx - sampleRate_rx / 2;          // Начальная частота
    for (size_t i = 0; i < shifted_lpdft.size(); ++i) {
        double freq = start_freq + i * freq_step; // Частота для текущего бина
        spectrumSeries->append(freq / 1e6, shifted_lpdft[i]); // Частота в МГц
    }

    // Обновление графика спектра
    QChart *spectrumChart = spectrumSeries->chart();
    if (spectrumChart) {
        spectrumChart->removeSeries(spectrumSeries);
        spectrumChart->addSeries(spectrumSeries);
        spectrumChart->createDefaultAxes();

        // // Обновление оси X
        QValueAxis *axisX = qobject_cast<QValueAxis*>(spectrumChart->axisX());
        QValueAxis *axisY = qobject_cast<QValueAxis*>(spectrumChart->axisY());

        // if (axisX) {
        //     double start_freq = (frequency_rx - sampleRate_rx / 2) / 1e6; // Начальная частота в МГц
        //     double end_freq = (frequency_rx + sampleRate_rx / 2) / 1e6;   // Конечная частота в МГц
        //     axisX->setRange(start_freq, end_freq);
            axisX->setTitleText("Frequency (MHz)");
            axisY->setTitleText("Power (dB)");

        // }
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
    updateSpectrum(data, size);
 
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
                                double bandwidthTX,
                                double gainRX,
                                double frequencyRX,
                                double sampleRateRX,
                                size_t* channels,
                                size_t channel_count,
                                double bandwidthRX) {
    
    // Применение настроек TX
    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], gainRX) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }

    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, frequencyTX, NULL) != 0) {
        qCritical() << "Ошибка установки частоты TX:" << SoapySDRDevice_lastError();
        return;
    }
    
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sampleRateTX) != 0) {
        qCritical() << "Ошибка установки скорости выборки TX:" << SoapySDRDevice_lastError();
        return;
    }
    
    if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_TX, 0, bandwidthTX) != 0) {
        qCritical() << "Ошибка установки полосы пропускания TX:" << SoapySDRDevice_lastError();
        return;
    }

   // Применение настроек RX
   
    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], gainRX) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }

    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, frequencyRX, NULL) != 0) {
        //qCritical() << "Ошибка установки частоты RX:" << SoapySDRDevice_lastError();
        return;
    }
   
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sampleRateRX) != 0) {
        qCritical() << "Ошибка установки скорости выборки RX:" << SoapySDRDevice_lastError();
        return;
    }

    if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_TX, 0, bandwidthTX) != 0) {
        qCritical() << "Ошибка установки полосы пропускания TX:" << SoapySDRDevice_lastError();
        return;
    }
   
    qDebug() << "Настройки SDR успешно применены.";

    // Обновление параметров SDR в MainWindow
        this->sampleRate_tx = sampleRateTX;
        this->sampleRate_rx = sampleRateRX;
        this->frequency_tx = frequencyTX;
        this->frequency_rx = frequencyRX;
        this->txGain = gainTX;
        this->rxGain = gainRX;
}

void MainWindow::applyTheme(const QString &theme) {
    if (theme == "Light") {
        qApp->setStyleSheet("");
        chart->setTheme(QChart::ChartThemeLight);
        spectrumChart->setTheme(QChart::ChartThemeLight);
        constellationChart->setTheme(QChart::ChartThemeLight);
    } else if (theme == "Dark") {
        qApp->setStyleSheet(
            "QWidget { background-color: #2E3440; color: #ECEFF4; }"
            "QMenuBar { background-color: #3B4252; color: #ECEFF4; }"
            "QMenuBar::item:selected { background-color: #4C566A; }"
            "QMenu { background-color: #3B4252; color: #ECEFF4; }"
            "QMenu::item:selected { background-color: #4C566A; }"
            "QDockWidget { background-color: #3B4252; color: #ECEFF4; }"
            "QToolBar { background-color: #3B4252; color: #ECEFF4; }"
            "QStatusBar { background-color: #3B4252; color: #ECEFF4; }"
            "QLineEdit { background-color: #4C566A; color: #ECEFF4; }"
            "QPushButton { background-color: #4C566A; color: #ECEFF4; }"
        );
        chart->setTheme(QChart::ChartThemeDark);
        spectrumChart->setTheme(QChart::ChartThemeDark);
        constellationChart->setTheme(QChart::ChartThemeDark);
    } else if (theme == "Custom") {
        qApp->setStyleSheet(
            "QWidget { background-color: #F0F0F0; color: #000000; }"
            "QMenuBar { background-color: #E0E0E0; color: #000000; }"
            "QMenuBar::item:selected { background-color: #C0C0C0; }"
            "QMenu { background-color: #E0E0E0; color: #000000; }"
            "QMenu::item:selected { background-color: #C0C0C0; }"
            "QDockWidget { background-color: #E0E0E0; color: #000000; }"
            "QToolBar { background-color: #E0E0E0; color: #000000; }"
            "QStatusBar { background-color: #E0E0E0; color: #000000; }"
            "QLineEdit { background-color: #FFFFFF; color: #000000; }"
            "QPushButton { background-color: #C0C0C0; color: #000000; }"
        );
        chart->setTheme(QChart::ChartThemeBlueCerulean);
        spectrumChart->setTheme(QChart::ChartThemeBlueCerulean);
        constellationChart->setTheme(QChart::ChartThemeBlueCerulean);
    }
}
