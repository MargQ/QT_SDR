#ifndef SDRWORKER_H
#define SDRWORKER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <vector>
#include <complex>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include "mainwindow.h"

class SdrWorker : public QObject {
    Q_OBJECT

public:
    explicit SdrWorker(SoapySDRDevice* sdr, 
                        size_t *channels, 
                        size_t channel_count,
                        QObject *parent = nullptr);

    ~SdrWorker();

public slots:
    void process();
    void generateTxData();

signals:
    void dataReady(const int16_t* data, size_t size);

private:
    SoapySDRDevice *sdr = nullptr;
    int samplesPerSymbol = 10;
    std::vector<size_t> channelList; // Список каналов
    // Данные для передачи
    std::vector<std::complex<int16_t>> tx_data;
    std::vector<int16_t> tx_buff;  // Буфер для передачи данных
    size_t tx_data_pos = 0;  // Текущая позиция в tx_data
    size_t tx_buffer_size = 0;  // Размер буфера передачи
    size_t iteration_count = 5;
    int flags;        // flags set by receive operation
    long long timeNs;

    SoapySDRStream *rxStream = nullptr;
    size_t rx_mtu = 0;

    SoapySDRStream *txStream = nullptr;
    size_t tx_mtu = 0;

    std::vector<int16_t> buffer;

    QTimer timer;

    bool initialized;
    QMutex mutex;
};

#endif // SDRWORKER_H

