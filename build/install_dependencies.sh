#!/bin/bash

# Обновление списка пакетов
sudo apt update

# Установка необходимых пакетов
sudo apt install -y libiio-dev libncurses5-dev libboost-program-options-dev libgpm-dev libqt5charts5-dev

echo "Установка завершена."
