// #include "sdr.h"
// #include <fstream>  // Для работы с файлами


// // Функция для инициализации и настройки TX
// SoapySDRStream* setup_tx(SoapySDRDevice *sdr, double sampleRate, double frequency, double txGain) {
    
    

//     // Настройка TX
//     if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sampleRate) != 0) {
//         printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, frequency, NULL) != 0) {
//         printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, 0, txGain) != 0) {
//         printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());
//     }

//     // Настройка канала TX
//     size_t channels[] = {0};
//     size_t channel_count = sizeof(channels) / sizeof(channels[0]);
//     SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
//     if (txStream == NULL) {
//         printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
//         return NULL;
//     }

//     // Получение MTU
//     size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
//     printf("MTU - TX: %lu\n", tx_mtu);

//     return txStream;
// }

// // Функция для инициализации и настройки RX
// SoapySDRStream* setup_rx(SoapySDRDevice *sdr, double sampleRate, double frequency, double rxGain) {
    
    
//     // Настройка RX
//     if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sampleRate) != 0) {
//         printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, frequency, NULL) != 0) {
//         printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, 0, rxGain) != 0) {
//         printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
//     }

//     // Настройка канала RX
//     size_t channels[] = {0};
//     size_t channel_count = sizeof(channels) / sizeof(channels[0]);
//     SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
//     if (rxStream == NULL) {
//         printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
//         return NULL;
//     }

//     // Получение MTU
//     size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
//     printf("MTU - RX: %lu\n", rx_mtu);

//     return rxStream;
// }

// // Функция для передачи данных
// void tx_function(SoapySDRDevice *sdr, SoapySDRStream *txStream, const std::vector<std::complex<int16_t>> &tx_data, size_t tx_mtu) {
//     int16_t tx_buff[2 * tx_mtu];
//     size_t tx_data_pos = 0;
//     size_t tx_buffer_size = tx_mtu;

//     for (size_t i = 0; i < tx_data.size() && i < tx_mtu; ++i) {
//         tx_buff[2 * i] = tx_data[i].real();
//         tx_buff[2 * i + 1] = tx_data[i].imag();
//     }

//     for (size_t buffers_read = 0; buffers_read < 50; ++buffers_read) {
//         // Заполнение буфера на передачу циклически
//         for (size_t i = 0; i < tx_buffer_size; ++i) {
//             size_t data_index = (tx_data_pos + i) % tx_data.size();
//             tx_buff[2 * i] = tx_data[data_index].real();
//             tx_buff[2 * i + 1] = tx_data[data_index].imag();
//         }
//         tx_data_pos = (tx_data_pos + tx_buffer_size) % tx_data.size();

//         // Передача данных
//         void *tx_buffs[] = {tx_buff};
//         int flags = SOAPY_SDR_HAS_TIME;
//         long long tx_time = 0; // Передача без задержки
//         int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_buffer_size, &flags, tx_time, 0);
//         if ((size_t)st != tx_buffer_size) {
//             printf("TX Failed: %i\n", st);
//         }
//     }
// }

// // Функция для приема данных
// std::vector<std::complex<int16_t>> rx_function(SoapySDRDevice *sdr, SoapySDRStream *rxStream, size_t rx_mtu) {
//     std::vector<std::complex<int16_t>> rx_data;  // Вектор для хранения принятых данных
//     int16_t *buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));
//     if (buffer == NULL) {
//         printf("Memory allocation failed\n");
//         return rx_data;  // Возвращаем пустой вектор в случае ошибки
//     }

//     const long timeoutUs = 400000;
//     long long last_time = 0;

//     for (size_t buffers_read = 0; buffers_read < 50; ++buffers_read) {
//         void *rx_buffs[] = {buffer};
//         int flags;
//         long long timeNs;

//         // Чтение данных из RX
//         int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
//         if (sr < 0) {
//             continue;  // Пропустить в случае ошибки
//         }

//         // Преобразование данных в std::complex<int16_t> и добавление в вектор
//         for (int i = 0; i < sr; ++i) {
//             int16_t real = buffer[2 * i];
//             int16_t imag = buffer[2 * i + 1];
//             rx_data.push_back(std::complex<int16_t>(real, imag));
//         }

//         last_time = timeNs;
//     }

//     free(buffer);
//     return rx_data;  // Возвращаем вектор с принятыми данными
// }




