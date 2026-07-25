/****************************************************************************
** Meta object code from reading C++ file 'viewoptionspopup.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/viewoptionspopup.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'viewoptionspopup.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__ViewOptionsPopup_t {
    QByteArrayData data[15];
    char stringdata0[199];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__ViewOptionsPopup_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__ViewOptionsPopup_t qt_meta_stringdata_Filer__ViewOptionsPopup = {
    {
QT_MOC_LITERAL(0, 0, 23), // "Filer::ViewOptionsPopup"
QT_MOC_LITERAL(1, 24, 14), // "stackByChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 5), // "index"
QT_MOC_LITERAL(4, 46, 13), // "sortByChanged"
QT_MOC_LITERAL(5, 60, 15), // "iconSizeChanged"
QT_MOC_LITERAL(6, 76, 4), // "size"
QT_MOC_LITERAL(7, 81, 18), // "gridSpacingChanged"
QT_MOC_LITERAL(8, 100, 7), // "spacing"
QT_MOC_LITERAL(9, 108, 15), // "textSizeChanged"
QT_MOC_LITERAL(10, 124, 20), // "labelPositionChanged"
QT_MOC_LITERAL(11, 145, 5), // "right"
QT_MOC_LITERAL(12, 151, 19), // "showItemInfoChanged"
QT_MOC_LITERAL(13, 171, 4), // "show"
QT_MOC_LITERAL(14, 176, 22) // "showIconPreviewChanged"

    },
    "Filer::ViewOptionsPopup\0stackByChanged\0"
    "\0index\0sortByChanged\0iconSizeChanged\0"
    "size\0gridSpacingChanged\0spacing\0"
    "textSizeChanged\0labelPositionChanged\0"
    "right\0showItemInfoChanged\0show\0"
    "showIconPreviewChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__ViewOptionsPopup[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,
       5,    1,   60,    2, 0x06 /* Public */,
       7,    1,   63,    2, 0x06 /* Public */,
       9,    1,   66,    2, 0x06 /* Public */,
      10,    1,   69,    2, 0x06 /* Public */,
      12,    1,   72,    2, 0x06 /* Public */,
      14,    1,   75,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Bool,   13,

       0        // eod
};

void Filer::ViewOptionsPopup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ViewOptionsPopup *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->stackByChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sortByChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->iconSizeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->gridSpacingChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->textSizeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->labelPositionChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->showItemInfoChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->showIconPreviewChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ViewOptionsPopup::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::stackByChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::sortByChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::iconSizeChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::gridSpacingChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::textSizeChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::labelPositionChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::showItemInfoChanged)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (ViewOptionsPopup::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ViewOptionsPopup::showIconPreviewChanged)) {
                *result = 7;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Filer::ViewOptionsPopup::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_Filer__ViewOptionsPopup.data,
    qt_meta_data_Filer__ViewOptionsPopup,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::ViewOptionsPopup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::ViewOptionsPopup::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__ViewOptionsPopup.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int Filer::ViewOptionsPopup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Filer::ViewOptionsPopup::stackByChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Filer::ViewOptionsPopup::sortByChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Filer::ViewOptionsPopup::iconSizeChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Filer::ViewOptionsPopup::gridSpacingChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Filer::ViewOptionsPopup::textSizeChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Filer::ViewOptionsPopup::labelPositionChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Filer::ViewOptionsPopup::showItemInfoChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Filer::ViewOptionsPopup::showIconPreviewChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
