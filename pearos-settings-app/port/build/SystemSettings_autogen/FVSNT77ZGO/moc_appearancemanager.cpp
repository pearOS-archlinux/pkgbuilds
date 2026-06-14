/****************************************************************************
** Meta object code from reading C++ file 'appearancemanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/appearancemanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'appearancemanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17AppearanceManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto AppearanceManager::qt_create_metaobjectdata<qt_meta_tag_ZN17AppearanceManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AppearanceManager",
        "appearanceChanged",
        "",
        "refresh",
        "setColorScheme",
        "scheme",
        "setFontSize",
        "size",
        "setFontFamily",
        "family",
        "setIconTheme",
        "theme",
        "setLgEnabled",
        "enabled",
        "setLgBlurStrength",
        "v",
        "setLgNoiseStrength",
        "setLgRefractionStrength",
        "setLgRefractionEdgeSize",
        "setLgRGBFringing",
        "colorScheme",
        "fontSize",
        "fontFamily",
        "iconTheme",
        "colorSchemes",
        "QVariantList",
        "iconThemes",
        "lgEnabled",
        "lgBlurStrength",
        "lgNoiseStrength",
        "lgRefractionStrength",
        "lgRefractionEdgeSize",
        "lgRGBFringing"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'appearanceChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'setColorScheme'
        QtMocHelpers::MethodData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Method 'setFontSize'
        QtMocHelpers::MethodData<void(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'setFontFamily'
        QtMocHelpers::MethodData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Method 'setIconTheme'
        QtMocHelpers::MethodData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Method 'setLgEnabled'
        QtMocHelpers::MethodData<void(bool)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Method 'setLgBlurStrength'
        QtMocHelpers::MethodData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'setLgNoiseStrength'
        QtMocHelpers::MethodData<void(int)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'setLgRefractionStrength'
        QtMocHelpers::MethodData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'setLgRefractionEdgeSize'
        QtMocHelpers::MethodData<void(int)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'setLgRGBFringing'
        QtMocHelpers::MethodData<void(int)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'colorScheme'
        QtMocHelpers::PropertyData<QString>(20, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'fontSize'
        QtMocHelpers::PropertyData<QString>(21, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'fontFamily'
        QtMocHelpers::PropertyData<QString>(22, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'iconTheme'
        QtMocHelpers::PropertyData<QString>(23, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'colorSchemes'
        QtMocHelpers::PropertyData<QVariantList>(24, 0x80000000 | 25, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'iconThemes'
        QtMocHelpers::PropertyData<QVariantList>(26, 0x80000000 | 25, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'lgEnabled'
        QtMocHelpers::PropertyData<bool>(27, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'lgBlurStrength'
        QtMocHelpers::PropertyData<int>(28, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'lgNoiseStrength'
        QtMocHelpers::PropertyData<int>(29, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'lgRefractionStrength'
        QtMocHelpers::PropertyData<int>(30, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'lgRefractionEdgeSize'
        QtMocHelpers::PropertyData<int>(31, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'lgRGBFringing'
        QtMocHelpers::PropertyData<int>(32, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AppearanceManager, qt_meta_tag_ZN17AppearanceManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AppearanceManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AppearanceManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AppearanceManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17AppearanceManagerE_t>.metaTypes,
    nullptr
} };

void AppearanceManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AppearanceManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->appearanceChanged(); break;
        case 1: _t->refresh(); break;
        case 2: _t->setColorScheme((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->setFontSize((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->setFontFamily((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->setIconTheme((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->setLgEnabled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->setLgBlurStrength((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->setLgNoiseStrength((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->setLgRefractionStrength((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->setLgRefractionEdgeSize((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->setLgRGBFringing((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AppearanceManager::*)()>(_a, &AppearanceManager::appearanceChanged, 0))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->colorScheme(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->fontSize(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->fontFamily(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->iconTheme(); break;
        case 4: *reinterpret_cast<QVariantList*>(_v) = _t->colorSchemes(); break;
        case 5: *reinterpret_cast<QVariantList*>(_v) = _t->iconThemes(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->lgEnabled(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->lgBlurStrength(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->lgNoiseStrength(); break;
        case 9: *reinterpret_cast<int*>(_v) = _t->lgRefractionStrength(); break;
        case 10: *reinterpret_cast<int*>(_v) = _t->lgRefractionEdgeSize(); break;
        case 11: *reinterpret_cast<int*>(_v) = _t->lgRGBFringing(); break;
        default: break;
        }
    }
}

const QMetaObject *AppearanceManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppearanceManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AppearanceManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AppearanceManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void AppearanceManager::appearanceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
