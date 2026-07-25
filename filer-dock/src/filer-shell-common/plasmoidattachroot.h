#ifndef PLASMOIDATTACHROOT_H
#define PLASMOIDATTACHROOT_H

#include <QObject>
#include <QQmlEngine>

#include "plasmoidshim.h"

// Provides `Plasmoid` as a real QML *attached* property (registered under
// the type name "Plasmoid" itself, via QML_NAMED_ELEMENT) -- this is what
// makes top-level statements like `Plasmoid.title: "..."` inside a hosted
// plasmoid's root item resolve, exactly like real Plasma's
// org.kde.plasma.plasmoid module does. QML attached properties attach to
// *any* QObject regardless of its own C++ class -- there is no need for the
// enclosing root item to be a special type, which is why PlasmoidHost.qml's
// root is plain `Item`.
//
// qmlAttachedProperties() hands back the exact same PlasmoidShim instance
// main.cpp already installed as that file's `Plasmoid` *context* property,
// so both the attached syntax and ordinary expression reads (`text:
// Plasmoid.title`) see one identical object.
class PlasmoidAttachedProvider : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Plasmoid)
    QML_UNCREATABLE("Attached property only")
    QML_ATTACHED(PlasmoidShim)

public:
    static PlasmoidShim *qmlAttachedProperties(QObject *object);
};

#endif // PLASMOIDATTACHROOT_H
