#include "mainwindow.h"
#include "sdrworker.h"
#include "fft.hpp"
#include <QMetaType>
#include <QDebug>
#include <QDoubleValidator>
#include <QMainWindow>  // Для QMainWindow
#include <QPointer>     // Для QPointer
#include <QToolButton>  // Для QToolButton


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
    QDockWidget *dockWidget_p = new QDockWidget("Параметры SDR", this);
    QWidget *dockWidgetContent = new QWidget(dockWidget_p);
    QFormLayout *formLayout = new QFormLayout(dockWidgetContent);

        // Фиксируем док-виджет слева
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget_p);

    // Запрещаем открепление и перемещение
    dockWidget_p->setFeatures(QDockWidget::DockWidgetClosable);  // Только закрытие

    // Фиксируем ширину док-виджета
    dockWidget_p->setFixedWidth(500);  // Ширина 300 пикселей
    
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
    QPushButton *applyButton = new QPushButton("Применить", this);
    
    formLayout->addRow("IP-адрес устройства:", ipAddressInput);

    formLayout->addRow("Усиление антенны (передача), дБ:", gainTX_Input);
    formLayout->addRow("Центральная частота (передача), Гц:", frequencyTX_Input);
    formLayout->addRow("Частота дискретизации (передача), Гц:", sampleRateTX_Input);
    formLayout->addRow("Ширина полосы частот (передача), Гц:", bandwidthTX_Input);

    formLayout->addRow("Усиление антенны (прием), дБ:", gainRX_Input);
    formLayout->addRow("Центральная частота (прием), Гц:", frequencyRX_Input);
    formLayout->addRow("Частота дискретизации (прием), Гц:", sampleRateRX_Input);
    formLayout->addRow("Ширина полосы частот (прием), Гц:", bandwidthRX_Input);
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
// Группа для управления режимами SDR
QGroupBox *sdrModeGroup = new QGroupBox("Режимы работы SDR", dockWidgetContent);
QVBoxLayout *modeLayout = new QVBoxLayout;

// SDR 1
QGroupBox *sdr1Group = new QGroupBox("SDR 1");
QHBoxLayout *sdr1Layout = new QHBoxLayout;

QCheckBox *sdr1RxCheck = new QCheckBox("RX (Прием)");
QCheckBox *sdr1TxCheck = new QCheckBox("TX (Передача)");
sdr1RxCheck->setChecked(true); // По умолчанию прием

sdr1Layout->addWidget(sdr1RxCheck);
sdr1Layout->addWidget(sdr1TxCheck);
sdr1Group->setLayout(sdr1Layout);

// SDR 2
QGroupBox *sdr2Group = new QGroupBox("SDR 2");
QHBoxLayout *sdr2Layout = new QHBoxLayout;

QCheckBox *sdr2RxCheck = new QCheckBox("RX (Прием)");
QCheckBox *sdr2TxCheck = new QCheckBox("TX (Передача)");
sdr2RxCheck->setChecked(true); // По умолчанию прием

sdr2Layout->addWidget(sdr2RxCheck);
sdr2Layout->addWidget(sdr2TxCheck);
sdr2Group->setLayout(sdr2Layout);

// Добавляем группы в основной layout
modeLayout->addWidget(sdr1Group);
modeLayout->addWidget(sdr2Group);
sdrModeGroup->setLayout(modeLayout);

