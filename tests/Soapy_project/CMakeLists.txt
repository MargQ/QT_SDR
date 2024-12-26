cmake_minimum_required(VERSION 2.8.9)

project(soapy_pluto_sdr_timestamp)

find_package(SoapySDR REQUIRED)
#set(SoapySDR "/home/srsRAN/lib/SoapySDR/modules0.8/libPlutoSDRSupport.so")

if (SoapySDR_FOUND)
    message(STATUS "SoapySDR_INCLUDE_DIRS: ${SoapySDR_INCLUDE_DIRS}")
    message(STATUS "SoapySDR_LIBRARIES: ${SoapySDR_LIBRARIES}")
    message(STATUS "SoapySDR_DEFINITIONS: ${SoapySDR_DEFINITIONS}")
endif()

add_executable(soapy_pluto_sdr_timestamp soapy_pluto_sdr_timestamp.c)

target_link_libraries(soapy_pluto_sdr_timestamp ${SoapySDR_LIBRARIES})
