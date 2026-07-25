/****************************************************************************
** Meta object code from reading C++ file 'placesview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/placesview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'placesview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__PlacesView_t {
    QByteArrayData data[22];
    char stringdata0[264];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__PlacesView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__PlacesView_t qt_meta_stringdata_Fm__PlacesView = {
    {
QT_MOC_LITERAL(0, 0, 14), // "Fm::PlacesView"
QT_MOC_LITERAL(1, 15, 14), // "chdirRequested"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 4), // "type"
QT_MOC_LITERAL(4, 36, 7), // "FmPath*"
QT_MOC_LITERAL(5, 44, 4), // "path"
QT_MOC_LITERAL(6, 49, 9), // "onClicked"
QT_MOC_LITERAL(7, 59, 11), // "QModelIndex"
QT_MOC_LITERAL(8, 71, 5), // "index"
QT_MOC_LITERAL(9, 77, 9), // "onPressed"
QT_MOC_LITERAL(10, 87, 17), // "onIconSizeChanged"
QT_MOC_LITERAL(11, 105, 4), // "size"
QT_MOC_LITERAL(12, 110, 12), // "onOpenNewTab"
QT_MOC_LITERAL(13, 123, 15), // "onOpenNewWindow"
QT_MOC_LITERAL(14, 139, 23), // "onShowInEnclosingFolder"
QT_MOC_LITERAL(15, 163, 19), // "onRemoveFromSidebar"
QT_MOC_LITERAL(16, 183, 9), // "onGetInfo"
QT_MOC_LITERAL(17, 193, 11), // "onAddToDock"
QT_MOC_LITERAL(18, 205, 13), // "onMountVolume"
QT_MOC_LITERAL(19, 219, 15), // "onUnmountVolume"
QT_MOC_LITERAL(20, 235, 13), // "onEjectVolume"
QT_MOC_LITERAL(21, 249, 14) // "onUnmountMount"

    },
    "Fm::PlacesView\0chdirRequested\0\0type\0"
    "FmPath*\0path\0onClicked\0QModelIndex\0"
    "index\0onPressed\0onIconSizeChanged\0"
    "size\0onOpenNewTab\0onOpenNewWindow\0"
    "onShowInEnclosingFolder\0onRemoveFromSidebar\0"
    "onGetInfo\0onAddToDock\0onMountVolume\0"
    "onUnmountVolume\0onEjectVolume\0"
    "onUnmountMount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__PlacesView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   89,    2, 0x09 /* Protected */,
       9,    1,   92,    2, 0x09 /* Protected */,
      10,    1,   95,    2, 0x09 /* Protected */,
      12,    0,   98,    2, 0x09 /* Protected */,
      13,    0,   99,    2, 0x09 /* Protected */,
      14,    0,  100,    2, 0x09 /* Protected */,
      15,    0,  101,    2, 0x09 /* Protected */,
      16,    0,  102,    2, 0x09 /* Protected */,
      17,    0,  103,    2, 0x09 /* Protected */,
      18,    0,  104,    2, 0x09 /* Protected */,
      19,    0,  105,    2, 0x09 /* Protected */,
      20,    0,  106,    2, 0x09 /* Protected */,
      21,    0,  107,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, QMetaType::QSize,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fm::PlacesView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlacesView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->chdirRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmPath*(*)>(_a[2]))); break;
        case 1: _t->onClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 2: _t->onPressed((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 3: _t->onIconSizeChanged((*reinterpret_cast< const QSize(*)>(_a[1]))); break;
        case 4: _t->onOpenNewTab(); break;
        case 5: _t->onOpenNewWindow(); break;
        case 6: _t->onShowInEnclosingFolder(); break;
        case 7: _t->onRemoveFromSidebar(); break;
        case 8: _t->onGetInfo(); break;
        case 9: _t->onAddToDock(); break;
        case 10: _t->onMountVolume(); break;
        case 11: _t->onUnmountVolume(); break;
        case 12: _t->onEjectVolume(); break;
        case 13: _t->onUnmountMount(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlacesView::*)(int , FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlacesView::chdirRequested)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::PlacesView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeView::staticMetaObject>(),
    qt_meta_stringdata_Fm__PlacesView.data,
    qt_meta_data_Fm__PlacesView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::PlacesView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::PlacesView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__PlacesView.stringdata0))
        return static_cast<void*>(this);
    return QTreeView::qt_metacast(_clname);
}

int Fm::PlacesView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void Fm::PlacesView::chdirRequested(int _t1, FmPath * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