// Добавляем группу режимов в основной layout формы
formLayout->insertRow(1, sdrModeGroup); // Вставляем после IP-адреса
///////////////////////////////////////////////////////////////////
    //setupUI();
    // Настройка графика
    realSeries->setName("Реальная часть");
    imagSeries->setName("Мнимая часть");
    chart->addSeries(realSeries);
    chart->addSeries(imagSeries);
    chart->createDefaultAxes();
    chart->axisY()->setRange(-250, 250);
    chart->axisX()->setRange(0, 1024);
    chart->setTitle("График отсчетов во времени");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Отсчеты");
    chart->setAxisX(axisX, realSeries);
    chart->setAxisX(axisX, imagSeries);

    QValueAxis *axisY_time = new QValueAxis();
    axisY_time->setTitleText("Амплитуда");
    chart->setAxisY(axisY_time, realSeries);
    chart->setAxisY(axisY_time, imagSeries);

    QChartView *chartView = new QChartView(chart); //////
    chartView->setRenderHint(QPainter::Antialiasing, false);
    chartView->setMinimumSize(500, 400);
    setCentralWidget(chartView);

    // Настройка графика для спектра
    spectrumChart = new QChart();
    spectrumSeries->setName("Спектр");
    spectrumChart->addSeries(spectrumSeries);

    // Создаем и настраиваем ось X для графика спектра
    // В конструкторе MainWindow
    axisX_spectrum = new QValueAxis();
    axisX_spectrum->setTitleText("Частота (МГц)");
    axisX_spectrum->setRange((frequency_rx - sampleRate_rx/2) / 1e6, (frequency_rx + sampleRate_rx/2) / 1e6);

    spectrumChart->addAxis(axisX_spectrum, Qt::AlignBottom);
    spectrumSeries->attachAxis(axisX_spectrum);

    QValueAxis *axisY_spectrum = new QValueAxis();
    axisY_spectrum->setTitleText("Мощность (дБ)");
    axisY_spectrum->setRange(-100, 0); // Диапазон мощности в dB
    spectrumChart->addAxis(axisY_spectrum, Qt::AlignLeft);
    spectrumSeries->attachAxis(axisY_spectrum);

    spectrumChart->setTitle("График частотной области сигнала");

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
    constellationSeries->setName("Созвездие");
    constellationSeries->setMarkerSize(7.0); // Размер точек

    // Настройка графика для диаграммы созвездия
    constellationChart = new QChart();
    constellationChart->addSeries(constellationSeries);
    constellationChart->createDefaultAxes();
    constellationChart->setTitle("Диаграмма созвездия");

    // Настроим оси X и Y для отображения созвездия
    QValueAxis *constAxisX = new QValueAxis();
    constAxisX->setRange(-1, 1); // Диапазон значений по X
    constAxisX->setTitleText("Синфазная компонента");
    constellationChart->setAxisX(constAxisX, constellationSeries);

    QValueAxis *constAxisY = new QValueAxis();
    constAxisY->setRange(-1, 1); // Диапазон значений по Y
    constAxisY->setTitleText("Квадратурная компонента");
    constellationChart->setAxisY(constAxisY, constellationSeries);

    // Виджет для отображения диаграммы созвездия
    QChartView *constellationView = new QChartView(constellationChart);
    constellationView->setRenderHint(QPainter::Antialiasing, false);
    constellationView->setMinimumSize(400, 400);
    constellationView->setFixedSize(400, 400);

    // Добавим виджет созвездия в layout
    layout->addWidget(constellationView, 0, Qt::AlignLeft);

    //////COSTAS LOOP
    // Добавление диаграммы созвездия
    constellationCLSeries = new QScatterSeries();
    constellationCLSeries->setName("Созвездие после Costas Loop");
    constellationCLSeries->setMarkerSize(7.0); // Размер точек

    // Настройка графика для диаграммы созвездия после Costas Loop(CL)
    constellationCLChart = new QChart();
    constellationCLChart->addSeries(constellationCLSeries);
    constellationCLChart->createDefaultAxes();
    constellationCLChart->setTitle("Диаграмма созвездия");

    // Настроим оси X и Y для отображения созвездия
    QValueAxis *constAxisX_CL = new QValueAxis();
    constAxisX_CL->setRange(-1, 1); // Диапазон значений по X
    constAxisX_CL->setTitleText("Синфазная компонента");
    constellationCLChart->setAxisX(constAxisX_CL, constellationCLSeries);

    QValueAxis *constAxisY_CL = new QValueAxis();
    constAxisY_CL->setRange(-1, 1); // Диапазон значений по Y
    constAxisY_CL->setTitleText("Квадратурная компонента");
    constellationCLChart->setAxisY(constAxisY_CL, constellationCLSeries);

    // Виджет для отображения диаграммы созвездия
    QChartView *constellationViewCL = new QChartView(constellationCLChart);
    constellationViewCL->setRenderHint(QPainter::Antialiasing, false);
    constellationViewCL->setMinimumSize(400, 400);
    constellationViewCL->setFixedSize(400, 400);

    // Добавим виджет созвездия в layout
    layout->addWidget(constellationViewCL);

    ////////////////////

    // EYEDIAGRAM///
    eyeDiagramSeries = new QLineSeries();
    eyeDiagramSeries->setName("Глазковая диаграмма");
    //eyeDiagramSeries->setMarkerSize(7.0); // Размер точек

    // Настройка графика для диаграммы созвездия после Costas Loop(CL)
    eyeDiagramChart = new QChart();
    eyeDiagramChart->addSeries(eyeDiagramSeries);
    eyeDiagramChart->createDefaultAxes();
    eyeDiagramChart->setTitle("Глазковая диаграмма");

    // Настроим оси X и Y для отображения созвездия
    QValueAxis *AxisX_ED = new QValueAxis();
    AxisX_ED->setRange(-15, 15); // Диапазон значений по X
    AxisX_ED->setTitleText("Отсчеты");
    eyeDiagramChart->setAxisX(AxisX_ED, eyeDiagramSeries);

    QValueAxis *AxisY_ED = new QValueAxis();
    AxisY_ED->setRange(-300, 300); // Диапазон значений по Y
    AxisY_ED->setTitleText("Амплитуда");
    eyeDiagramChart->setAxisY(AxisY_ED, eyeDiagramSeries);

    // Виджет для отображения диаграммы созвездия
    eyeDiagramView = new QChartView(eyeDiagramChart);
    eyeDiagramView->setRenderHint(QPainter::Antialiasing, false);
    eyeDiagramView->setMinimumSize(400, 400);
    eyeDiagramView->setFixedSize(400, 400);
    eyeDiagramView->setRenderHint(QPainter::Antialiasing, true);
    ////////

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
    QMenu *fileMenu = menuBar->addMenu("&Файл");
    QMenu *viewMenu = menuBar->addMenu("&Вид");

    QAction *exitAction = new QAction("&Выход", this);
    fileMenu->addAction(exitAction);

    // QAction *toggleDockAction = new QAction("&Toggle Dock", this);
    // toggleDockAction->setCheckable(true);
    // toggleDockAction->setChecked(true);
    // viewMenu->addAction(toggleDockAction);

    // Создание тулбара
    QToolBar *toolBar = new QToolBar("Main Toolbar", this);
    toolBar->addAction(exitAction);
    //toolBar->addAction(toggleDockAction);
    addToolBar(toolBar);

    // Добавление подменю для управления функциями док-виджетов
    QMenu *ChartsMenu = menuBar->addMenu("Графики");

    // Добавление действий для управления функциями
    QMenu *movableMenu = ChartsMenu->addMenu("Movable Feature");
    QMenu *floatableMenu = ChartsMenu->addMenu("Floatable Feature");
    QMenu *closableMenu = ChartsMenu->addMenu("Closable Feature");

    SpectrumMovableAction = new QAction("Spectrum Chart", this);
    TimeMovableAction = new QAction("Time Chart", this);
    ConstellationMovableAction = new QAction("Constellation Chart", this);
    ConstellationCLMovableAction = new QAction("Constellation Costas Loop Chart", this);
    EyeDiagramMovableAction = new QAction("Eye Diagram", this);

    SpectrumFloatableAction = new QAction("Spectrum Chart", this);
    TimeFloatableAction = new QAction("Time Chart", this);
    ConstellationFloatableAction = new QAction("Constellation Chart", this);
    ConstellationCLFloatableAction = new QAction("Constellation Costas Loop Chart", this);
    EyeDiagramFloatableAction = new QAction("Eye Diagram", this);

    SpectrumClosableAction = new QAction("Spectrum Chart", this);
    TimeClosableAction = new QAction("Time Chart", this);
    ConstellationClosableAction = new QAction("Constellation Chart", this);
    ConstellationCLClosableAction = new QAction("Constellation Costas Loop Chart", this);
    EyeDiagramClosableAction = new QAction("Eye Diagram", this);

    SpectrumMovableAction->setCheckable(true);
    SpectrumFloatableAction->setCheckable(true);
    SpectrumMovableAction->setChecked(true);
    SpectrumFloatableAction->setChecked(true);
    SpectrumClosableAction->setCheckable(true);
    SpectrumClosableAction->setChecked(true);

    TimeMovableAction->setCheckable(true);
    TimeMovableAction->setChecked(true);
    TimeFloatableAction->setCheckable(true);
    TimeFloatableAction->setChecked(true);
    TimeClosableAction->setCheckable(true);
    TimeClosableAction->setChecked(true);

    ConstellationMovableAction->setCheckable(true);
    ConstellationMovableAction->setChecked(true);
    ConstellationFloatableAction->setCheckable(true);
    ConstellationFloatableAction->setChecked(true);
    ConstellationClosableAction->setCheckable(true);
    ConstellationClosableAction->setChecked(true);

    ConstellationCLMovableAction->setCheckable(true);
    ConstellationCLMovableAction->setChecked(true);
    ConstellationCLFloatableAction->setCheckable(true);
    ConstellationCLFloatableAction->setChecked(true);
    ConstellationCLClosableAction->setCheckable(true);
    ConstellationCLClosableAction->setChecked(true);

    EyeDiagramMovableAction->setCheckable(true);
    EyeDiagramMovableAction->setChecked(true);
    EyeDiagramFloatableAction->setCheckable(true);
    EyeDiagramFloatableAction->setChecked(true);
    EyeDiagramClosableAction->setCheckable(true);
    EyeDiagramClosableAction->setChecked(true);


    movableMenu->addAction(SpectrumMovableAction);
    movableMenu->addAction(TimeMovableAction);
    movableMenu->addAction(ConstellationMovableAction);
    movableMenu->addAction(ConstellationCLMovableAction);
    movableMenu->addAction(EyeDiagramMovableAction);

    floatableMenu->addAction(SpectrumFloatableAction);
    floatableMenu->addAction(TimeFloatableAction);
    floatableMenu->addAction(ConstellationFloatableAction);
    floatableMenu->addAction(ConstellationCLFloatableAction);
    floatableMenu->addAction(EyeDiagramFloatableAction);

    closableMenu->addAction(SpectrumClosableAction);
    closableMenu->addAction(TimeClosableAction);
    closableMenu->addAction(ConstellationClosableAction);
    closableMenu->addAction(ConstellationCLClosableAction);
    closableMenu->addAction(EyeDiagramClosableAction);

    // Подключение действий к слотам
    connect(SpectrumMovableAction, &QAction::toggled, this, &MainWindow::toggleSpectrumMovable);
    connect(TimeMovableAction, &QAction::toggled, this, &MainWindow::toggleTimeMovable);
    connect(ConstellationMovableAction, &QAction::toggled, this, &MainWindow::toggleConstellationMovable);
    connect(ConstellationCLMovableAction, &QAction::toggled, this, &MainWindow::toggleConstellationCLMovable);
    connect(EyeDiagramMovableAction, &QAction::toggled, this, &MainWindow::toggleEyeDiagramMovable);

    connect(SpectrumFloatableAction, &QAction::toggled, this, &MainWindow::toggleSpectrumFloatable);
    connect(TimeFloatableAction, &QAction::toggled, this, &MainWindow::toggleTimeFloatable);
    connect(ConstellationFloatableAction, &QAction::toggled, this, &MainWindow::toggleConstellationFloatable);
    connect(ConstellationCLFloatableAction, &QAction::toggled, this, &MainWindow::toggleConstellationCLFloatable);
    connect(EyeDiagramFloatableAction, &QAction::toggled, this, &MainWindow::toggleEyeDiagramFloatable);

    connect(SpectrumClosableAction, &QAction::toggled, this, &MainWindow::toggleSpectrumClosable);
    connect(TimeClosableAction, &QAction::toggled, this, &MainWindow::toggleTimeClosable);
    connect(ConstellationClosableAction, &QAction::toggled, this, &MainWindow::toggleConstellationClosable);
    connect(ConstellationCLClosableAction, &QAction::toggled, this, &MainWindow::toggleConstellationCLClosable);
    connect(EyeDiagramClosableAction, &QAction::toggled, this, &MainWindow::toggleEyeDiagramClosable);




    ////ВЗАИМОДЕЙСТВИЕ С ГРАФИКАМИ//////////////

    // Создаем док-виджеты для каждого графика
    chartDock = new QDockWidget(this);
    spectrumDock = new QDockWidget(this);
    constellationDock = new QDockWidget(this);
    constellationCLDock = new QDockWidget(this);
    eyeDiagramDock = new QDockWidget(this);

    // Добавляем QChartView в док-виджеты
    chartDock->setWidget(chartView);
    spectrumDock->setWidget(spectrumView);
    constellationDock->setWidget(constellationView);
    constellationCLDock->setWidget(constellationViewCL);
    eyeDiagramDock->setWidget(eyeDiagramView);
    eyeDiagramView->setRenderHint(QPainter::Antialiasing);


    // Задаем начальное расположение графиков
    addDockWidget(Qt::RightDockWidgetArea, spectrumDock);          // Временной график сверху
    splitDockWidget(spectrumDock, chartDock, Qt::Vertical);   // Спектр под временным графиком
    splitDockWidget(chartDock, constellationDock, Qt::Horizontal);  // Созвездие справа от спектра
    splitDockWidget(constellationDock, constellationCLDock, Qt::Horizontal);
    splitDockWidget(constellationCLDock, eyeDiagramDock, Qt::Horizontal);

    // Разрешаем открепление, перемещение и сворачивание
    chartDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    spectrumDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    constellationDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    constellationCLDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    eyeDiagramDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);



    // Настройка пропорций
    QList<QDockWidget*> leftDocks;
    leftDocks << dockWidget_p;
    resizeDocks(leftDocks, {300}, Qt::Horizontal);  // Ширина левой области

    QList<QDockWidget*> rightDocks;
    rightDocks << chartDock << spectrumDock << constellationDock << constellationCLDock << eyeDiagramDock;
    resizeDocks(rightDocks, {600, 300, 200, 200, 200}, Qt::Vertical);  // Настройка пропорции графиков
    // Настройка главного окна
    setWindowTitle("Анализатор спектра");
    /////////////////////////////////////////

    ///////////////////////ТЕМА//////////////
    QMenu *themeMenu = menuBar->addMenu("&Тема");

    lightThemeAction = new QAction("Light", this);
    darkThemeAction = new QAction("Dark", this);
    customThemeAction = new QAction("Custom", this);

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

    darkThemeAction->setChecked(true);
    currentTheme = "Dark"; // Сохраняем выбранную тему
    applyTheme(currentTheme);
    ///////////////////////////////
    loadSettings();

    // Создание кнопки Save Settings
    QPushButton *saveSettingsButton = new QPushButton("Сохранить настройки", this);
    formLayout->addRow(saveSettingsButton);

    connect(saveSettingsButton, &QPushButton::clicked, this, &MainWindow::saveSettings);

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
// Сохранение настроек при закрытии приложения
void MainWindow::closeEvent(QCloseEvent *event) {
    saveSettings();
    // Подтверждение закрытие окна
    event->accept();
}

