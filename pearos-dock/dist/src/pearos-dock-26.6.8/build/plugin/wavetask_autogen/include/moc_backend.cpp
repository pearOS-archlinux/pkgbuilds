/****************************************************************************
** Meta object code from reading C++ file 'backend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../plugin/backend.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'backend.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7BackendE_t {};
} // unnamed namespace

template <> constexpr inline auto Backend::qt_create_metaobjectdata<qt_meta_tag_ZN7BackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Backend",
        "QML.Element",
        "auto",
        "addLauncher",
        "",
        "QUrl",
        "url",
        "showAllPlaces",
        "handleRecentDocumentAction",
        "jumpListActions",
        "QVariantList",
        "launcherUrl",
        "parent",
        "placesActions",
        "recentDocumentActions",
        "setActionGroup",
        "QAction*",
        "action",
        "setBlurBehind",
        "QWindow*",
        "window",
        "enable",
        "x",
        "y",
        "w",
        "h",
        "radius",
        "globalRect",
        "QRect",
        "QQuickItem*",
        "item",
        "isApplication",
        "parentPid",
        "pid",
        "tryDecodeApplicationsUrl",
        "applicationCategories",
        "MiddleClickAction",
        "None",
        "Close",
        "NewInstance",
        "ToggleMinimized",
        "ToggleGrouping",
        "BringToCurrentDesktop"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'addLauncher'
        QtMocHelpers::SignalData<void(const QUrl &) const>(3, 4, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Signal 'showAllPlaces'
        QtMocHelpers::SignalData<void()>(7, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'handleRecentDocumentAction'
        QtMocHelpers::SlotData<void() const>(8, 4, QMC::AccessPrivate, QMetaType::Void),
        // Method 'jumpListActions'
        QtMocHelpers::MethodData<QVariantList(const QUrl &, QObject *)>(9, 4, QMC::AccessPublic, 0x80000000 | 10, {{
            { 0x80000000 | 5, 11 }, { QMetaType::QObjectStar, 12 },
        }}),
        // Method 'placesActions'
        QtMocHelpers::MethodData<QVariantList(const QUrl &, bool, QObject *)>(13, 4, QMC::AccessPublic, 0x80000000 | 10, {{
            { 0x80000000 | 5, 11 }, { QMetaType::Bool, 7 }, { QMetaType::QObjectStar, 12 },
        }}),
        // Method 'recentDocumentActions'
        QtMocHelpers::MethodData<QVariantList(const QUrl &, QObject *)>(14, 4, QMC::AccessPublic, 0x80000000 | 10, {{
            { 0x80000000 | 5, 11 }, { QMetaType::QObjectStar, 12 },
        }}),
        // Method 'setActionGroup'
        QtMocHelpers::MethodData<void(QAction *) const>(15, 4, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Method 'setBlurBehind'
        QtMocHelpers::MethodData<void(QWindow *, bool, int, int, int, int, int)>(18, 4, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 }, { QMetaType::Bool, 21 }, { QMetaType::Int, 22 }, { QMetaType::Int, 23 },
            { QMetaType::Int, 24 }, { QMetaType::Int, 25 }, { QMetaType::Int, 26 },
        }}),
        // Method 'globalRect'
        QtMocHelpers::MethodData<QRect(QQuickItem *) const>(27, 4, QMC::AccessPublic, 0x80000000 | 28, {{
            { 0x80000000 | 29, 30 },
        }}),
        // Method 'isApplication'
        QtMocHelpers::MethodData<bool(const QUrl &) const>(31, 4, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Method 'parentPid'
        QtMocHelpers::MethodData<qint64(qint64) const>(32, 4, QMC::AccessPublic, QMetaType::LongLong, {{
            { QMetaType::LongLong, 33 },
        }}),
        // Method 'tryDecodeApplicationsUrl'
        QtMocHelpers::MethodData<QUrl(const QUrl &)>(34, 4, QMC::AccessPublic, 0x80000000 | 5, {{
            { 0x80000000 | 5, 11 },
        }}),
        // Method 'applicationCategories'
        QtMocHelpers::MethodData<QStringList(const QUrl &)>(35, 4, QMC::AccessPublic, QMetaType::QStringList, {{
            { 0x80000000 | 5, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'MiddleClickAction'
        QtMocHelpers::EnumData<enum MiddleClickAction>(36, 36, QMC::EnumFlags{}).add({
            {   37, MiddleClickAction::None },
            {   38, MiddleClickAction::Close },
            {   39, MiddleClickAction::NewInstance },
            {   40, MiddleClickAction::ToggleMinimized },
            {   41, MiddleClickAction::ToggleGrouping },
            {   42, MiddleClickAction::BringToCurrentDesktop },
        }),
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<Backend, void>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject Backend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7BackendE_t>.metaTypes,
    nullptr
} };

void Backend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Backend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->addLauncher((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 1: _t->showAllPlaces(); break;
        case 2: _t->handleRecentDocumentAction(); break;
        case 3: { QVariantList _r = _t->jumpListActions((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[2])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 4: { QVariantList _r = _t->placesActions((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[3])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 5: { QVariantList _r = _t->recentDocumentActions((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[2])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 6: _t->setActionGroup((*reinterpret_cast<std::add_pointer_t<QAction*>>(_a[1]))); break;
        case 7: _t->setBlurBehind((*reinterpret_cast<std::add_pointer_t<QWindow*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[6])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[7]))); break;
        case 8: { QRect _r = _t->globalRect((*reinterpret_cast<std::add_pointer_t<QQuickItem*>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QRect*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->isApplication((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { qint64 _r = _t->parentPid((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])));
            if (_a[0]) *reinterpret_cast<qint64*>(_a[0]) = std::move(_r); }  break;
        case 11: { QUrl _r = _t->tryDecodeApplicationsUrl((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QUrl*>(_a[0]) = std::move(_r); }  break;
        case 12: { QStringList _r = _t->applicationCategories((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QUrl & ) const>(_a, &Backend::addLauncher, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)()>(_a, &Backend::showAllPlaces, 1))
            return;
    }
}

const QMetaObject *Backend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Backend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Backend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void Backend::addLauncher(const QUrl & _t1)const
{
    QMetaObject::activate<void>(const_cast< Backend *>(this), &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Backend::showAllPlaces()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
