#ifndef SDRWORKER_H
#define SDRWORKER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <vector>
#include <complex>
#include <cstddef> // для size_t
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

extern size_t channels[1];
extern size_t channel_count;
extern SoapySDRDevice *sdr;

// Параметры SDR
extern double sampleRate_tx;
extern double sampleRate_rx;
extern double frequency_tx;
extern double frequency_rx;
extern double txGain;
extern double rxGain;
extern double bandwidth_tx;
extern double bandwidth_rx;
extern SoapySDRStream *rxStream;
extern SoapySDRStream *txStream;
extern size_t rx_mtu;
extern size_t tx_mtu;

class SdrWorker : public QObject {
    Q_OBJECT

public:
    explicit SdrWorker(
                        QObject *parent = nullptr);

    ~SdrWorker();

public slots:
    void process();
    void generateTxData();

signals:
    void dataReady(const int16_t* data, size_t size);

private:
    int samplesPerSymbol = 10;
     // Список каналов
    std::vector<size_t> channelList;
    // Данные для передачи
    std::vector<std::complex<int16_t>> tx_data;
    // Буфер для передачи данных
    std::vector<int16_t> tx_buff;
    // Текущая позиция в tx_data
    size_t tx_data_pos = 0;
    // Размер буфера передачи
    size_t tx_buffer_size = 0;
    
    size_t iteration_count = 5;
    int flags;
    long long timeNs;




    std::vector<int16_t> buffer;

    QTimer timer;

    bool initialized;
    QMutex mutex;

};

#endif // SDRWORKER_H

