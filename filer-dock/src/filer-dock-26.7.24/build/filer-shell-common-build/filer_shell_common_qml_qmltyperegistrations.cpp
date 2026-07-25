/****************************************************************************
** Generated QML type registration code
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtQml/qqml.h>
#include <QtQml/qqmlmoduleregistration.h>

#if __has_include(<dialogshim.h>)
#  include <dialogshim.h>
#endif
#if __has_include(<plasmatypesshim.h>)
#  include <plasmatypesshim.h>
#endif
#if __has_include(<plasmoidattachroot.h>)
#  include <plasmoidattachroot.h>
#endif


#if !defined(QT_STATIC)
#define Q_QMLTYPE_EXPORT Q_DECL_EXPORT
#else
#define Q_QMLTYPE_EXPORT
#endif
Q_QMLTYPE_EXPORT void qml_register_types_org_pearos_shellshim()
{
    QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED
    qmlRegisterTypesAndRevisions<DialogShim>("org.pearos.shellshim", 1);
    qmlRegisterAnonymousType<QQuickWindow, 254>("org.pearos.shellshim", 1);
    qmlRegisterAnonymousType<QWindow, 254>("org.pearos.shellshim", 1);
    {
        Q_CONSTINIT static auto metaType = QQmlPrivate::metaTypeForNamespace(
            [](const QtPrivate::QMetaTypeInterface *) {return &PlasmaTypesShim::staticMetaObject;},
            "PlasmaTypesShim");
        QMetaType(&metaType).id();
    }
    qmlRegisterNamespaceAndRevisions(&PlasmaTypesShim::staticMetaObject, "org.pearos.shellshim", 1, nullptr, &PlasmaTypesShim::staticMetaObject, nullptr);
    qmlRegisterEnum<PlasmaTypesShim::FormFactor>("PlasmaTypesShim::FormFactor");
    qmlRegisterEnum<PlasmaTypesShim::Location>("PlasmaTypesShim::Location");
    qmlRegisterEnum<PlasmaTypesShim::ItemStatus>("PlasmaTypesShim::ItemStatus");
    qmlRegisterEnum<PlasmaTypesShim::BackgroundHints>("PlasmaTypesShim::BackgroundHints");
    qmlRegisterEnum<PlasmaTypesShim::ContainmentDisplayHint>("PlasmaTypesShim::ContainmentDisplayHint");
    qmlRegisterTypesAndRevisions<PlasmoidAttachedProvider>("org.pearos.shellshim", 1);
    QT_WARNING_POP
    qmlRegisterModule("org.pearos.shellshim", 1, 0);
}

static const QQmlModuleRegistration orgpearosshellshimRegistration("org.pearos.shellshim", qml_register_types_org_pearos_shellshim);
