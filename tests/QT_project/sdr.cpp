#include "sdr.h"
#include <fstream>  // Для работы с файлами

/* helper macros */
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

struct stream_cfg {
	long long bw_hz; // Analog banwidth in Hz
	long long fs_hz; // Baseband sample rate in Hz
	long long lo_hz; // Local oscillator frequency in Hz
	const char* rfport; // Port name
};

std::vector<std::complex<int16_t>> getSdrData(struct iio_context *ctx, 
                                              struct iio_device *rx_dev, 
                                              struct iio_channel *rx0_i, 
                                              struct iio_channel *rx0_q, 
                                              struct iio_channels_mask *rxmask, 
                                              struct iio_stream  *rxstream)
{
    // Конфиг. параметры "потоков"
    struct stream_cfg rxcfg;

    //printf("* Инициализация AD9361 устройств\n");
    struct iio_device *phy_dev;
    phy_dev  =  iio_context_find_device(ctx, "ad9361-phy");
    rx_dev =    iio_context_find_device(ctx, "cf-ad9361-lpc");

    //printf("* Enabling IIO streaming channels\n");
    iio_channel_enable(rx0_i, rxmask);
    iio_channel_enable(rx0_q, rxmask);

    // RX stream config
    rxcfg.bw_hz = MHZ(1);   // 2 MHz rf bandwidth
    rxcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s rx sample rate
    rxcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
    rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
    //rxcfg. = 40; // Устанавливаем усиление в 40 дБ, настройте по необходимости

    
    //printf("* Настройка параметров %s канала AD9361 \n", "RX");
    struct iio_channel *rx_chn = NULL;
    rx_chn = iio_device_find_channel(phy_dev, "voltage0", false);
    const struct iio_attr *rx_rf_port_attr = iio_channel_find_attr(rx_chn, "rf_port_select");
    iio_attr_write_string(rx_rf_port_attr, rxcfg.rfport);
    const struct iio_attr *rx_bw_attr = iio_channel_find_attr(rx_chn, "rf_bandwidth");
    iio_attr_write_longlong(rx_bw_attr, rxcfg.bw_hz);
    const struct iio_attr *rx_fs_attr = iio_channel_find_attr(rx_chn, "sampling_frequency");
    iio_attr_write_longlong(rx_fs_attr, rxcfg.fs_hz);
    const struct iio_attr *rx_gain_attr = iio_channel_find_attr(rx_chn, "gain_control_mode");
    //iio_attr_write_string(rx_gain_attr, "slow_attack");
    // Настройка усиления
    iio_attr_write_longlong(rx_gain_attr, 50);
    
    //printf("* Настройка частоты опорного генератора (lo, local oscilator)  %s \n", "RX");
    struct iio_channel *rx_lo_chn = NULL;
    rx_lo_chn = iio_device_find_channel(phy_dev, "altvoltage0", true);
    const struct iio_attr *rx_lo_attr = iio_channel_find_attr(rx_lo_chn, "frequency");
    iio_attr_write_longlong(rx_lo_attr, rxcfg.lo_hz);

    //printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
    struct iio_buffer *rxbuf = iio_device_create_buffer(rx_dev, 0, rxmask);
    if (!rxbuf) {
        std::cerr << "Unable to create buffer" << std::endl;
        return {};
    }

    std::vector<std::complex<int16_t>> data;
    data.reserve(1024);
    rxstream = iio_buffer_create_stream(rxbuf, 4, pow(2, 10));   
    size_t rx_sample_sz;
    rx_sample_sz = iio_device_get_sample_size(rx_dev, rxmask);
    //printf("* rx_sample_sz = %d\n",rx_sample_sz);

    const struct iio_block *rxblock;
    rxblock = iio_stream_get_next_block(rxstream);
    int16_t *p_dat;
    ptrdiff_t p_inc;

    void *p_end = iio_block_end(rxblock);
    auto p_dat_i = (int16_t *)iio_block_first(rxblock, rx0_i);
    auto p_dat_q = (int16_t *)iio_block_first(rxblock, rx0_q);
    rxblock = iio_stream_get_next_block(rxstream);

    // Открываем файл для записи данных в текстовом формате
    std::ofstream outfile("sdr_data.txt");

    // if (!outfile.is_open()) {
    //     std::cerr << "Error: Unable to open file for writing." << std::endl;
    //     iio_buffer_destroy(rxbuf);
    //     return {};
    // }

    p_inc = rx_sample_sz;
    size_t current_size = data.size();
    size_t new_elements = ((int16_t*)p_end - p_dat_i);
    //size_t new_elements = (p_end - p_dat_i) / sizeof(int16_t);
    data.resize(current_size + new_elements / 2); // Делим на 2, так как каждый комплексный элемент состоит из двух int16_t

    std::complex<int16_t>* data_ptr = data.data() + current_size;
    int32_t counter = 0;

