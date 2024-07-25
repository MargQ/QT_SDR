#include "sdr.h"
#include <iostream>

struct iio_context* scan() {
    struct iio_scan_context *scan_ctx;
    struct iio_context_info **info;
    struct iio_context *ctx = NULL;
    ssize_t ret;

    scan_ctx = iio_create_scan_context(NULL, 0);
    if (!scan_ctx) {
        std::cerr << "Unable to create scan context" << std::endl;
        return NULL;
    }

    ret = iio_scan_context_get_info_list(scan_ctx, &info);
    if (ret < 0) {
        char err_str[1024];
        iio_strerror(-ret, err_str, sizeof(err_str));
        std::cerr << "Scanning for IIO contexts failed: " << err_str << std::endl;
        goto err_free_ctx;
    }

    if (ret == 0) {
        std::cerr << "No IIO context found." << std::endl;
        goto err_free_info_list;
    }

    if (ret == 1) {
        ctx = iio_create_context_from_uri(iio_context_info_get_uri(info[0]));
    } else {
        std::cerr << "Multiple contexts found. Please select one using --uri:" << std::endl;
        for (int i = 0; i < ret; i++) {
            std::cerr << "\t" << i << ": " << iio_context_info_get_description(info[i]) << " [" << iio_context_info_get_uri(info[i]) << "]" << std::endl;
        }
    }

    err_free_info_list:
        iio_context_info_list_free(info);
    err_free_ctx:
        iio_scan_context_destroy(scan_ctx);

    return ctx;
}

std::vector<std::complex<int16_t>> getSdrData(struct iio_context *ctx, struct iio_device *rx, struct iio_channel *rx0_i, struct iio_channel *rx0_q) {
    iio_channel_enable(rx0_i);
    iio_channel_enable(rx0_q);

    struct iio_buffer *rxbuf = iio_device_create_buffer(rx, 1024, false);
    if (!rxbuf) {
        std::cerr << "Unable to create buffer" << std::endl;
        return {};
    }

    std::vector<std::complex<int16_t>> data;
    data.reserve(1024);
    ssize_t nbytes = iio_buffer_refill(rxbuf);
    if (nbytes < 0) {
        std::cerr << "Error refilling buffer: " << nbytes << std::endl;
        iio_buffer_destroy(rxbuf);
        return {};
    }

    void *p_end = iio_buffer_end(rxbuf);
    ptrdiff_t p_inc = iio_buffer_step(rxbuf);
    auto p_dat_i = (int16_t *)iio_buffer_first(rxbuf, rx0_i);
    auto p_dat_q = (int16_t *)iio_buffer_first(rxbuf, rx0_q);

    while (p_dat_i < p_end && p_dat_q < p_end) {
        data.emplace_back(*p_dat_i, *p_dat_q);
        p_dat_i = (int16_t *)((char *)p_dat_i + p_inc);
        p_dat_q = (int16_t *)((char *)p_dat_q + p_inc);
    }

    iio_buffer_destroy(rxbuf);
    return data;
}
