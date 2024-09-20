 # Минимальная версия Qt, которая поддерживает Qt Charts
cmake_minimum_required(VERSION 3.5)
project(qt)

# Опция для включения или отключения установки зависимостей
option(INSTALL_DEPS "Установить зависимости" ON)

# Список библиотек для установки
set(DEPENDENCIES "libboost-all-dev" "libiio-dev" "libncurses5-dev" "libboost-program-options-dev" "libgpm-dev" "qtbase5-dev" "libqt5charts5-dev" "libspdlog-dev")

# Проверка наличия библиотек
foreach(DEP ${DEPENDENCIES})
    find_package(${DEP} QUIET)
    if(NOT ${DEP}_FOUND AND INSTALL_DEPS)
        message(STATUS "${DEP} не найден, начинается установка...")

        # Обновление репозиториев
        execute_process(COMMAND sudo -S -E apt-get -y update)

        # Установка библиотеки
        execute_process(
            COMMAND sudo -S -E apt-get -y install ${DEP}
            RESULT_VARIABLE INSTALL_RESULT
        )

        if(INSTALL_RESULT EQUAL 0)
            message(STATUS "${DEP} успешно установлен")
        else()
            message(FATAL_ERROR "Не удалось установить ${DEP}")
        endif()
    elseif(NOT ${DEP}_FOUND)
        message(FATAL_ERROR "${DEP} не найден и установка зависимостей отключена")
    else()
        message(STATUS "${DEP} уже установлен")
    endif()
endforeach()

include(FetchContent)
FetchContent_Declare(
  Gnuradio
  GIT_REPOSITORY https://github.com/gnuradio/gnuradio.git
  GIT_TAG        v3.10.11.0
)
FetchContent_MakeAvailable(Gnuradio)

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
find_package(Gnuradio QUIET)

if(NOT Gnuradio_FOUND)
    message(STATUS "Gnuradio не найден")
else()
    message(STATUS "Gnuradio уже установлен")
endif()

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

