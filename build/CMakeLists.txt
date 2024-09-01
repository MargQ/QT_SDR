# Минимальная версия Qt, которая поддерживает Qt Charts
cmake_minimum_required(VERSION 3.5)
project(qt)

# Опция для включения или отключения установки зависимостей
option(INSTALL_DEPS "Установить зависимости" ON)

# Проверка наличия библиотеки libcurl
find_library(CURL_LIBRARY NAMES curl)
find_path(CURL_INCLUDE_DIR NAMES curl/curl.h)

if(CURL_LIBRARY AND CURL_INCLUDE_DIR)
    message(STATUS "libcurl уже установлен")
elseif(INSTALL_DEPS)
    message(STATUS "libcurl не найден, начинается установка...")

    # Обновление репозиториев
    execute_process(COMMAND sudo -S -E apt-get -y update)

    # Установка libcurl
    execute_process(
        COMMAND sudo -S -E apt-get -y install libcurl4-nss-dev
        RESULT_VARIABLE INSTALL_RESULT
    )

    if(INSTALL_RESULT EQUAL 0)
        message(STATUS "libcurl успешно установлен")
    else()
        message(FATAL_ERROR "Не удалось установить libcurl")
    endif()
else()
    message(FATAL_ERROR "libcurl не найден и установка зависимостей отключена")
endif()

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
find_package(Gnuradio REQUIRED)

# Исходные файлы проекта
set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    sdr.cpp
    sdrworker.cpp
)

set(HEADER_FILES
    mainwindow.h
    sdr.h
    sdrworker.h
)

# Путь до необходимых библиотек
include_directories(${PATH}/libiio)
link_directories(${PATH}/libiio)

include_directories(${PATH}/gnuradio)
link_directories(${PATH}/gnuradio)

# При необходимости CMake будет запускать moc
set(CMAKE_AUTOMOC ON)

# Добавляем исполняемый файл
add_executable(qt ${SOURCE_FILES})

# Линковка с библиотеками (Qt, iio)
target_link_libraries(qt Qt5::Core Qt5::Widgets Qt5::Charts iio gnuradio::gnuradio-runtime)

#include_directories(${Gnuradio_INCLUDE_DIRS})
#link_directories(${Gnuradio_LIBRARY_DIRS})

