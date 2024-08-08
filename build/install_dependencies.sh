#!/bin/bash

# Обновление пакетов
sudo apt update

# Функция для проверки пакета
package_exists() {
    dpkg -s "$1" &> /dev/null
}

# Список пакетов
required_packages=("libiio-dev" "libncurses5-dev" "libboost-program-options-dev" "libgpm-dev" "qtbase5-dev" "qtcharts5-dev")

# Установка пакетов
for package in "${required_packages[@]}"; do
    if ! package_exists "$package"; then
        sudo apt install -y "$package"
        if package_exists "$package"; then
            echo "$package успешно установлен."
        else
            echo "Ошибка при установке $package."
        fi
    else
        echo "$package уже установлен."
    fi
done

echo "Установка завершена."
