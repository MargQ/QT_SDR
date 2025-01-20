import numpy as np
import matplotlib.pyplot as plt

# Открываем файл для чтения
name = "/home/ub/Desktop/QT_SDR/tests/NOT_QT_project/build/rxdata.pcm"
#name = "/home/ub/Desktop/QT_SDR/src/samples/data/txdata_bark.pcm"


# Инициализируем список для хранения данных
data = []

# Чтение данных из файла
with open(name, "rb") as f:
    while True:
        # Считываем 4 байта (2 слова по 2 байта для I и Q)
        bytes_data = f.read(4)
        if not bytes_data:
            break
        
        # Преобразуем 2 байта в int16 (I)
        I = int.from_bytes(bytes_data[:2], byteorder='little', signed=True)
        # Преобразуем следующие 2 байта в int16 (Q)
        Q = int.from_bytes(bytes_data[2:4], byteorder='little', signed=True)
        
        # Добавляем комплексное число в список
        data.append(complex(I, Q))

# Преобразуем список в массив NumPy
rx_sig = np.array(data)



# Отображение исходных данных
plt.figure(1)
plt.plot(rx_sig.real, color='red', label="Real (I)")
plt.plot(rx_sig.imag, color='blue', label="Imag (Q)")
plt.xlabel("Sample Index")
plt.ylabel("Normalized Amplitude")
plt.legend()
plt.title("Original I/Q Data")

# Нормализация данных
rx_max = np.max(np.abs(rx_sig))
rx_sig = rx_sig / rx_max

# Отображение созвездия QPSK
plt.figure(2)
plt.scatter(rx_sig.real, rx_sig.imag, color='red')
plt.xlabel("Real (I)")
plt.ylabel("Imag (Q)")
plt.title("QPSK Constellation")
plt.grid()

# Открываем файл для чтения
name = "/home/ub/Desktop/QT_SDR/tests/NOT_QT_project/build/txdata.pcm"
#name = "/home/ub/Desktop/QT_SDR/src/samples/data/txdata_bark.pcm"


# Инициализируем список для хранения данных
data = []

# Чтение данных из файла
with open(name, "rb") as f:
    while True:
        # Считываем 4 байта (2 слова по 2 байта для I и Q)
        bytes_data = f.read(4)
        if not bytes_data:
            break
        
        # Преобразуем 2 байта в int16 (I)
        I = int.from_bytes(bytes_data[:2], byteorder='little', signed=True)
        # Преобразуем следующие 2 байта в int16 (Q)
        Q = int.from_bytes(bytes_data[2:4], byteorder='little', signed=True)
        
        # Добавляем комплексное число в список
        data.append(complex(I, Q))

# Преобразуем список в массив NumPy
rx_sig = np.array(data)



# Отображение исходных данных
plt.figure(3)
plt.plot(rx_sig.real, color='red', label="Real (I)")
plt.plot(rx_sig.imag, color='blue', label="Imag (Q)")
plt.xlabel("Sample Index")
plt.ylabel("Normalized Amplitude")
plt.legend()
plt.title("Original I/Q Data")

# # Нормализация данных
# rx_max = np.max(np.abs(rx_sig))
# rx_sig = rx_sig / rx_max

# Отображение созвездия QPSK
plt.figure(4)
plt.scatter(rx_sig.real, rx_sig.imag, color='red')
plt.xlabel("Real (I)")
plt.ylabel("Imag (Q)")
plt.title("QPSK Constellation")
plt.grid()

plt.show()