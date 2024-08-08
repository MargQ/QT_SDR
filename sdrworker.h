#ifndef SDRWORKER_H
#define SDRWORKER_H

#include <QObject>
#include <vector>
#include <complex>
#include <QDebug>
#include <QTimer>
#include "iio.h"

class SdrWorker : public QObject {
    Q_OBJECT

public:
    SdrWorker(struct iio_context* ctx, struct iio_device* rx, struct iio_channel* rx0_i, struct iio_channel* rx0_q, QObject* parent = nullptr);
    ~SdrWorker();

public slots:
    void process();

signals:
    void dataReady(const std::vector<std::complex<int16_t>>& data);

private:
    struct iio_context* ctx;
    struct iio_device* rx;
    struct iio_channel* rx0_i;
    struct iio_channel* rx0_q;
    QTimer timer;
};

#endif // SDRWORKER_H

