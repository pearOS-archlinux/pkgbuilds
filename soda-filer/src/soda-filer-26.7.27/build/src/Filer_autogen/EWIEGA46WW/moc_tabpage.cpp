/****************************************************************************
** Meta object code from reading C++ file 'tabpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/tabpage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tabpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__TabPage_t {
    QByteArrayData data[19];
    char stringdata0[236];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__TabPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__TabPage_t qt_meta_stringdata_Filer__TabPage = {
    {
QT_MOC_LITERAL(0, 0, 14), // "Filer::TabPage"
QT_MOC_LITERAL(1, 15, 13), // "statusChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 4), // "type"
QT_MOC_LITERAL(4, 35, 10), // "statusText"
QT_MOC_LITERAL(5, 46, 12), // "titleChanged"
QT_MOC_LITERAL(6, 59, 5), // "title"
QT_MOC_LITERAL(7, 65, 16), // "openDirRequested"
QT_MOC_LITERAL(8, 82, 7), // "FmPath*"
QT_MOC_LITERAL(9, 90, 4), // "path"
QT_MOC_LITERAL(10, 95, 6), // "target"
QT_MOC_LITERAL(11, 102, 17), // "sortFilterChanged"
QT_MOC_LITERAL(12, 120, 16), // "forwardRequested"
QT_MOC_LITERAL(13, 137, 17), // "backwardRequested"
QT_MOC_LITERAL(14, 155, 18), // "onOpenDirRequested"
QT_MOC_LITERAL(15, 174, 24), // "onModelSortFilterChanged"
QT_MOC_LITERAL(16, 199, 12), // "onSelChanged"
QT_MOC_LITERAL(17, 212, 6), // "numSel"
QT_MOC_LITERAL(18, 219, 16) // "restoreScrollPos"

    },
    "Filer::TabPage\0statusChanged\0\0type\0"
    "statusText\0titleChanged\0title\0"
    "openDirRequested\0FmPath*\0path\0target\0"
    "sortFilterChanged\0forwardRequested\0"
    "backwardRequested\0onOpenDirRequested\0"
    "onModelSortFilterChanged\0onSelChanged\0"
    "numSel\0restoreScrollPos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__TabPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       5,    1,   69,    2, 0x06 /* Public */,
       7,    2,   72,    2, 0x06 /* Public */,
      11,    0,   77,    2, 0x06 /* Public */,
      12,    0,   78,    2, 0x06 /* Public */,
      13,    0,   79,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    2,   80,    2, 0x09 /* Protected */,
      15,    0,   85,    2, 0x09 /* Protected */,
      16,    1,   86,    2, 0x09 /* Protected */,
      18,    0,   89,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void,

       0        // eod
};

void Filer::TabPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TabPage *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->statusChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->titleChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->openDirRequested((*reinterpret_cast< FmPath*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->sortFilterChanged(); break;
        case 4: _t->forwardRequested(); break;
        case 5: _t->backwardRequested(); break;
        case 6: _t->onOpenDirRequested((*reinterpret_cast< FmPath*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->onModelSortFilterChanged(); break;
        case 8: _t->onSelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->restoreScrollPos(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TabPage::*)(int , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TabPage::statusChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TabPage::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TabPage::titleChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TabPage::*)(FmPath * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TabPage::openDirRequested)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TabPage::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TabPage::sortFilterChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TabPage::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TabPage::forwardRequested)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (TabPage::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TabPage::backwardRequested)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Filer::TabPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Filer__TabPage.data,
    qt_meta_data_Filer__TabPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::TabPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::TabPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__TabPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Filer::TabPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Filer::TabPage::statusChanged(int _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Filer::TabPage::titleChanged(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Filer::TabPage::openDirRequested(FmPath * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Filer::TabPage::sortFilterChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Filer::TabPage::forwardRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Filer::TabPage::backwardRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
