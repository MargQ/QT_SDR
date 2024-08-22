#include "sdrworker.h"
#include <iostream>
#include "sdr.h"
#include <QThread>
#include <vector>
#include <complex>
#include <iio.h>
#include <QDebug>


SdrWorker::SdrWorker(struct iio_context* ctx, struct iio_device* rx, struct iio_channel* rx0_i, struct iio_channel* rx0_q, QObject* parent)
    : QObject(parent), ctx(ctx), rx(rx), rx0_i(rx0_i), rx0_q(rx0_q) {
    connect(&timer, &QTimer::timeout, this, &SdrWorker::process);
    timer.start(180);  // Интервал
}

SdrWorker::~SdrWorker() { // Деструктор
}

void SdrWorker::process() {
    qDebug() << "SdrWorker::process called from thread:" << QThread::currentThread();
    //while (running) {
    std::vector<std::complex<int16_t>> data = getSdrData(ctx, rx, rx0_i, rx0_q);
    if (!data.empty()) {
        emit dataReady(data);
        }
   //}
}


