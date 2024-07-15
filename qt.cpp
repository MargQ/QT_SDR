#include <iio.h>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <complex>

using namespace QtCharts;

namespace po = boost::program_options;
using std::chrono::high_resolution_clock;

static struct iio_context *scan(void)
{
    struct iio_scan_context *scan_ctx;
    struct iio_context_info **info;
    struct iio_context *ctx = NULL;
    unsigned int i;
    ssize_t ret;

    scan_ctx = iio_create_scan_context(NULL, 0);
    if (!scan_ctx) {
        fprintf(stderr, "Unable to create scan context\n");
        return NULL;
    }

    ret = iio_scan_context_get_info_list(scan_ctx, &info);
    if (ret < 0) {
        char err_str[1024];
        iio_strerror(-ret, err_str, sizeof(err_str));
        fprintf(stderr, "Scanning for IIO contexts failed: %s\n", err_str);
        goto err_free_ctx;
    }

    if (ret == 0) {
        printf("No IIO context found.\n");
        goto err_free_info_list;
    }

    if (ret == 1) {
        ctx = iio_create_context_from_uri(iio_context_info_get_uri(info[0]));
    } else {
        fprintf(stderr, "Multiple contexts found. Please select one using --uri:\n");

        for (i = 0; i < (size_t) ret; i++) {
            fprintf(stderr, "\t%d: %s [%s]\n", i,
                iio_context_info_get_description(info[i]),
                iio_context_info_get_uri(info[i]));
        }
    }

    err_free_info_list:
        iio_context_info_list_free(info);
    err_free_ctx:
        iio_scan_context_destroy(scan_ctx);

    return ctx;
}

// Функция для инициализации SDR и получения данных
std::vector<std::complex<int16_t>> getSdrData(struct iio_context *ctx, struct iio_device *rx, struct iio_channel *rx0_i, struct iio_channel *rx0_q) {
    iio_channel_enable(rx0_i);
    iio_channel_enable(rx0_q);
    
    // Создание буфера размером 1024 сэмпла
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

    void *p_dat, *p_end = iio_buffer_end(rxbuf);
    ptrdiff_t p_inc = iio_buffer_step(rxbuf);

    // Используются оба канала для извлечения данных
    auto p_dat_i = (int16_t *)iio_buffer_first(rxbuf, rx0_i);
    auto p_dat_q = (int16_t *)iio_buffer_first(rxbuf, rx0_q);

    while (p_dat_i < p_end && p_dat_q < p_end) {
        int16_t i = *p_dat_i;
        int16_t q = *p_dat_q;
        data.emplace_back(i, q);
        p_dat_i = (int16_t *)((char *)p_dat_i + p_inc);
        p_dat_q = (int16_t *)((char *)p_dat_q + p_inc);
    }

    iio_buffer_destroy(rxbuf);
    return data;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    struct iio_context *ctx = iio_create_context_from_uri("ip:192.168.3.1");
    if (!ctx) {
        std::cerr << "Unable to create context" << std::endl;
        return 1;
    }

    struct iio_device *rx = iio_context_find_device(ctx, "cf-ad9361-lpc");
    if (!rx) {
        std::cerr << "Unable to find device" << std::endl;
        iio_context_destroy(ctx);
        return 1;
    }

    struct iio_channel *rx0_i = iio_device_find_channel(rx, "voltage0", false);
    struct iio_channel *rx0_q = iio_device_find_channel(rx, "voltage1", false);
    if (!rx0_i || !rx0_q) {
        std::cerr << "Unable to find channels" << std::endl;
        iio_context_destroy(ctx);
        return 1;
    }

    // Создание экземпляра графика (сам график)
    QChart *chart = new QChart();
    // Создание линейного графика (данные, которые передаются в график)
    QLineSeries *realSeries = new QLineSeries();
    QLineSeries *imagSeries = new QLineSeries();

    // Установка легенды
    realSeries->setName("Real");
    imagSeries->setName("Imag");

    //Создание графиков (добавляем объект QLineSeries в график QChart)
    chart->addSeries(realSeries);
    chart->addSeries(imagSeries);

    // Создание осей по умолчанию
    chart->createDefaultAxes();
    // Настройка диапазона осей
    chart->axisY()->setRange(-2000, 2000);
    chart->axisX()->setRange(0, 1024);

    // Заголовок графика
    chart->setTitle("SDR Data Visualization");

    // Устанавливаем подписи для осей
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Samples");
    chart->setAxisX(axisX, realSeries);
    chart->setAxisX(axisX, imagSeries);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Amplitude");
    chart->setAxisY(axisY, realSeries);
    chart->setAxisY(axisY, imagSeries);

    // Создание виджета для отображения графика
    QChartView *chartView = new QChartView(chart);
    // Установка параметра сглаживания
    chartView->setRenderHint(QPainter::Antialiasing);
    // Установка размера окна
    chartView->setMinimumSize(800, 600);

    // Создание главного окна
    QMainWindow mainWindow;
    // Отображение графика в центре главного окна приложения
    mainWindow.setCentralWidget(chartView);
    // Размер главного окна
    mainWindow.resize(800, 600);
    // Отображение главного окна
    mainWindow.show();
    
    // Создание таймера
    QTimer timer;

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        auto data = getSdrData(ctx, rx, rx0_i, rx0_q);
        if (data.empty()) {
            return;
        }

        realSeries->clear();
        imagSeries->clear();

        for (size_t i = 0; i < data.size(); ++i) {
            realSeries->append(i, data[i].real());
            imagSeries->append(i, data[i].imag());
        }

    });

    timer.start(10);

    int ret = app.exec();

    iio_context_destroy(ctx);
    return ret;
}
