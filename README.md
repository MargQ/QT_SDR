## QT SDR

Раскладка виджетов в главном окне:

<img src = "files/widget.png">

Текущий результат:

<img src = "files/spectrum.gif">

## Необходимые библиотеки для запуска

```
sudo apt install libiio-dev libncurses5-dev libboost-program-options-dev libgpm-dev

sudo apt install libqt5charts5-dev

```

## Сборка проекта

```
cd QT_SDR/build
```
```
chmod 777 install_dependencies.sh 
```
```
cd ..
```

```
cmake build/
```
```
make
```
```
./qt
```
## Опции

Чтобы отключить установку зависимостей можно запустить CMake с параметром -DINSTALL_DEPENDENCIES=OFF

```
cmake -DINSTALL_DEPENDENCIES=OFF build/
```

По умолчанию он включен