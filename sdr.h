#ifndef SDR_H
#define SDR_H

#include <iostream>
#include <iio/iio.h>
#include <vector>
#include <complex>
#include "mainwindow.h"

struct iio_context* scan();
std::vector<std::complex<int16_t>> getSdrData(struct iio_context *ctx, struct iio_device *rx_dev, struct iio_channel *rx0_i, struct iio_channel *rx0_q,struct iio_channels_mask *rxmask, struct iio_stream  *rxstream);
void sendSdrData(struct iio_context *ctx, struct iio_device *tx_dev, struct iio_channel *tx0_i, struct iio_channel *tx0_q, struct iio_channels_mask *txmask, struct iio_stream  *txstream, const std::vector<std::complex<int16_t>>& tx_data);

#endif // SDR_H
