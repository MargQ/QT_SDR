#include <QThread>
#include <iostream>
#include "mainwindow.h"


// Конструктор с параметрами для передачи данных
SdrWorker::SdrWorker(struct iio_context* ctx, struct iio_device* rx_dev, struct iio_channel* rx0_i, struct iio_channel* rx0_q, struct iio_channels_mask *rxmask, struct iio_stream  *rxstream,
                     struct iio_device* tx_dev, struct iio_channel* tx0_i, struct iio_channel* tx0_q, struct iio_channels_mask *txmask, struct iio_stream  *txstream, QObject* parent)
    : QObject(parent), ctx(ctx), rx_dev(rx_dev), rx0_i(rx0_i), rx0_q(rx0_q), rxmask(rxmask), rxstream(rxstream),
      tx_dev(tx_dev), tx0_i(tx0_i), tx0_q(tx0_q), txmask(txmask), txstream(txstream) {
    connect(&timer, &QTimer::timeout, this, &SdrWorker::process);
    timer.start(1);  // Интервал
}


SdrWorker::~SdrWorker() { // Деструктор
}


void SdrWorker::process() {
  //qDebug() << "SdrWorker::process called from thread:" << QThread::currentThread();

  // вектор, в котором хранится сформированный qam сигнал
  std::vector<std::complex<int16_t>> tx_data;
  // Определяем код Баркера длиной 7
    std::vector<int> barker7 = {1, -1, 1, 1, 1, -1, 1};
  QAM_Mod qamModulator(4);  // Для QPSK=4, для 16-QAM =16
  // Передаем сообщение hello world
  std::vector<int> message = {0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0,
  0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0};

  // Импульсная характеристика формирующего фильтра из единииц (16)
  std::vector<int> formingFilter = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
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

      //     // Вывод результата
      // for (const auto& value : filteredImag) {
      //     std::cout << value << " ";
      // }
      //std::cout << std::endl;

      // // Вывод данных
      // std::cout << "Original real value: " << real_int << "\n";
      // std::cout << "Oversampled real values: ";
      // for (const auto& val : oversampledReal) {
      //     std::cout << val << " ";
      // }
      // std::cout << "\n";

      // Определяем коэффициент масштабирования, эквивалентный 2^14
      int scaling_factor = 16384;
      // Добавляем весь оверсемплированный сигнал в tx_data
      for (size_t i = 0; i < filteredReal.size(); ++i) {
          // int16_t real_scaled = static_cast<int16_t>(oversampledReal[i] * scaling_factor);
          // int16_t imag_scaled = static_cast<int16_t>(oversampledImag[i] * scaling_factor);
          int16_t real_scaled = static_cast<int16_t>(filteredReal[i] * scaling_factor);
          int16_t imag_scaled = static_cast<int16_t>(filteredImag[i] * scaling_factor);
          tx_data.push_back(std::complex<int16_t>(real_scaled, imag_scaled));
      }

      

      // Масштабируем передаваемые значения
      // // Это эквивалентно 2^14 = 16384
      // int16_t scaling_factor = 1 << 14;  
      // int16_t real_scaled = real * scaling_factor;
      // int16_t imag_scaled = imag * scaling_factor;
      //tx_data.push_back(std::complex<int16_t>(real_scaled, imag_scaled));
    }
  

  //while(1){
    
    // Приём данных
    std::vector<std::complex<int16_t>> rx_data = getSdrData(ctx, rx_dev, rx0_i, rx0_q, rxmask, rxstream);
    //if (!data.empty()) {
    // Передача принятых данных для обработки
    emit dataReady(rx_data);
    // // Отправка данных
    sendSdrData(ctx, tx_dev, tx0_i, tx0_q, txmask, txstream, tx_data);
    //}

}