#include <QThread>
#include <iostream>
#include "mainwindow.h"

SdrWorker::SdrWorker(SoapySDRDevice *sdr,
                        size_t *channels, 
                        size_t channel_count,
                        QObject* parent)
    : QObject(parent), sdr(sdr), channelList(channels, channels + channel_count)
      {

    // Инициализация потоков
    rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channelList.data(), channelList.size(), NULL);
    if (rxStream == nullptr) {
        qCritical() << "Failed to setup RX stream:" << SoapySDRDevice_lastError();
        exit(1);
    }

    txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channelList.data(), channelList.size(), NULL);
    if (txStream == nullptr) {
        qCritical() << "Failed to setup TX stream:" << SoapySDRDevice_lastError();
        exit(1);
    }

    rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    
    iteration_count = 6;

    // Буфер для приема
    buffer.resize(2 * rx_mtu);
    // Буфер для передачи
    //tx_buff.resize(2 * tx_mtu);

    // Активация потоков
    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);

    // Генерация данных для передачи
    generateTxData();

    connect(&timer, &QTimer::timeout, this, &SdrWorker::process);
    timer.start(30);  // Интервал
}

SdrWorker::~SdrWorker() { // Деструктор

    // Остановка таймера
    timer.stop();
    
    // Деактивация и закрытие потоков
    if (rxStream) {
        SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
        SoapySDRDevice_closeStream(sdr, rxStream);
    }
    if (txStream) {
        SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
        SoapySDRDevice_closeStream(sdr, txStream);
    }

    // Уничтожение устройства
    if (sdr) {
        SoapySDRDevice_unmake(sdr);
    }

}

void SdrWorker::generateTxData() {
    // Вектор, в котором хранится сформированный qam сигнал
//std::vector<std::complex<int16_t>> tx_data;
// Определяем код Баркера длиной 7
std::vector<int> barker7 = {1, 0, 1, 1, 1, 0, 1};
QAM_Mod qamModulator(4);  // Для QPSK=4, для 16-QAM =16
// Передаем сообщение hello world
std::vector<int> message = {0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0,
                            0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0};

// Импульсная характеристика формирующего фильтра из единииц (10)
std::vector<int> formingFilter = {1,1,1,1,1,1,1,1,1,1};
// Формирование пакета = заголовок (код Баркера) + данные (message)
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

}

void SdrWorker::process() {

//qDebug() << "SdrWorker::process called from thread:" << QThread::currentThread();
//qDebug() << "Entering process()";

if (!rxStream || !txStream) {
    qCritical() << "Streams are not initialized!";
    return;
}

//qDebug() << "Streams are initialized";

// Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
// size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
// size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
//printf("MTU - TX: %lu, RX: %lu\n", tx_mtu, rx_mtu);

// Текущая позиция в данных для передачи
// Размер буфера на передачу
tx_buffer_size = tx_mtu; 

// Формирование данных для передачи


// // Заполнение буфера tx_buff
int16_t tx_buff[2 * tx_mtu];
// for (size_t i = 0; i < tx_data.size() && i < tx_mtu; ++i) {
//     tx_buff[2 * i] = tx_data[i].real();
//     tx_buff[2 * i + 1] = tx_data[i].imag();
// }

// Заполнение буфера на передачу циклически
//int16_t tx_buff[2 * tx_mtu];
    for (size_t i = 0; i < tx_buffer_size; ++i) {
        size_t data_index = (tx_data_pos + i) % tx_data.size();
        tx_buff[2 * i] = tx_data[data_index].real();
        tx_buff[2 * i + 1] = tx_data[data_index].imag();}

// // Активация потоков
// SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
// SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming

// Основной цикл передачи и приема
//printf("Start test...\n");
const long timeoutUs = 400000; // 

//int16_t *buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));
//std::vector<int16_t> buffer(2 * rx_mtu);
if (buffer.data() == NULL) {
    printf("Memory allocation failed\n");
    exit(1);
}

// При первом запуске очищаем rx_buff от возможного мусора.
for (size_t buffers_read = 0; buffers_read < 30; /* in loop */)
{
    void *buffs[] = {buffer.data()}; //void *buffs[] = {rx_buff[0][0], rx_buff[0][1]}; //array of buffers
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

//long long last_time = 0;

// Количество итераций
//size_t iteration_count = 6;
// Начинается работа с получения и отправки сэмплов
for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++)
{
    void *rx_buffs[] = {buffer.data()};
    // int flags;        // flags set by receive operation
    // long long timeNs; //timestamp for receive buffer

    int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
    if (sr < 0)
    {
        // Skip read on error (likely timeout)
        continue;
    }

    if (buffers_read % 2 == 0) {  // Вызываем сигнал каждые n буферов
        emit dataReady(buffer.data(), sr);
        buffer.clear();  // Очищаем данные после передачи
        }
    // Dump info
    //printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
    //fwrite(buffer.data(), 2 * rx_mtu * sizeof(int16_t), 1, file);

    // Преобразование данных в std::complex<int16_t> и добавление в вектор
    // for (int i = 0; i < sr; ++i) {
    //     int16_t real = buffer[2 * i];
    //     int16_t imag = buffer[2 * i + 1];
    //     //rx_data.push_back(std::complex<int16_t>(real, imag));
    // }

    //last_time = timeNs;

    // Calculate transmit time 4ms in future 
    //long long tx_time = timeNs + (4* 1000 * 1000);
    long long tx_time = timeNs + (4* 1000 * 1000);

    // if (!rx_data.empty()) {
    //  // Передача принятых данных для обработки
    //  emit dataReady(rx_data);
    // }

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
        //fwrite(tx_buff, 2 * tx_buffer_size * sizeof(int16_t), 1, file2);
        if ((size_t)st != tx_buffer_size)
        {
            printf("TX Failed: %i\n", st);
        }
    //}
    
}
}
