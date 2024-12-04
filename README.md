## QT SDR

Раскладка виджетов в главном окне:

<img src = "files/widget.png">

Текущий результат:

<img src = "files/spectrum.gif">

## Сборка проекта

Клонируем репозиторий:

```
git clone https://github.com/MargQ/QT_SDR
```

Переходим в папку с проектом и собираем его:

```
mkdir build
```

```
cd build
```

```
cmake ..
```
```
make
```

Запускаем:
```
./qt
```
## Опции

Чтобы отключить установку зависимостей можно запустить CMake с параметром -DINSTALL_DEPENDENCIES=OFF

```
cmake -DINSTALL_DEPENDENCIES=OFF build/
```

По умолчанию он включен
