/****************************************************************************
** Meta object code from reading C++ file 'dialogshim.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../filer-shell-common/dialogshim.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dialogshim.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10DialogShimE_t {};
} // unnamed namespace

template <> constexpr inline auto DialogShim::qt_create_metaobjectdata<qt_meta_tag_ZN10DialogShimE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DialogShim",
        "QML.Element",
        "ShellDialog",
        "mainItemChanged",
        "",
        "visualParentChanged",
        "locationChanged",
        "backgroundHintsChanged",
        "typeChanged",
        "hideOnWindowDeactivateChanged",
        "registerGlassRect",
        "token",
        "QRectF",
        "rect",
        "unregisterGlassRect",
        "mainItem",
        "QQuickItem*",
        "visualParent",
        "location",
        "backgroundHints",
        "type",
        "hideOnWindowDeactivate",
        "Popup",
        "AppletPopup"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'mainItemChanged'
        QtMocHelpers::SignalData<void()>(3, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'visualParentChanged'
        QtMocHelpers::SignalData<void()>(5, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'locationChanged'
        QtMocHelpers::SignalData<void()>(6, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'backgroundHintsChanged'
        QtMocHelpers::SignalData<void()>(7, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'typeChanged'
        QtMocHelpers::SignalData<void()>(8, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'hideOnWindowDeactivateChanged'
        QtMocHelpers::SignalData<void()>(9, 4, QMC::AccessPublic, QMetaType::Void),
        // Method 'registerGlassRect'
        QtMocHelpers::MethodData<void(QObject *, const QRectF &)>(10, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 11 }, { 0x80000000 | 12, 13 },
        }}),
        // Method 'unregisterGlassRect'
        QtMocHelpers::MethodData<void(QObject *)>(14, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'mainItem'
        QtMocHelpers::PropertyData<QQuickItem*>(15, 0x80000000 | 16, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 0),
        // property 'visualParent'
        QtMocHelpers::PropertyData<QQuickItem*>(17, 0x80000000 | 16, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 1),
        // property 'location'
        QtMocHelpers::PropertyData<int>(18, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'backgroundHints'
        QtMocHelpers::PropertyData<int>(19, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'type'
        QtMocHelpers::PropertyData<int>(20, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'hideOnWindowDeactivate'
        QtMocHelpers::PropertyData<bool>(21, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'Popup'
        QtMocHelpers::PropertyData<int>(22, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'AppletPopup'
        QtMocHelpers::PropertyData<int>(23, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<DialogShim, void>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject DialogShim::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10DialogShimE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10DialogShimE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10DialogShimE_t>.metaTypes,
    nullptr
} };

void DialogShim::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DialogShim *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->mainItemChanged(); break;
        case 1: _t->visualParentChanged(); break;
        case 2: _t->locationChanged(); break;
        case 3: _t->backgroundHintsChanged(); break;
        case 4: _t->typeChanged(); break;
        case 5: _t->hideOnWindowDeactivateChanged(); break;
        case 6: _t->registerGlassRect((*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QRectF>>(_a[2]))); break;
        case 7: _t->unregisterGlassRect((*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DialogShim::*)()>(_a, &DialogShim::mainItemChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DialogShim::*)()>(_a, &DialogShim::visualParentChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DialogShim::*)()>(_a, &DialogShim::locationChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DialogShim::*)()>(_a, &DialogShim::backgroundHintsChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DialogShim::*)()>(_a, &DialogShim::typeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DialogShim::*)()>(_a, &DialogShim::hideOnWindowDeactivateChanged, 5))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQuickItem* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QQuickItem**>(_v) = _t->mainItem(); break;
        case 1: *reinterpret_cast<QQuickItem**>(_v) = _t->visualParent(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->location(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->backgroundHints(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->type(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->hideOnWindowDeactivate(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->popupConstant(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->appletPopupConstant(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setMainItem(*reinterpret_cast<QQuickItem**>(_v)); break;
        case 1: _t->setVisualParent(*reinterpret_cast<QQuickItem**>(_v)); break;
        case 2: _t->setLocation(*reinterpret_cast<int*>(_v)); break;
        case 3: _t->setBackgroundHints(*reinterpret_cast<int*>(_v)); break;
        case 4: _t->setType(*reinterpret_cast<int*>(_v)); break;
        case 5: _t->setHideOnWindowDeactivate(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *DialogShim::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DialogShim::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10DialogShimE_t>.strings))
        return static_cast<void*>(this);
    return QQuickWindow::qt_metacast(_clname);
}

int DialogShim::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void DialogShim::mainItemChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DialogShim::visualParentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DialogShim::locationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DialogShim::backgroundHintsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DialogShim::typeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DialogShim::hideOnWindowDeactivateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