    while (p_dat_i < p_end && p_dat_q < p_end) 
    {
        // Копируем I и Q компоненты в вектор данных
        data_ptr->real(*p_dat_i);
        data_ptr->imag(*p_dat_q);

        // Записываем I и Q компоненты в текстовом формате
        outfile << *p_dat_i << ", " << *p_dat_q << "\n";

        // Увеличиваем указатели на следующий отсчет
        p_dat_i = (int16_t *)((char *)p_dat_i + p_inc);
        p_dat_q = (int16_t *)((char *)p_dat_q + p_inc);
        data_ptr++;  // Переходим к следующему комплексному числу в векторе data
    }


    // Закрываем файл
    outfile.close();

    iio_buffer_destroy(rxbuf);
    return data;
}


void sendSdrData(struct iio_context *ctx, struct iio_device *tx_dev, struct iio_channel *tx0_i, struct iio_channel *tx0_q, struct iio_channels_mask *txmask, struct iio_stream  *txstream, const std::vector<std::complex<int16_t>>& tx_data) {
    // Конфиг. параметры "потоков"
	struct stream_cfg txcfg;

    //printf("* Инициализация AD9361 устройств\n");
    struct iio_device *phy_dev;
    phy_dev  =  iio_context_find_device(ctx, "ad9361-phy");
    tx_dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");

    if (!tx_dev) {
    std::cerr << "Unable to find TX device" << std::endl;
    //return;
    }

    //printf("* Enabling IIO streaming channels\n");
	iio_channel_enable(tx0_i, txmask);
	iio_channel_enable(tx0_q, txmask);

    if (!tx0_i || !tx0_q) {
    std::cerr << "Unable to find TX channels" << std::endl;
    //return;
    }

    // TX stream config
	txcfg.bw_hz = MHZ(1); // 1.5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
	txcfg.rfport = "A"; // port A (select for rf freq.)

    //printf("* Настройка параметров %s канала AD9361 \n", "TX");
    struct iio_channel *tx_chn = NULL;
    tx_chn = iio_device_find_channel(phy_dev, "voltage0", true);

    const struct iio_attr *tx_rf_port_attr = iio_channel_find_attr(tx_chn, "rf_port_select");
    iio_attr_write_string(tx_rf_port_attr, txcfg.rfport);
    const struct iio_attr *tx_bw_attr = iio_channel_find_attr(tx_chn, "rf_bandwidth");
    iio_attr_write_longlong(tx_bw_attr, txcfg.bw_hz);
    const struct iio_attr *tx_fs_attr = iio_channel_find_attr(tx_chn, "sampling_frequency");
    iio_attr_write_longlong(tx_fs_attr, txcfg.fs_hz);
    //printf("* Настройка частоты опорного генератора (lo, local oscilator)  %s \n", "TX");
    struct iio_channel *tx_lo_chn = NULL;
    tx_lo_chn = iio_device_find_channel(phy_dev, "altvoltage1", true);
    const struct iio_attr *tx_lo_attr = iio_channel_find_attr(tx_lo_chn, "frequency");
    iio_attr_write_longlong(tx_lo_attr, txcfg.lo_hz);
    const struct iio_attr *tx_gain_attr = iio_channel_find_attr(tx_chn, "hardwaregain");
    iio_attr_write_longlong(tx_gain_attr, 10);


    //printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
    struct iio_buffer *txbuf = iio_device_create_buffer(tx_dev, 0, txmask);
    if (!txbuf) {
        std::cerr << "Unable to create buffer" << std::endl;
        //return NULL;
    }

    // std::vector<std::complex<int16_t>> data;
    // data.reserve(1024);
    /*Размер буфера.
     Был задан 1024*1024,
     так как график выводится в реальном времени, то появилась проблема
     в виде задержки обновления графика. 
     Для работы в реальном времени лучше использовать маленький буфер */
    txstream = iio_buffer_create_stream(txbuf, 4, pow(2, 10));   
    size_t tx_sample_sz;
    tx_sample_sz = iio_device_get_sample_size(tx_dev, txmask);
    //printf("* tx_sample_sz = %d\n",tx_sample_sz);

    const struct iio_block *txblock;
    txblock = iio_stream_get_next_block(txstream);
    int16_t *p_dat;
	ptrdiff_t p_inc;

    void *p_end = iio_block_end(txblock);
    auto p_dat_i = (int16_t *)iio_block_first(txblock, tx0_i);
    auto p_dat_q = (int16_t *)iio_block_first(txblock, tx0_q);
    txblock = iio_stream_get_next_block(txstream);

    // Открываем файл для записи данных
    std::ofstream outfile("sent_qam_signal.txt", std::ios::out);

    if (!outfile.is_open()) {
        std::cerr << "Unable to open file for writing" << std::endl;
        //return;
    }

    // Заполнение буфера данными
    size_t i = 0;
    while (p_dat_i && p_dat_q && i < tx_data.size()) {
        *p_dat_i = tx_data[i].real();
        *p_dat_q = tx_data[i].imag();
        outfile << tx_data[i].real() << ", " << tx_data[i].imag() << std::endl;
        p_dat_i++;
        p_dat_q++;
        i++;
        //printf("%d, %d\n", i, q);

    }

    iio_buffer_destroy(txbuf);
}
