<<<<<<< HEAD
# Минимальная версия Qt, которая поддерживает Qt Charts
cmake_minimum_required(VERSION 3.5)
project(qt)

# Опция для включения или отключения установки зависимостей
option(INSTALL_DEPENDENCIES "Install dependencies using a script" ON)

# Если опция включена, добавляем команду для запуска скрипта
if(INSTALL_DEPENDENCIES)
    add_custom_command(
        OUTPUT install_dependencies
        COMMAND ${CMAKE_SOURCE_DIR}/install_dependencies.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running install_dependencies.sh"
        VERBATIM
    )

    # Таргет для запуска скрипта
    add_custom_target(install_dependencies_target ALL
        DEPENDS install_dependencies
    )
endif() 

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)

# Исходные файлы проекта
set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    sdr.cpp
)

set(HEADER_FILES
    mainwindow.h
    sdr.h
)

# Путь до необходимых библиотек
include_directories(${PATH}/libiio)
link_directories(${PATH}/libiio)

# При необходимости CMake будет запускать moc
set(CMAKE_AUTOMOC ON)

# Добавляем исполняемый файл
add_executable(qt ${SOURCE_FILES})

# Линковка с библиотеками (Qt, iio)
target_link_libraries(qt Qt5::Core Qt5::Widgets Qt5::Charts iio)
=======
cmake_minimum_required(VERSION 3.5) 
project(qt) 
 
# Запуск bash скрипта  
add_custom_command( 
    OUTPUT install_dependencies 
    COMMAND ${CMAKE_SOURCE_DIR}/install_dependencies.sh 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
    COMMENT "Running install_dependencies.sh" 
    VERBATIM 
) 
 
# Таргет для запуска скрипта 
add_custom_target(install_dependencies_target ALL 
    DEPENDS install_dependencies 
) 
 
find_package(Qt5 COMPONENTS Widgets Charts REQUIRED) 
 
# Исходные файлы проекта 
set(SOURCES 
    qt.cpp 
) 
 
# Путь до библиотек 
include_directories(${PATH}/libiio) 
link_directories(${PATH}/libiio) 
 
# Добавляем исполняемый файл 
add_executable(qt ${SOURCES}) 
 
# Линковка с библиотеками (Qt, iio) 
target_link_libraries(qt Qt5::Widgets Qt5::Charts iio)
>>>>>>> d211dd0b00e4972be09b52b1d7d97b72fb6b2cbe
