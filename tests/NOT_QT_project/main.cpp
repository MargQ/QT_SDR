#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include "filter.h"
#include "qam.h"
#include "TED.h"

int main()
{
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:");
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");
    }

    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "loopback", "0");

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL)
    {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return EXIT_FAILURE;
    }

    // Настраиваем устройства RX\TX
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, 1e6) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, 800e6, NULL) != 0)
    {
        printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, 1e6) != 0)
    {
        printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, 800e6, NULL) != 0)
    {
        printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
    }
    printf("SoapySDRDevice_getFrequency tx: %lf\n", SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));
    
    // Настройка каналов и стримов
    size_t channels[] = {0}; // {0} or {0, 1} 
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (rxStream == NULL)
    {
        printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }
    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -50.0) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (txStream == NULL)
    {
        printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    printf("MTU - TX: %lu, RX: %lu\n", tx_mtu, rx_mtu);

    // Формирование данных для передачи
    std::vector<std::complex<int16_t>> tx_data;
    //std::vector<int> barker7 = {1, -1, 1, 1, 1, -1, 1};
    std::vector<int> barker7 = {1, 0, 1, 1, 1, 0, 1};
    QAM_Mod qamModulator(4);  // Для QPSK=4, для 16-QAM =16
    std::vector<int> message = {0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0,
                                0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0};

    std::vector<int> formingFilter = {1,1,1,1,1,1,1,1,1,1};
    std::vector<int> combinedStream;
    combinedStream.insert(combinedStream.end(), barker7.begin(), barker7.end());
    combinedStream.insert(combinedStream.end(), message.begin(), message.end());

    // Добавляем padding, если размер combinedStream не кратен symbolBits
    int symbolBits = std::log2(4);
    if (combinedStream.size() % symbolBits != 0) {
        size_t paddingSize = symbolBits - (combinedStream.size() % symbolBits);
        combinedStream.insert(combinedStream.end(), paddingSize, 0);
    }

    std::vector<std::complex<float>> qamSymbols = qamModulator.mod(combinedStream);
    tx_data.clear();
    for (const auto& symbol : qamSymbols) {
        int real_int = static_cast<int>(symbol.real());
        int imag_int = static_cast<int>(symbol.imag());

        std::vector<int> realVector = {real_int};
        std::vector<int> imagVector = {imag_int};

        std::vector<int> filteredReal = FormingFilter(realVector, formingFilter);
        std::vector<int> filteredImag = FormingFilter(imagVector, formingFilter);

        int scaling_factor = pow(2,14);
        for (size_t i = 0; i < filteredReal.size(); ++i) {
            int16_t real_scaled = static_cast<int16_t>(filteredReal[i] * scaling_factor);
            int16_t imag_scaled = static_cast<int16_t>(filteredImag[i] * scaling_factor);
            tx_data.push_back(std::complex<int16_t>(real_scaled, imag_scaled));
        }
    }

    // Заполнение буфера tx_buff
    int16_t tx_buff[2 * tx_mtu];
    for (size_t i = 0; i < tx_data.size() && i < tx_mtu; ++i) {
        tx_buff[2 * i] = tx_data[i].real();
        tx_buff[2 * i + 1] = tx_data[i].imag();
    }

    // Активация потоков
    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming

    // Основной цикл передачи и приема
    printf("Start test...\n");
    const long timeoutUs = 400000; // 

    int16_t *buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    // При первом запуске очищаем rx_buff от возможного мусора.
    for (size_t buffers_read = 0; buffers_read < 128; /* in loop */)
    {
        void *buffs[] = {buffer}; //void *buffs[] = {rx_buff[0][0], rx_buff[0][1]}; //array of buffers
        int flags; //flags set by receive operation
        long long timeNs; //timestamp for receive buffer
        // Read samples
        int sr = SoapySDRDevice_readStream(sdr, rxStream, buffs, rx_mtu, &flags, &timeNs, timeoutUs); // 100ms timeout
        if (sr < 0)
        {
            // Skip read on error (likely timeout)
            continue;
        }
        // Increment number of buffers read
        buffers_read++;
    }

    long long last_time = 0;

    // Создаем файл для записи сэмплов с rx_buff
    FILE *file = fopen("rxdata.pcm", "w");

    // Создаем файл для записи сэмплов с tx_buff
    FILE *file2 = fopen("txdata.pcm", "w");
    
    // Текущая позиция в данных для передачи
    size_t tx_data_pos = 0;
    // Размер буфера на передачу
    size_t tx_buffer_size = tx_mtu;  
    // Количество итераций
    size_t iteration_count = 8;

    std::vector<std::complex<int16_t>> rx_data;

    // Начинается работа с получения и отправки сэмплов
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++)
    {
        void *rx_buffs[] = {buffer};
        int flags;        // flags set by receive operation
        long long timeNs; //timestamp for receive buffer

        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        if (sr < 0)
        {
            // Skip read on error (likely timeout)
            continue;
        }

        // Dump info
        printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
        fwrite(buffer, 2 * rx_mtu * sizeof(int16_t), 1, file);
        // Преобразование данных в std::complex<int16_t> и добавление в вектор
        for (int i = 0; i < sr; ++i) {
            int16_t real = buffer[2 * i];
            int16_t imag = buffer[2 * i + 1];
            rx_data.push_back(std::complex<int16_t>(real, imag));
        }

        last_time = timeNs;

        // Calculate transmit time 4ms in future 
        //long long tx_time = timeNs + (4* 1000 * 1000);
        long long tx_time = timeNs + (4* 1000 * 1000);


        // Заполнение буфера на передачу циклически
        for (size_t i = 0; i < tx_buffer_size; ++i) {
            size_t data_index = (tx_data_pos + i) % tx_data.size();
            tx_buff[2 * i] = tx_data[data_index].real();
            tx_buff[2 * i + 1] = tx_data[data_index].imag();
        }
        tx_data_pos = (tx_data_pos + tx_buffer_size) % tx_data.size();

        // Передача данных
        void *tx_buffs[] = {tx_buff};
        // Без цикла заполняется каждый блок буфера, но прием выходит странный
        //if (buffers_read == 1) {
            flags = SOAPY_SDR_HAS_TIME;
            int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_buffer_size, &flags, tx_time, 0);
            fwrite(tx_buff, 2 * tx_buffer_size * sizeof(int16_t), 1, file2);
            if ((size_t)st != tx_buffer_size)
            {
                printf("TX Failed: %i\n", st);
            }
        //}
    }
    fclose(file);

    // Нормализация данных
    // Нужно тут нормализоавать в начале, потом фильтр (свертка), 
    //потом TED (символьная синхронизация), затем частотная (?), демодулятор, декодер

    std::vector<std::complex<int16_t>> filtered_rx_data;
    // Проводим данные через согласованный фильтр
    for (const auto& sample : rx_data) {
        int real_int = static_cast<int>(sample.real());
        int imag_int = static_cast<int>(sample.imag());

        std::vector<int> realVector = {real_int};
        std::vector<int> imagVector = {imag_int};
        // Фильтр с ИХ 1111111111
        std::vector<int> filteredReal = FormingFilter(realVector, formingFilter);
        std::vector<int> filteredImag = FormingFilter(imagVector, formingFilter);
        for (size_t i = 0; i < filteredReal.size(); ++i) {
                int16_t real_scaled = static_cast<int16_t>(filteredReal[i]);
                int16_t imag_scaled = static_cast<int16_t>(filteredImag[i]);
                filtered_rx_data.push_back(std::complex<int16_t>(real_scaled, imag_scaled));
            }
    }

    // Преобразование данных из std::complex<int16_t> в std::complex<double>
    std::vector<std::complex<double>> filtered_rx_data_double;
    for (const auto& sample : rx_data) {
        filtered_rx_data_double.push_back(std::complex<double>(
            static_cast<double>(sample.real()),
            static_cast<double>(sample.imag())
        ));
    }

    // Вызов функции TED_loop_filter
    std::vector<int> TED_indices = TED(filtered_rx_data_double);

    // Массив rx с индексами из TED
    std::vector<std::complex<int16_t>> TED_rx_data;

    // Создаем новый вектор, используя индексы из TED_indices
    for (int index : TED_indices) {
        // Проверяем, что индекс находится в пределах допустимого диапазона
        if (index >= 0 && index < static_cast<int>(filtered_rx_data.size())) {
            TED_rx_data.push_back(filtered_rx_data[index]);
        } else {
            printf("Index %d is out of bounds for rx_data\n", index);
        }
    }

    // Создаем файл для записи отфильтрованных сэмплов
    FILE *file3 = fopen("TED.pcm", "wb");
    if (file3 == nullptr) {
        printf("Failed to open file for writing\n");
        return EXIT_FAILURE;
    }

    // Записываем отфильтрованные данные
    for (const auto& sample : filtered_rx_data) { // filtered_rx_data TED_rx_data
        int16_t real = sample.real();
        int16_t imag = sample.imag();
        
        // Записываем реальную и мнимую часть в файл
        fwrite(&real, sizeof(int16_t), 1, file3);
        fwrite(&imag, sizeof(int16_t), 1, file3);
    }

    // Закрываем файл
    fclose(file3);


    // Остановка потоков
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

    // Закрытие потоков
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    // Уничтожение устройства
    SoapySDRDevice_unmake(sdr);

    // Освобождение памяти
    free(buffer);

    printf("Test complete!\n");
    return EXIT_SUCCESS;
}