// // Метод создает интерфейс
// void MainWindow::setupUI() {
//     QVBoxLayout *layout = new QVBoxLayout;
//     layout->addWidget(chartView);
//     QWidget *centralWidget = new QWidget(this);
//     centralWidget->setLayout(layout);
//     setCentralWidget(centralWidget);
// }

void MainWindow::saveSettings() {
    QSettings settings("Me", "SDRApp");
    qDebug() << "Saving settings...";
    // Сохраняем тему
    settings.setValue("Theme", currentTheme);
    qDebug() << "Saved theme:" << currentTheme;

    // // Сохраняем параметры SDR
    // settings.setValue("SDR/IPAddress", ipAddressInput->text());
    // settings.setValue("SDR/GainTX", gainTX_Input->text());
    // settings.setValue("SDR/FrequencyTX", frequencyTX_Input->text());
    // settings.setValue("SDR/SampleRateTX", sampleRateTX_Input->text());
    // settings.setValue("SDR/BandwidthTX", bandwidthTX_Input->text());
    // settings.setValue("SDR/GainRX", gainRX_Input->text());
    // settings.setValue("SDR/FrequencyRX", frequencyRX_Input->text());
    // settings.setValue("SDR/SampleRateRX", sampleRateRX_Input->text());
    // settings.setValue("SDR/BandwidthRX", bandwidthRX_Input->text());

    // Сохранение настройки графиков
    settings.setValue("spectrumDock/movable", SpectrumMovableAction->isChecked());
    settings.setValue("spectrumDock/floatable", SpectrumFloatableAction->isChecked());
    settings.setValue("spectrumDock/closable", SpectrumClosableAction->isChecked());

    settings.setValue("chartDock/movable", TimeMovableAction->isChecked());
    settings.setValue("chartDock/floatable", TimeFloatableAction->isChecked());
    settings.setValue("chartDock/closable", TimeClosableAction->isChecked());

    settings.setValue("constellationDock/movable", ConstellationMovableAction->isChecked());
    settings.setValue("constellationDock/floatable", ConstellationFloatableAction->isChecked());
    settings.setValue("constellationDock/closable", ConstellationClosableAction->isChecked());

    settings.setValue("constellationCLDock/movable", ConstellationCLMovableAction->isChecked());
    settings.setValue("constellationCLDock/floatable", ConstellationCLFloatableAction->isChecked());
    settings.setValue("constellationCLDock/closable", ConstellationCLClosableAction->isChecked());

    settings.setValue("eyeDiagramDock/movable", EyeDiagramMovableAction->isChecked());
    settings.setValue("eyeDiagramDock/floatable", EyeDiagramFloatableAction->isChecked());
    settings.setValue("eyeDiagramDock/closable", EyeDiagramClosableAction->isChecked()); 

    qDebug() << "Settings saved successfully";
    statusBar()->showMessage("Settings saved successfully");

}

