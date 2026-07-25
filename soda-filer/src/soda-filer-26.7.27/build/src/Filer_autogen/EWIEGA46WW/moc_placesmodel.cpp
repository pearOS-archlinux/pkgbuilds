/****************************************************************************
** Meta object code from reading C++ file 'placesmodel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/placesmodel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'placesmodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__PlacesModel_t {
    QByteArrayData data[15];
    char stringdata0[172];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__PlacesModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__PlacesModel_t qt_meta_stringdata_Fm__PlacesModel = {
    {
QT_MOC_LITERAL(0, 0, 15), // "Fm::PlacesModel"
QT_MOC_LITERAL(1, 16, 11), // "updateIcons"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 11), // "updateTrash"
QT_MOC_LITERAL(4, 41, 14), // "setTagIconSize"
QT_MOC_LITERAL(5, 56, 4), // "size"
QT_MOC_LITERAL(6, 61, 18), // "setCategorySpacing"
QT_MOC_LITERAL(7, 80, 3), // "top"
QT_MOC_LITERAL(8, 84, 6), // "bottom"
QT_MOC_LITERAL(9, 91, 19), // "setCategoryFontSize"
QT_MOC_LITERAL(10, 111, 15), // "onDmgMountAdded"
QT_MOC_LITERAL(11, 127, 15), // "Filer::DmgMount"
QT_MOC_LITERAL(12, 143, 5), // "mount"
QT_MOC_LITERAL(13, 149, 17), // "onDmgMountRemoved"
QT_MOC_LITERAL(14, 167, 4) // "path"

    },
    "Fm::PlacesModel\0updateIcons\0\0updateTrash\0"
    "setTagIconSize\0size\0setCategorySpacing\0"
    "top\0bottom\0setCategoryFontSize\0"
    "onDmgMountAdded\0Filer::DmgMount\0mount\0"
    "onDmgMountRemoved\0path"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__PlacesModel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x0a /* Public */,
       3,    0,   50,    2, 0x0a /* Public */,
       4,    1,   51,    2, 0x0a /* Public */,
       6,    2,   54,    2, 0x0a /* Public */,
       9,    1,   59,    2, 0x0a /* Public */,
      10,    1,   62,    2, 0x0a /* Public */,
      13,    1,   65,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    7,    8,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, QMetaType::QString,   14,

       0        // eod
};

void Fm::PlacesModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlacesModel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateIcons(); break;
        case 1: _t->updateTrash(); break;
        case 2: _t->setTagIconSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setCategorySpacing((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->setCategoryFontSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onDmgMountAdded((*reinterpret_cast< const Filer::DmgMount(*)>(_a[1]))); break;
        case 6: _t->onDmgMountRemoved((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::PlacesModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QStandardItemModel::staticMetaObject>(),
    qt_meta_stringdata_Fm__PlacesModel.data,
    qt_meta_data_Fm__PlacesModel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::PlacesModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::PlacesModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__PlacesModel.stringdata0))
        return static_cast<void*>(this);
    return QStandardItemModel::qt_metacast(_clname);
}

int Fm::PlacesModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStandardItemModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
