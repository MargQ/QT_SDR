/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[28];
    char stringdata0[293];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 8), // "fftshift"
QT_MOC_LITERAL(2, 20, 18), // "std::vector<float>"
QT_MOC_LITERAL(3, 39, 0), // ""
QT_MOC_LITERAL(4, 40, 4), // "data"
QT_MOC_LITERAL(5, 45, 14), // "updateSpectrum"
QT_MOC_LITERAL(6, 60, 14), // "const int16_t*"
QT_MOC_LITERAL(7, 75, 6), // "size_t"
QT_MOC_LITERAL(8, 82, 4), // "size"
QT_MOC_LITERAL(9, 87, 10), // "updateData"
QT_MOC_LITERAL(10, 98, 16), // "applySdrSettings"
QT_MOC_LITERAL(11, 115, 15), // "SoapySDRDevice*"
QT_MOC_LITERAL(12, 131, 3), // "sdr"
QT_MOC_LITERAL(13, 135, 11), // "std::string"
QT_MOC_LITERAL(14, 147, 9), // "ipAddress"
QT_MOC_LITERAL(15, 157, 6), // "gainTX"
QT_MOC_LITERAL(16, 164, 11), // "frequencyTX"
QT_MOC_LITERAL(17, 176, 12), // "sampleRateTX"
QT_MOC_LITERAL(18, 189, 11), // "bandwidthTX"
QT_MOC_LITERAL(19, 201, 6), // "gainRX"
QT_MOC_LITERAL(20, 208, 11), // "frequencyRX"
QT_MOC_LITERAL(21, 220, 12), // "sampleRateRX"
QT_MOC_LITERAL(22, 233, 7), // "size_t*"
QT_MOC_LITERAL(23, 241, 8), // "channels"
QT_MOC_LITERAL(24, 250, 13), // "channel_count"
QT_MOC_LITERAL(25, 264, 11), // "bandwidthRX"
QT_MOC_LITERAL(26, 276, 10), // "applyTheme"
QT_MOC_LITERAL(27, 287, 5) // "theme"

    },
    "MainWindow\0fftshift\0std::vector<float>\0"
    "\0data\0updateSpectrum\0const int16_t*\0"
    "size_t\0size\0updateData\0applySdrSettings\0"
    "SoapySDRDevice*\0sdr\0std::string\0"
    "ipAddress\0gainTX\0frequencyTX\0sampleRateTX\0"
    "bandwidthTX\0gainRX\0frequencyRX\0"
    "sampleRateRX\0size_t*\0channels\0"
    "channel_count\0bandwidthRX\0applyTheme\0"
    "theme"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   39,    3, 0x0a /* Public */,
       5,    2,   42,    3, 0x0a /* Public */,
       9,    2,   47,    3, 0x0a /* Public */,
      10,   12,   52,    3, 0x0a /* Public */,
      26,    1,   77,    3, 0x0a /* Public */,

 // slots: parameters
    0x80000000 | 2, 0x80000000 | 2,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 7,    4,    8,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 7,    4,    8,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 13, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, 0x80000000 | 22, 0x80000000 | 7, QMetaType::Double,   12,   14,   15,   16,   17,   18,   19,   20,   21,   23,   24,   25,
    QMetaType::Void, QMetaType::QString,   27,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { std::vector<float> _r = _t->fftshift((*reinterpret_cast< const std::vector<float>(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< std::vector<float>*>(_a[0]) = std::move(_r); }  break;
        case 1: _t->updateSpectrum((*reinterpret_cast< const int16_t*(*)>(_a[1])),(*reinterpret_cast< size_t(*)>(_a[2]))); break;
        case 2: _t->updateData((*reinterpret_cast< const int16_t*(*)>(_a[1])),(*reinterpret_cast< size_t(*)>(_a[2]))); break;
        case 3: _t->applySdrSettings((*reinterpret_cast< SoapySDRDevice*(*)>(_a[1])),(*reinterpret_cast< const std::string(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< double(*)>(_a[5])),(*reinterpret_cast< double(*)>(_a[6])),(*reinterpret_cast< double(*)>(_a[7])),(*reinterpret_cast< double(*)>(_a[8])),(*reinterpret_cast< double(*)>(_a[9])),(*reinterpret_cast< size_t*(*)>(_a[10])),(*reinterpret_cast< size_t(*)>(_a[11])),(*reinterpret_cast< double(*)>(_a[12]))); break;
        case 4: _t->applyTheme((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