void MainWindow::loadSettings() {
    QSettings settings("Me", "SDRApp");
    qDebug() << "Loading settings...";

    // Загрузка темы
    QString theme = settings.value("Theme", currentTheme).toString();
    applyTheme(theme);

    qDebug() << "Loaded theme:" << currentTheme;

    // Обновление состояний действий для темы
    if (theme == "Light") {
        lightThemeAction->setChecked(true);
    } else if (theme == "Dark") {
        darkThemeAction->setChecked(true);
    } else if (theme == "Custom") {
        customThemeAction->setChecked(true);
    }
    // // Загрузка параметров SDR
    // ipAddressInput->setText(settings.value("ipAddress", "192.168.2.1").toString());
    // gainTX_Input->setText(QString::number(settings.value("gainTX", 0.0).toDouble()));
    // frequencyTX_Input->setText(QString::number(settings.value("frequencyTX", 0.0).toDouble(), 'g', 10));
    // sampleRateTX_Input->setText(QString::number(settings.value("sampleRateTX", 0.0).toDouble(), 'g', 10));
    // bandwidthTX_Input->setText(QString::number(settings.value("bandwidthTX", 0.0).toDouble(), 'g', 10));
    // gainRX_Input->setText(QString::number(settings.value("gainRX", 0.0).toDouble()));
    // frequencyRX_Input->setText(QString::number(settings.value("frequencyRX", 0.0).toDouble(), 'g', 10));
    // sampleRateRX_Input->setText(QString::number(settings.value("sampleRateRX", 0.0).toDouble(), 'g', 10));
    // bandwidthRX_Input->setText(QString::number(settings.value("bandwidthRX", 0.0).toDouble(), 'g', 10));

    // // Применение настроек SDR
    // applySdrSettings(sdr,
    //                  ipAddressInput->text().toStdString(),
    //                  gainTX_Input->text().toDouble(),
    //                  frequencyTX_Input->text().toDouble(),
    //                  sampleRateTX_Input->text().toDouble(),
    //                  bandwidthTX_Input->text().toDouble(),
    //                  gainRX_Input->text().toDouble(),
    //                  frequencyRX_Input->text().toDouble(),
    //                  sampleRateRX_Input->text().toDouble(),
    //                  channels,
    //                  channel_count,
                    //  bandwidthRX_Input->text().toDouble());

    // Загрузка настроек графиков
    QDockWidget::DockWidgetFeatures spectrumFeatures = 0;
    if (settings.value("spectrumDock/movable", true).toBool()) {
        spectrumFeatures |= QDockWidget::DockWidgetMovable;
        SpectrumMovableAction->setChecked(true); // Обновляем состояние действия
    } else {
        SpectrumMovableAction->setChecked(false);
    }
    if (settings.value("spectrumDock/floatable", true).toBool()) {
        spectrumFeatures |= QDockWidget::DockWidgetFloatable;
        SpectrumFloatableAction->setChecked(true); // Обновляем состояние действия
    } else {
        SpectrumFloatableAction->setChecked(false);
    }
    if (settings.value("spectrumDock/closable", true).toBool()) {
        spectrumFeatures |= QDockWidget::DockWidgetClosable;
        SpectrumClosableAction->setChecked(true); // Обновляем состояние действия
    } else {
        SpectrumClosableAction->setChecked(false);
    }
    spectrumDock->setFeatures(spectrumFeatures);

    QDockWidget::DockWidgetFeatures chartFeatures = 0;
    if (settings.value("chartDock/movable", true).toBool()) {
        chartFeatures |= QDockWidget::DockWidgetMovable;
        TimeMovableAction->setChecked(true); // Обновляем состояние действия
    } else {
        TimeMovableAction->setChecked(false);
    }
    if (settings.value("chartDock/floatable", true).toBool()) {
        chartFeatures |= QDockWidget::DockWidgetFloatable;
        TimeFloatableAction->setChecked(true); // Обновляем состояние действия
    } else {
        TimeFloatableAction->setChecked(false);
    }
    if (settings.value("chartDock/closable", true).toBool()) {
        chartFeatures |= QDockWidget::DockWidgetClosable;
        TimeClosableAction->setChecked(true); // Обновляем состояние действия
    } else {
        TimeClosableAction->setChecked(false);
    }
    chartDock->setFeatures(chartFeatures);

    QDockWidget::DockWidgetFeatures constellationFeatures = 0;
    if (settings.value("constellationDock/movable", true).toBool()) {
        constellationFeatures |= QDockWidget::DockWidgetMovable;
        ConstellationMovableAction->setChecked(true); // Обновляем состояние действия
    } else {
        ConstellationMovableAction->setChecked(false);
    }
    if (settings.value("constellationDock/floatable", true).toBool()) {
        constellationFeatures |= QDockWidget::DockWidgetFloatable;
        ConstellationFloatableAction->setChecked(true); // Обновляем состояние действия
    } else {
        ConstellationFloatableAction->setChecked(false);
    }
    if (settings.value("constellationDock/closable", true).toBool()) {
        constellationFeatures |= QDockWidget::DockWidgetClosable;
        ConstellationClosableAction->setChecked(true); // Обновляем состояние действия
    } else {
        ConstellationClosableAction->setChecked(false);
    }
    constellationDock->setFeatures(constellationFeatures);

    QDockWidget::DockWidgetFeatures constellationCLFeatures = 0;
    if (settings.value("constellationCLDock/movable", true).toBool()) {
        constellationCLFeatures |= QDockWidget::DockWidgetMovable;
        ConstellationCLMovableAction->setChecked(true); // Обновляем состояние действия
    } else {
        ConstellationCLMovableAction->setChecked(false);
    }
    if (settings.value("constellationCLDock/floatable", true).toBool()) {
        constellationCLFeatures |= QDockWidget::DockWidgetFloatable;
        ConstellationCLFloatableAction->setChecked(true); // Обновляем состояние действия
    } else {
        ConstellationCLFloatableAction->setChecked(false);
    }
    if (settings.value("constellationCLDock/closable", true).toBool()) {
        constellationCLFeatures |= QDockWidget::DockWidgetClosable;
        ConstellationCLClosableAction->setChecked(true); // Обновляем состояние действия
    } else {
        ConstellationCLClosableAction->setChecked(false);
    }
    constellationCLDock->setFeatures(constellationCLFeatures);

    QDockWidget::DockWidgetFeatures eyeDiagramFeatures = 0;
    if (settings.value("eyeDiagramDock/movable", true).toBool()) {
        eyeDiagramFeatures |= QDockWidget::DockWidgetMovable;
        EyeDiagramMovableAction->setChecked(true); // Обновляем состояние действия
    } else {
        EyeDiagramMovableAction->setChecked(false);
    }
    if (settings.value("eyeDiagramDock/floatable", true).toBool()) {
        eyeDiagramFeatures |= QDockWidget::DockWidgetFloatable;
        EyeDiagramFloatableAction->setChecked(true); // Обновляем состояние действия
    } else {
        EyeDiagramFloatableAction->setChecked(false);
    }
    if (settings.value("eyeDiagramDock/closable", true).toBool()) {
        eyeDiagramFeatures |= QDockWidget::DockWidgetClosable;
        EyeDiagramClosableAction->setChecked(true); // Обновляем состояние действия
    } else {
        EyeDiagramClosableAction->setChecked(false);
    }
    eyeDiagramDock->setFeatures(eyeDiagramFeatures);
    qDebug() << "Settings loaded successfully";

    statusBar()->showMessage("Настройки успешно загружены");
}

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
        axisX->setTitleText("Частота (МГц)");
        axisY->setTitleText("Мощность (дБ)");
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

        constellationSeries->append(I*3, Q*3); // Сохраняем значения в серии

        // Заполнение буферов
        m_realBuffer[i].setX(i);
        m_realBuffer[i].setY(I * 1000); // Масштабирование по необходимости
        m_imagBuffer[i].setX(i);
        m_imagBuffer[i].setY(Q * 1000); // Масштабирование по необходимости
    }
    
    std::vector<double> filter(10, 1.0);  // Прямоугольный фильтр
    std::vector<double> filter_srrc = GenerateSRRC(syms, beta, P);

    // Обработка Costas Loop с отфильтрованными данными
    m_costasLoop.process(data, size);  // Передаем отфильтрованные данные
    
    // Обновление графика
    constellationCLSeries->clear();
    for (const auto& sample : m_costasLoop.getOutput()) {
        constellationCLSeries->append(sample.real()*3, sample.imag()*3);
    }

    // Получение обработанных данных
    const std::vector<std::complex<double>>& processed = m_costasLoop.getOutput();

    std::vector<std::complex<double>> filtered_signal = ConvolveSame(processed, filter_srrc);


    // FILE* file = fopen("TED.pcm", "wb");
    // if (!file) {
    //     perror("Ошибка открытия файла");
    // } else {
    //     for (const auto& sample : processed) {
    //         int16_t real = static_cast<int16_t>(std::clamp(sample.real() * 32767.0, -32768.0, 32767.0));
    //         int16_t imag = static_cast<int16_t>(std::clamp(sample.imag() * 32767.0, -32768.0, 32767.0));

    //         fwrite(&real, sizeof(int16_t), 1, file);
    //         fwrite(&imag, sizeof(int16_t), 1, file);
    //     }
    // }
    // fclose(file);

    // // Генерация QPSK с 10 отсчётами на символ
    // std::vector<std::complex<double>> ideal_qpsk;
    // for (int i = 0; i < 1000; ++i) {
    //     double phase = (i % 4) * M_PI/2 + M_PI/4; // 45, 135, 225, 315 градусов
    //     for (int j = 0; j < 10; ++j) {
    //         ideal_qpsk.push_back(std::complex<double>(cos(phase), sin(phase)));
    //     }
    // }

    // Вызов TED
    // Символьная синхронизация
    std::vector<int> tedIndices = TED(filtered_signal);

    //// Выборка индексов для идеальных данных (вместо TED)
    // std::vector<int> tedIndices;
    // for (int i = 0; i < ideal_qpsk.size(); i += 10) {
    //     tedIndices.push_back(i + 10 / 2);  // по центру каждого символа
    // }

        // Построение глазковой диаграммы из обработанных данных
    updateEyeDiagram(filtered_signal, tedIndices);

    // Обновление данных в графиках
    realSeries->replace(m_realBuffer);
    imagSeries->replace(m_imagBuffer);
    updateSpectrum(data, size);
 
}

