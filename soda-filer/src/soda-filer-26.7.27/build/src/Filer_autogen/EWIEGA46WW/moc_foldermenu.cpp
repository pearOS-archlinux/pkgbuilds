/****************************************************************************
** Meta object code from reading C++ file 'foldermenu.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/foldermenu.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'foldermenu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__FolderMenu_t {
    QByteArrayData data[12];
    char stringdata0[272];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__FolderMenu_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__FolderMenu_t qt_meta_stringdata_Fm__FolderMenu = {
    {
QT_MOC_LITERAL(0, 0, 14), // "Fm::FolderMenu"
QT_MOC_LITERAL(1, 15, 22), // "onPasteActionTriggered"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 26), // "onSelectAllActionTriggered"
QT_MOC_LITERAL(4, 66, 32), // "onInvertSelectionActionTriggered"
QT_MOC_LITERAL(5, 99, 21), // "onSortActionTriggered"
QT_MOC_LITERAL(6, 121, 7), // "checked"
QT_MOC_LITERAL(7, 129, 26), // "onSortOrderActionTriggered"
QT_MOC_LITERAL(8, 156, 27), // "onShowHiddenActionTriggered"
QT_MOC_LITERAL(9, 184, 30), // "onCaseSensitiveActionTriggered"
QT_MOC_LITERAL(10, 215, 28), // "onFolderFirstActionTriggered"
QT_MOC_LITERAL(11, 244, 27) // "onPropertiesActionTriggered"

    },
    "Fm::FolderMenu\0onPasteActionTriggered\0"
    "\0onSelectAllActionTriggered\0"
    "onInvertSelectionActionTriggered\0"
    "onSortActionTriggered\0checked\0"
    "onSortOrderActionTriggered\0"
    "onShowHiddenActionTriggered\0"
    "onCaseSensitiveActionTriggered\0"
    "onFolderFirstActionTriggered\0"
    "onPropertiesActionTriggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__FolderMenu[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x09 /* Protected */,
       3,    0,   60,    2, 0x09 /* Protected */,
       4,    0,   61,    2, 0x09 /* Protected */,
       5,    1,   62,    2, 0x09 /* Protected */,
       7,    1,   65,    2, 0x09 /* Protected */,
       8,    1,   68,    2, 0x09 /* Protected */,
       9,    1,   71,    2, 0x09 /* Protected */,
      10,    1,   74,    2, 0x09 /* Protected */,
      11,    0,   77,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void,

       0        // eod
};

void Fm::FolderMenu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FolderMenu *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onPasteActionTriggered(); break;
        case 1: _t->onSelectAllActionTriggered(); break;
        case 2: _t->onInvertSelectionActionTriggered(); break;
        case 3: _t->onSortActionTriggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->onSortOrderActionTriggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onShowHiddenActionTriggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->onCaseSensitiveActionTriggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->onFolderFirstActionTriggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->onPropertiesActionTriggered(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::FolderMenu::staticMetaObject = { {
    QMetaObject::SuperData::link<QMenu::staticMetaObject>(),
    qt_meta_stringdata_Fm__FolderMenu.data,
    qt_meta_data_Fm__FolderMenu,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::FolderMenu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::FolderMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__FolderMenu.stringdata0))
        return static_cast<void*>(this);
    return QMenu::qt_metacast(_clname);
}

int Fm::FolderMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenu::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
