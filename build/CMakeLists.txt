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