// Вспомогательная функция для преобразования данных в комплексный формат
std::vector<std::complex<double>> MainWindow::convertToComplex(const int16_t* data, size_t size) {
    std::vector<std::complex<double>> complexData(size / 2);
    for (size_t i = 0; i < size / 2; ++i) {
        double real = data[2 * i] / 2048.0;
        double imag = data[2 * i + 1] / 2048.0;
        complexData[i] = std::complex<double>(real, imag);
    }
    return complexData;
}

void MainWindow::updateEyeDiagram(const std::vector<std::complex<double>>& signal, 
    const std::vector<int>& tedIndices) {
    const int samplesPerSymbol = 10;
    const int windowSize = 2 * samplesPerSymbol;
    const int halfWindow = windowSize / 2;

    if (signal.empty() || tedIndices.empty()) {
        qWarning() << "Empty input data!";
        return;
    }

    // Получение текущего чарта
    QChart* chart = eyeDiagramView->chart();
    if (!chart) {
        chart = new QChart();
        eyeDiagramView->setChart(chart);
    }

    // Сохранение текущей темы
    QChart::ChartTheme currentTheme = chart->theme();

    // Очищение серии
    chart->removeAllSeries();
    for (auto axis : chart->axes()) {
        chart->removeAxis(axis);
        delete axis;
    }

    // Настройки чарта
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->legend()->hide();

    // Масштабирование
    double maxAmplitude = 1e-6;
    for (int idx : tedIndices) {
        if (idx >= halfWindow && idx + halfWindow < static_cast<int>(signal.size())) {
            for (int j = -halfWindow; j < halfWindow; ++j) {
                maxAmplitude = std::max(maxAmplitude, std::abs(signal[idx+j].real()));
            }
        }
    }
    const double scale = (maxAmplitude > 0) ? 0.8/maxAmplitude : 1.0;

    // Создание новых осей
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Добавление данных
    const int maxTraces = qMin(50, static_cast<int>(tedIndices.size()));
    for (int i = 0; i < maxTraces; ++i) {
        int centerIdx = tedIndices[i];
        if (centerIdx < halfWindow || centerIdx + halfWindow >= static_cast<int>(signal.size())) 
            continue;

    QLineSeries *trace = new QSplineSeries();
    for (int j = -halfWindow; j < halfWindow; ++j) {
        int idx = centerIdx + j;
        double time = static_cast<double>(j) / samplesPerSymbol;
        double value = signal[idx].real() * scale;
        trace->append(time, value);
    }

    chart->addSeries(trace);
    trace->attachAxis(axisX);
    trace->attachAxis(axisY);
    }

    // Восстановление темы
    chart->setTheme(currentTheme);

    // Настройка осей и заголовка
    axisX->setRange(-1, 1);
    axisX->setTitleText("Символы");
    axisY->setRange(-1, 1);
    axisY->setTitleText("Амплитуда (I)");
    chart->setTitle("Глазковая диаграмма");
    // Сглаживание
    eyeDiagramView->setRenderHint(QPainter::Antialiasing);
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
        constellationCLChart->setTheme(QChart::ChartThemeLight);
        eyeDiagramView->chart()->setTheme(QChart::ChartThemeLight);        // tempChart->setTheme(QChart::ChartThemeLight);
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
        constellationCLChart->setTheme(QChart::ChartThemeDark);
        eyeDiagramView->chart()->setTheme(QChart::ChartThemeDark);        // tempChart->setTheme(QChart::ChartThemeDark);
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
        constellationCLChart->setTheme(QChart::ChartThemeBlueCerulean);
        eyeDiagramView->chart()->setTheme(QChart::ChartThemeBlueCerulean);        // tempChart->setTheme(QChart::ChartThemeBlueCerulean);
    }

    currentTheme = theme;
}

    // Слоты для управления функциями док-виджетов
