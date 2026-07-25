/****************************************************************************
** Meta object code from reading C++ file 'desktoppreferencesdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/desktoppreferencesdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'desktoppreferencesdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__DesktopPreferencesDialog_t {
    QByteArrayData data[6];
    char stringdata0[92];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__DesktopPreferencesDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__DesktopPreferencesDialog_t qt_meta_stringdata_Filer__DesktopPreferencesDialog = {
    {
QT_MOC_LITERAL(0, 0, 31), // "Filer::DesktopPreferencesDialog"
QT_MOC_LITERAL(1, 32, 22), // "onWallpaperModeChanged"
QT_MOC_LITERAL(2, 55, 0), // ""
QT_MOC_LITERAL(3, 56, 5), // "index"
QT_MOC_LITERAL(4, 62, 15), // "onBrowseClicked"
QT_MOC_LITERAL(5, 78, 13) // "applySettings"

    },
    "Filer::DesktopPreferencesDialog\0"
    "onWallpaperModeChanged\0\0index\0"
    "onBrowseClicked\0applySettings"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__DesktopPreferencesDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x09 /* Protected */,
       4,    0,   32,    2, 0x09 /* Protected */,
       5,    0,   33,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Filer::DesktopPreferencesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DesktopPreferencesDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onWallpaperModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onBrowseClicked(); break;
        case 2: _t->applySettings(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Filer::DesktopPreferencesDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_Filer__DesktopPreferencesDialog.data,
    qt_meta_data_Filer__DesktopPreferencesDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::DesktopPreferencesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::DesktopPreferencesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__DesktopPreferencesDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int Filer::DesktopPreferencesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
