cmake_minimum_required(VERSION 3.5)
project(qt)

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)

# Минимальная версия Qt, которая поддерживает Qt Charts
#find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)

# Исходные файлы проекта
set(SOURCES
    qt.cpp
)

# Путь до необходимых библиотек
include_directories(/home/ub/libiio)
link_directories(/home/ub/libiio)

# Добавляем исполняемый файл
add_executable(qt ${SOURCES})

# Линковка с библиотеками (Qt, iio)
target_link_libraries(qt Qt5::Widgets Qt5::Charts iio)
