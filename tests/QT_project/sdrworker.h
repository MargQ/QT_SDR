#ifndef SDRWORKER_H
#define SDRWORKER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
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

signals:
    void dataReady(const int16_t* data, size_t size);

private:
    SoapySDRDevice *sdr = nullptr;
    std::vector<size_t> channelList; // Список каналов
    double sampleRate;
    double frequency;
    double txGain;

    double sampleRate2;
    double frequency2;
    double rxGain;

    // SoapySDRStream *rxStream = nullptr;
    // size_t rx_mtu = 0;

    // SoapySDRStream *txStream = nullptr;
    // size_t tx_mtu = 0;
    //size_t tx_data_pos = 0;
    QTimer timer;
};

#endif // SDRWORKER_H

