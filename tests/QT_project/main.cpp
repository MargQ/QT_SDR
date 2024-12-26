#include <QApplication>
#include "mainwindow.h"
#include <QMetaType>

Q_DECLARE_METATYPE(std::vector<std::complex<int16_t>>)

int main(int argc, char *argv[]) {
    qRegisterMetaType<std::vector<std::complex<int16_t>>>("std::vector<std::complex<int16_t>>");
    
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
