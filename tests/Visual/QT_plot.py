import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import fft, ifft,  fftshift, ifftshift

# Открываем файл для чтения
with open('/home/ub/Desktop/QT_SDR/build/sent_qam_signal.txt', 'r') as file:
    # Читаем все строки из файла
    lines = file.readlines()

# Инициализируем список для хранения данных
data = []

# Обрабатываем каждую строку
for line in lines:
    line = line.strip()  # Убираем лишние пробелы
    if line:  # Пропускаем пустые строки
        try:
            # Разделяем строку по запятой и преобразуем в числа
            numbers = list(map(float, [value.strip() for value in line.split(',')]))
            # Создаём комплексное число (I + jQ)
            complex_number = complex(numbers[0], numbers[1])
            data.append(complex_number)
        except ValueError:
            print(f"Ошибка преобразования строки: {line}")

# Преобразуем список в массив NumPy
rx_nothreads = np.array(data)

# Вывод загруженных данных
#print(rx_nothreads)
plt.figure(1)
plt.scatter(rx_nothreads.real, rx_nothreads.imag)  # Используем scatter для диаграммы созвездия
plt.xlabel('I (Real)')
plt.ylabel('Q (Imaginary)')
plt.title('Передаваемое созвездие 4-QAM')
plt.grid(True)

plt.figure(2)
plt.plot(rx_nothreads)
plt.title('Передаваемый прямоугольный сигнал')

plt.show()


# Открываем файл для чтения
#with open('/home/ub/Desktop/NOT_QT/build/sdr_data.txt', 'r') as file: # из папки билд
with open('/home/ub/Desktop/sdr_data2.txt', 'r') as file: #из рабочего стола лучшие rx
#with open('/home/ub/sdrLessons/build/rx_signal.txt', 'r') as file:
    # Читаем все строки из файла
    lines = file.readlines()

# Инициализируем список для хранения данных
data = []

# Обрабатываем каждую строку
for line in lines:
    line = line.strip()  # Убираем лишние пробелы
    if line:  # Пропускаем пустые строки
        try:
            # Разделяем строку по запятой и преобразуем в числа
            numbers = list(map(float, [value.strip() for value in line.split(',')]))
            # Создаём комплексное число (I + jQ)
            complex_number = complex(numbers[0], numbers[1])
            data.append(complex_number)
        except ValueError:
            print(f"Ошибка преобразования строки: {line}")

# Преобразуем список в массив NumPy
rx_nothreads = np.array(data)

# Вывод загруженных данных
plt.figure(3)
plt.scatter(rx_nothreads.real, rx_nothreads.imag)  # Используем scatter для диаграммы созвездия
plt.xlabel('I (Real)')
plt.ylabel('Q (Imaginary)')
plt.title('Принимаемое созвездие 4-QAM')
plt.grid(True)

plt.figure(4)
plt.title('Принимаемый сигнал real')
plt.plot(rx_nothreads.real)

plt.figure(5)
plt.title('Принимаемый сигнал imag')
plt.plot(rx_nothreads.imag)

plt.figure(6)
plt.title('Принимаемый сигнал')
plt.plot(rx_nothreads)

a, b = 0, 3000

rx_nothreads_norm = rx_nothreads[a:b]/2**11

fig, ax = plt.subplots(2, 2, figsize = (16, 8))
ax[0][0].plot(rx_nothreads_norm.real[0:150])
ax[0][0].set_title(r"$\Re$ part before matched filter")

ax[0][1].plot(rx_nothreads_norm.imag[0:150])
ax[0][1].set_title(r"$\Im$ part before matched filter")

rx_nothreads_norm_matched = np.convolve(rx_nothreads_norm, np.ones(16))


ax[1][0].plot(rx_nothreads_norm_matched.real[0:250], "g")
ax[1][0].set_title(r"$\Re$ part after matched filter")

ax[1][1].plot(rx_nothreads_norm_matched.imag[0:250], "g")
ax[1][1].set_title(r"$\Im$ part after matched filter")

# plt.figure(5)
# plt.title('Спектр rx')
# plt.plot(ifft(rx_nothreads))
# plt.show()

