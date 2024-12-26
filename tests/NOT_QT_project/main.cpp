#include <iostream>
#include <iio/iio.h>
#include <iio/iio-debug.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <complex>
#include <fstream>  // Для работы с файлами

#include "qam.h"
#include "filter.h"

/* helper macros */
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

 struct iio_context* ctx;
    struct iio_device* rx_dev;
    struct iio_device* tx_dev;
    struct iio_channel* rx0_i;
    struct iio_channel *tx0_i;
    struct iio_channel* rx0_q;
    struct iio_channel *tx0_q;
    struct iio_channels_mask* rxmask = NULL;
    struct iio_channels_mask* txmask = NULL;
    struct iio_stream  *rxstream = NULL;
    struct iio_stream  *txstream = NULL;

struct stream_cfg {
	long long bw_hz; // Analog banwidth in Hz
	long long fs_hz; // Baseband sample rate in Hz
	long long lo_hz; // Local oscillator frequency in Hz
	const char* rfport; // Port name
};


std::vector<std::complex<int16_t>> repeat(const std::vector<std::complex<int16_t>>& input, int repeats) {
    std::vector<std::complex<int16_t>> output;
    output.reserve(input.size() * repeats); // Резервируем память для выходного вектора

    for (const auto& element : input) {
        for (int i = 0; i < repeats; ++i) {
            output.push_back(element); // Добавляем элемент в выходной вектор
        }
    }
    return output;
}

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
    rxcfg.bw_hz = MHZ(2);   // 2 MHz rf bandwidth
    rxcfg.fs_hz = MHZ(2.1);   // 2.5 MS/s rx sample rate
    rxcfg.lo_hz = GHZ(1.9); // 2.5 GHz rf frequency
    rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
    
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
    // Настройка усиления
    iio_attr_write_string(rx_gain_attr, "manual"); // Автоматическое усилиение - slow attack,ручное - manual
    iio_attr_write_longlong(rx_gain_attr, 20);
    
    //printf("* Настройка частоты опорного генератора (lo, local oscilator)  %s \n", "RX");
    struct iio_channel *rx_lo_chn = NULL;
    rx_lo_chn = iio_device_find_channel(phy_dev, "altvoltage0", true);
    const struct iio_attr *rx_lo_attr = iio_channel_find_attr(rx_lo_chn, "frequency");
    iio_attr_write_longlong(rx_lo_attr, rxcfg.lo_hz);

    //printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
    struct iio_buffer *rxbuf = iio_device_create_buffer(rx_dev, 0, rxmask);
    if (!rxbuf) {
        std::cerr << "Unable to create buffer" << std::endl;
        exit(1);
    }

    std::vector<std::complex<int16_t>> data;
    //data.reserve(pow(2, 11));
    rxstream = iio_buffer_create_stream(rxbuf, 4, pow(2, 11));   
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

    // Открываем файл для записи данных в текстовом формате
    std::ofstream outfile("sdr_data.txt");

    p_inc = rx_sample_sz;

    for (p_dat = (int16_t*)iio_block_first(rxblock, rx0_i); p_dat < p_end;p_dat += p_inc / sizeof(*p_dat)) 

    //while (p_dat_i < p_end && p_dat_q < p_end) 
    {
        // Копируем I и Q компоненты в вектор данных
        rxblock = iio_stream_get_next_block(rxstream);

        int16_t real_part = *p_dat_i;
        int16_t imag_part = *p_dat_q;

        // Записываем I и Q компоненты в текстовом формате
        outfile << real_part << ", " << imag_part << "\n";

        data.push_back(std::complex<int16_t>(real_part, imag_part));
        // Увеличиваем указатели на следующий отсчет
        p_dat_i += rx_sample_sz / sizeof(p_dat_i); // Переход к следующему элементу I
        p_dat_q += rx_sample_sz / sizeof(p_dat_q);

    }

    // Закрываем файл
    outfile.close();

    iio_buffer_destroy(rxbuf);
    return data;
}


void sendSdrData(struct iio_context *ctx, 
                 struct iio_device *tx_dev, 
                 struct iio_channel *tx0_i, 
                 struct iio_channel *tx0_q, 
                 struct iio_channels_mask *txmask, 
                 struct iio_stream  *txstream, 
                 const std::vector<std::complex<int16_t>>& tx_data) 
{
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
	txcfg.bw_hz = MHZ(2); // 1.5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(2.1);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = GHZ(1.9); // 2.5 GHz rf frequency
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
    // Настройка усиления
    iio_attr_write_string(tx_gain_attr, "manual");
    iio_attr_write_longlong(tx_gain_attr,-30);
    
    // const char *loopback_attr_name = "loopback";
    // tx_fs_attr = iio_channel_find_attr(tx_chn, loopback_attr_name);

    // if (!tx_fs_attr) {
    //     fprintf(stderr, "Ошибка: не удалось найти атрибут '%s' на устройстве TX\n", loopback_attr_name);
    //     iio_context_destroy(ctx);
    //     exit(1);
    // }

    // printf("Атрибут '%s' найден на устройстве TX\n", loopback_attr_name);

    //printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
    struct iio_buffer *txbuf = iio_device_create_buffer(tx_dev, 0, txmask);
    if (!txbuf) {
        std::cerr << "Unable to create buffer" << std::endl;
        exit(1);
    }

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
        i++;
        //printf("%d, %d\n", i, q);

    }

    // Закрываем файл
    outfile.close();

    iio_buffer_destroy(txbuf);
}

