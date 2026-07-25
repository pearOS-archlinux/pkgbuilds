/*
 * xdg-desktop-portal FileChooser backend implementation
 * (org.freedesktop.impl.portal.FileChooser). When xdg-desktop-portal is
 * configured to prefer the "filer" portal for FileChooser, applications
 * that pick files through the portal (browsers' "upload from computer",
 * flatpaks, GTK_USE_PORTAL=1 apps...) get Filer's OpenDialog.
 */

#ifndef FILER_FILECHOOSERPORTAL_H
#define FILER_FILECHOOSERPORTAL_H

#include <QObject>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QVariantMap>

namespace Filer {

class FileChooserPortal : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.FileChooser")

public:
    explicit FileChooserPortal(QObject* parent = 0);

    // returns true if the service and object were registered on the bus
    static bool registerPortal(QObject* parent);

public Q_SLOTS:
    Q_SCRIPTABLE uint OpenFile(const QDBusObjectPath& handle, const QString& app_id,
                               const QString& parent_window, const QString& title,
                               const QVariantMap& options, QVariantMap& results);
    Q_SCRIPTABLE uint SaveFile(const QDBusObjectPath& handle, const QString& app_id,
                               const QString& parent_window, const QString& title,
                               const QVariantMap& options, QVariantMap& results);

private:
    QString lastDir_;
};

} // namespace Filer

#endif // FILER_FILECHOOSERPORTAL_H