void MainWindow::toggleSpectrumMovable(bool enabled) {
    setDockFeatures(spectrumDock, QDockWidget::DockWidgetMovable, enabled);
}

void MainWindow::toggleTimeMovable(bool enabled) {
    setDockFeatures(chartDock, QDockWidget::DockWidgetMovable, enabled);
}

void MainWindow::toggleConstellationMovable(bool enabled) {
    setDockFeatures(constellationDock, QDockWidget::DockWidgetMovable, enabled);
}

void MainWindow::toggleConstellationCLMovable(bool enabled) {
    setDockFeatures(constellationCLDock, QDockWidget::DockWidgetMovable, enabled);
}

void MainWindow::toggleEyeDiagramMovable(bool enabled) {
    setDockFeatures(eyeDiagramDock, QDockWidget::DockWidgetMovable, enabled);
}

void MainWindow::toggleSpectrumFloatable(bool enabled) {
    setDockFeatures(spectrumDock, QDockWidget::DockWidgetFloatable, enabled);
}

void MainWindow::toggleTimeFloatable(bool enabled) {
    setDockFeatures(chartDock, QDockWidget::DockWidgetFloatable, enabled);
}

void MainWindow::toggleConstellationFloatable(bool enabled) {
    setDockFeatures(constellationDock, QDockWidget::DockWidgetFloatable, enabled);
}