int main()
{
    // Инициализация SDR
    ctx = iio_create_context(NULL,"ip:192.168.3.1");
    if (!ctx) {
        std::cerr << "Unable to create context" << std::endl;
        exit(1);
    }
    rx_dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
    if (!rx_dev) {
        std::cerr << "Unable to find device" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }
    tx_dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (!tx_dev) {
        std::cerr << "Unable to find device" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }

    //printf("* Инициализация потоков I/Q %s канала AD9361 \n", "TX");
    tx0_i = iio_device_find_channel(tx_dev, "voltage0", true);
    tx0_q = iio_device_find_channel(tx_dev, "voltage1", true);

    //printf("* Инициализация потоков I/Q %s канала AD9361 \n", "RX");
    rx0_i = iio_device_find_channel(rx_dev, "voltage0", false);
    rx0_q = iio_device_find_channel(rx_dev, "voltage1", false);
    if (!tx0_i || !tx0_q) {
        std::cerr << "Unable to find TX channels" << std::endl;
        exit(1);
    }
    if (!rx0_i || !rx0_q) {
        std::cerr << "Unable to find channels" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }

    if (!tx0_i || !tx0_q) {
        std::cerr << "Unable to find channels tx" << std::endl;
        iio_context_destroy(ctx);
        exit(1);
    }
    
    rxmask = iio_create_channels_mask(iio_device_get_channels_count(rx_dev));
    if (!rxmask) {
		fprintf(stderr, "Unable to alloc RX channels mask\n");
	}

    txmask = iio_create_channels_mask(iio_device_get_channels_count(tx_dev));
	if (!txmask) {
		fprintf(stderr, "Unable to alloc TX channels mask\n");
	}

  // вектор, в котором хранится сформированный qam сигнал
  std::vector<std::complex<int16_t>> tx_data;
  // Определяем код Баркера длиной 7
    std::vector<int> barker7 = {1, -1, 1, 1, 1, -1, 1};
  QAM_Mod qamModulator(4);  // Для QPSK=4, для 16-QAM =16
  // Передаем сообщение hello world
  std::vector<int> message = {0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0,
  0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0};

  // Импульсная характеристика формирующего фильтра из единииц (16)
  std::vector<int> formingFilter = {1,1,1,1,1,1,1,1,1,1};
  // Формирование пакета = заголовок (код Баркера) + даные (message)
  std::vector<int> combinedStream;
  // Добавляем код Баркера
   combinedStream.insert(combinedStream.end(), barker7.begin(), barker7.end());
  // Добавляем битовый поток
  combinedStream.insert(combinedStream.end(), message.begin(), message.end());
  // Модуляция битов в QAM символы
  std::vector<std::complex<float>> qamSymbols = qamModulator.mod(message);
  // Преобразование символов в формат int16_t для передачи
  tx_data.clear();
  for (const auto& symbol : qamSymbols) {
      int real_int = static_cast<int>(symbol.real());
      int imag_int = static_cast<int>(symbol.imag());

      // Преобразуем значения real_int и imag_int в векторы
      std::vector<int> realVector = {real_int};
      std::vector<int> imagVector = {imag_int};

      // Выполняем оверсэмплинг
      //std::vector<int> oversampledReal = oversample(realVector, 16);
      //std::vector<int> oversampledImag = oversample(imagVector, 16);

      // Применение формирующего фильтра
        std::vector<int> filteredReal = FormingFilter(realVector, formingFilter);
        std::vector<int> filteredImag = FormingFilter(imagVector, formingFilter);

      // Определяем коэффициент масштабирования, эквивалентный 2^14
      int scaling_factor = pow(2,14);      // Добавляем весь отфильтрованный/оверсемплированный сигнал в tx_data
      for (size_t i = 0; i < filteredReal.size(); ++i) {
          int16_t real_scaled = static_cast<int16_t>(filteredReal[i] * scaling_factor);
          int16_t imag_scaled = static_cast<int16_t>(filteredImag[i] * scaling_factor);
          tx_data.push_back(std::complex<int16_t>(real_scaled, imag_scaled));
      }

    }
    
    // Повторяем данные на отправку
    int repeat_count = 15; // Количество повторений
    // Повторяем элементы вектора
    std::vector<std::complex<int16_t>> repeated_data = repeat(tx_data, repeat_count);

    // Передача данных
    sendSdrData(ctx, tx_dev, tx0_i, tx0_q, txmask, txstream, repeated_data);
    
    // Приём данных
    std::vector<std::complex<int16_t>> rx_data = getSdrData(ctx, rx_dev, rx0_i, rx0_q, rxmask, rxstream);

    
        return 0;
}