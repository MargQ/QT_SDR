# Минимальная версия Qt, которая поддерживает Qt Charts
cmake_minimum_required(VERSION 3.5)
project(qt)

# Опция для включения или отключения установки зависимостей
option(INSTALL_DEPS "Установить зависимости" ON)

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
# Пытаемся найти библиотеку fftw3 вручную
find_library(FFTW3_LIB fftw3 REQUIRED PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
# find_package(FFTW3 REQUIRED)

find_package(SoapySDR REQUIRED)

if (SoapySDR_FOUND)
    message(STATUS "SoapySDR_INCLUDE_DIRS: ${SoapySDR_INCLUDE_DIRS}")
    message(STATUS "SoapySDR_LIBRARIES: ${SoapySDR_LIBRARIES}")
    message(STATUS "SoapySDR_DEFINITIONS: ${SoapySDR_DEFINITIONS}")
endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Исходные файлы проекта
set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    sdr.cpp
    sdrworker.cpp
    qam.cpp
    oversample.cpp
    filter.cpp
)

set(HEADER_FILES
    mainwindow.h
    sdr.h
    sdrworker.h
    qam.h
    oversample.h
    filter.h
    ascii_art_dft.hpp
)

# Путь до необходимых библиотек
include_directories(${PATH}/libiio)
link_directories(${PATH}/libiio)
include_directories(/usr/include)
link_directories(/usr/lib)

# При необходимости CMake будет запускать moc
set(CMAKE_AUTOMOC ON)

# Установите уровень логирования SoapySDR
set(ENV{SOAPY_SDR_LOG_LEVEL} "error")

# Устанавливаем флаги компиляции для отладки при помощи Valgrind
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Добавляем исполняемый файл
add_executable(qt ${SOURCE_FILES})

# Включаем тесты для Valgrind
#enable_testing()
#add_test(NAME ValgrindTest COMMAND valgrind --leak-check=full --track-origins=yes ./qt)

# Линковка с библиотеками (Qt, iio)
target_link_libraries(qt Qt5::Core Qt5::Widgets Qt5::Charts iio)
target_link_libraries(qt ${FFTW3_LIB} ${SoapySDR_LIBRARIES})

