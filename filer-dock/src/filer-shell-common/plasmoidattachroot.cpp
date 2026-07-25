#include "plasmoidattachroot.h"

#include <QQmlContext>

PlasmoidShim *PlasmoidAttachedProvider::qmlAttachedProperties(QObject *object)
{
    // QQmlContext::contextProperty() only checks the exact context it's
    // called on -- it does NOT climb parent contexts (unlike ordinary QML
    // *expression* name resolution, e.g. `text: Plasmoid.title`, which is
    // why plain reads elsewhere in these files already worked fine before
    // this class existed). The object this attached-property block lives on
    // sits in a QQmlContext created internally for its own file/component, a
    // *child* of the context main.cpp set "Plasmoid" on when creating this
    // plasmoid -- so the lookup has to climb parentContext() manually here.
    for (QQmlContext *ctx = qmlContext(object); ctx; ctx = ctx->parentContext()) {
        const QVariant value = ctx->contextProperty(QStringLiteral("Plasmoid"));
        if (auto *shim = qobject_cast<PlasmoidShim *>(value.value<QObject *>())) {
            return shim;
        }
    }
    return nullptr;
}
