/****************************************************************************
** Meta object code from reading C++ file 'systeminfo.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/systeminfo.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'systeminfo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10SystemInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto SystemInfo::qt_create_metaobjectdata<qt_meta_tag_ZN10SystemInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SystemInfo",
        "infoChanged",
        "",
        "storageChanged",
        "refresh",
        "refreshStorage",
        "osName",
        "osVersion",
        "kernelVersion",
        "cpuModel",
        "cpuCores",
        "gpuModel",
        "totalRam",
        "usedRam",
        "totalDisk",
        "usedDisk",
        "diskName",
        "hostName",
        "serialNumber",
        "storageApps",
        "storageDocuments",
        "storagePhotos",
        "storageDownloads",
        "storageDesktop",
        "storageSystem"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'infoChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'storageChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshStorage'
        QtMocHelpers::MethodData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'osName'
        QtMocHelpers::PropertyData<QString>(6, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'osVersion'
        QtMocHelpers::PropertyData<QString>(7, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'kernelVersion'
        QtMocHelpers::PropertyData<QString>(8, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'cpuModel'
        QtMocHelpers::PropertyData<QString>(9, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'cpuCores'
        QtMocHelpers::PropertyData<int>(10, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'gpuModel'
        QtMocHelpers::PropertyData<QString>(11, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'totalRam'
        QtMocHelpers::PropertyData<qint64>(12, QMetaType::LongLong, QMC::DefaultPropertyFlags, 0),
        // property 'usedRam'
        QtMocHelpers::PropertyData<qint64>(13, QMetaType::LongLong, QMC::DefaultPropertyFlags, 0),
        // property 'totalDisk'
        QtMocHelpers::PropertyData<qint64>(14, QMetaType::LongLong, QMC::DefaultPropertyFlags, 0),
        // property 'usedDisk'
        QtMocHelpers::PropertyData<qint64>(15, QMetaType::LongLong, QMC::DefaultPropertyFlags, 0),
        // property 'diskName'
        QtMocHelpers::PropertyData<QString>(16, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'hostName'
        QtMocHelpers::PropertyData<QString>(17, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'serialNumber'
        QtMocHelpers::PropertyData<QString>(18, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'storageApps'
        QtMocHelpers::PropertyData<double>(19, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'storageDocuments'
        QtMocHelpers::PropertyData<double>(20, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'storagePhotos'
        QtMocHelpers::PropertyData<double>(21, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'storageDownloads'
        QtMocHelpers::PropertyData<double>(22, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'storageDesktop'
        QtMocHelpers::PropertyData<double>(23, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'storageSystem'
        QtMocHelpers::PropertyData<double>(24, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SystemInfo, qt_meta_tag_ZN10SystemInfoE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SystemInfo::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10SystemInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10SystemInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10SystemInfoE_t>.metaTypes,
    nullptr
} };

void SystemInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SystemInfo *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->infoChanged(); break;
        case 1: _t->storageChanged(); break;
        case 2: _t->refresh(); break;
        case 3: _t->refreshStorage(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SystemInfo::*)()>(_a, &SystemInfo::infoChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SystemInfo::*)()>(_a, &SystemInfo::storageChanged, 1))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->osName(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->osVersion(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->kernelVersion(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->cpuModel(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->cpuCores(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->gpuModel(); break;
        case 6: *reinterpret_cast<qint64*>(_v) = _t->totalRam(); break;
        case 7: *reinterpret_cast<qint64*>(_v) = _t->usedRam(); break;
        case 8: *reinterpret_cast<qint64*>(_v) = _t->totalDisk(); break;
        case 9: *reinterpret_cast<qint64*>(_v) = _t->usedDisk(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->diskName(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->hostName(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->serialNumber(); break;
        case 13: *reinterpret_cast<double*>(_v) = _t->storageApps(); break;
        case 14: *reinterpret_cast<double*>(_v) = _t->storageDocuments(); break;
        case 15: *reinterpret_cast<double*>(_v) = _t->storagePhotos(); break;
        case 16: *reinterpret_cast<double*>(_v) = _t->storageDownloads(); break;
        case 17: *reinterpret_cast<double*>(_v) = _t->storageDesktop(); break;
        case 18: *reinterpret_cast<double*>(_v) = _t->storageSystem(); break;
        default: break;
        }
    }
}

const QMetaObject *SystemInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SystemInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10SystemInfoE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SystemInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void SystemInfo::infoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SystemInfo::storageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
