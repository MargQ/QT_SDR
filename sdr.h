#ifndef SDR_H
#define SDR_H

#include <iio.h>
#include <vector>
#include <complex>

struct iio_context* scan();
std::vector<std::complex<int16_t>> getSdrData(struct iio_context *ctx, struct iio_device *rx, struct iio_channel *rx0_i, struct iio_channel *rx0_q);

#endif // SDR_H