void MainWindow::toggleConstellationCLFloatable(bool enabled) {
    setDockFeatures(constellationCLDock, QDockWidget::DockWidgetFloatable, enabled);
}

void MainWindow::toggleEyeDiagramFloatable(bool enabled) {
    setDockFeatures(eyeDiagramDock, QDockWidget::DockWidgetFloatable, enabled);
}

void MainWindow::toggleSpectrumClosable(bool enabled) {
    setDockFeatures(spectrumDock, QDockWidget::DockWidgetClosable, enabled);
}

void MainWindow::toggleTimeClosable(bool enabled) {
    setDockFeatures(chartDock, QDockWidget::DockWidgetClosable, enabled);
}

void MainWindow::toggleConstellationClosable(bool enabled) {
    setDockFeatures(constellationDock, QDockWidget::DockWidgetClosable, enabled);
}

void MainWindow::toggleConstellationCLClosable(bool enabled) {
    setDockFeatures(constellationCLDock, QDockWidget::DockWidgetClosable, enabled);
}

void MainWindow::toggleEyeDiagramClosable(bool enabled) {
    setDockFeatures(eyeDiagramDock, QDockWidget::DockWidgetClosable, enabled);
}

// Функция для установки/снятия флагов функций док-виджета
void MainWindow::setDockFeatures(QDockWidget *dock, QDockWidget::DockWidgetFeature feature, bool enabled) {
    QDockWidget::DockWidgetFeatures features = dock->features();
    if (enabled) {
        features |= feature; // Включение функции
    } else {
        features &= ~feature; // Отключение функцию
    }
    dock->setFeatures(features);
}





