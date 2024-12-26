#ifndef SDRWORKER_H
#define SDRWORKER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <vector>
#include <complex>
#include <iio/iio.h>
#include "mainwindow.h"

class SdrWorker : public QObject {
    Q_OBJECT

public:
    SdrWorker(struct iio_context* ctx, struct iio_device* rx_dev, struct iio_channel* rx0_i, struct iio_channel* rx0_q, struct iio_channels_mask *rxmask, struct iio_stream  *rxstream, struct iio_device* tx_dev, struct iio_channel* tx0_i, struct iio_channel* tx0_q, struct iio_channels_mask *txmask, struct iio_stream  *txstream, QObject* parent = nullptr);
    ~SdrWorker();

public slots:
    void process();

signals:
    void dataReady(const std::vector<std::complex<int16_t>>& data);

private:
    struct iio_context* ctx;
    struct iio_device* rx_dev;
    struct iio_device* tx_dev;
    struct iio_channel* rx0_i;
    struct iio_channel* tx0_i;
    struct iio_channel* rx0_q;
    struct iio_channel* tx0_q;
    struct iio_channels_mask* rxmask = NULL;
    struct iio_channels_mask* txmask = NULL;
    struct iio_stream  *rxstream = NULL;
    struct iio_stream  *txstream = NULL;
    QTimer timer;
};

#endif // SDRWORKER_H

