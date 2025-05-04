#include <QApplication>
#include "sdrworker.h"
#include "mainwindow.h"
#include <QMetaType>
#include <QStyleFactory>

Q_DECLARE_METATYPE(std::vector<std::complex<int16_t>>)

int main(int argc, char *argv[]) {
    qRegisterMetaType<std::vector<std::complex<int16_t>>>("std::vector<std::complex<int16_t>>");
    // Регистрация типа std::vector<std::complex<int16_t>>
    qRegisterMetaType<std::vector<std::complex<int16_t>>>("std::vector<std::complex<int16_t>>");
    // Регистрация типа size_t
    qRegisterMetaType<size_t>("size_t");
    
    QApplication app(argc, argv);


    // Устанавливаем стиль Fusion
    app.setStyle(QStyleFactory::create("Imagine"));

    MainWindow mainWindow(
        sampleRate_tx,
        sampleRate_rx,
        frequency_tx,
        frequency_rx,
        txGain,
        rxGain,
        bandwidth_tx,
        bandwidth_rx,
        rxStream,
        txStream);
    
    mainWindow.show();
    return app.exec();
}
