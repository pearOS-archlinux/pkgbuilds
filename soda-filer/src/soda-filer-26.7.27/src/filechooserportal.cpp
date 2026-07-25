#include "filechooserportal.h"
#include "opendialog.h"

#include <QDBusConnection>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>

namespace Filer {

FileChooserPortal::FileChooserPortal(QObject* parent):
    QObject(parent) {
}

// static
bool FileChooserPortal::registerPortal(QObject* parent) {
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if(!dbus.registerService(QStringLiteral("org.freedesktop.impl.portal.desktop.filer"))) {
        qWarning() << "FileChooserPortal: could not register portal service";
        return false;
    }
    FileChooserPortal* portal = new FileChooserPortal(parent);
    if(!dbus.registerObject(QStringLiteral("/org/freedesktop/portal/desktop"), portal,
                            QDBusConnection::ExportScriptableSlots)) {
        qWarning() << "FileChooserPortal: could not register portal object";
        return false;
    }
    qDebug() << "FileChooserPortal: registered org.freedesktop.impl.portal.desktop.filer";
    return true;
}

static FmPath* startPathFromOptions(const QVariantMap& options, const QString& lastDir) {
    // "current_folder" arrives as a NUL-terminated bytestring (ay)
    QByteArray folderBytes = options.value(QStringLiteral("current_folder")).toByteArray();
    while(folderBytes.endsWith('\0'))
        folderBytes.chop(1);
    QString folder = QString::fromUtf8(folderBytes);
    if(folder.isEmpty() || !QFileInfo(folder).isDir())
        folder = lastDir;
    if(folder.isEmpty() || !QFileInfo(folder).isDir())
        return NULL; // OpenDialog falls back to home
    FmPath* path = fm_path_new_for_str(folder.toUtf8().constData());
    return path;
}

uint FileChooserPortal::OpenFile(const QDBusObjectPath& handle, const QString& app_id,
                                 const QString& parent_window, const QString& title,
                                 const QVariantMap& options, QVariantMap& results) {
    Q_UNUSED(handle);
    Q_UNUSED(app_id);
    Q_UNUSED(parent_window);
    qDebug() << "FileChooserPortal::OpenFile" << title << options.keys();

    bool multiple = options.value(QStringLiteral("multiple")).toBool();
    bool directory = options.value(QStringLiteral("directory")).toBool();
    QString acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    // strip the mnemonic underscores portals use ("_Open")
    acceptLabel.remove(QLatin1Char('_'));

    FmPath* startPath = startPathFromOptions(options, lastDir_);
    OpenDialog dlg(startPath, OpenDialog::OpenMode, multiple, directory, title, acceptLabel);
    if(startPath)
        fm_path_unref(startPath);

    if(dlg.exec() != QDialog::Accepted || dlg.selectedUris().isEmpty())
        return 1; // cancelled

    QStringList uris = dlg.selectedUris();
    results.insert(QStringLiteral("uris"), uris);
    lastDir_ = QFileInfo(QUrl(uris.first()).toLocalFile()).absolutePath();
    return 0;
}

uint FileChooserPortal::SaveFile(const QDBusObjectPath& handle, const QString& app_id,
                                 const QString& parent_window, const QString& title,
                                 const QVariantMap& options, QVariantMap& results) {
    Q_UNUSED(handle);
    Q_UNUSED(app_id);
    Q_UNUSED(parent_window);
    qDebug() << "FileChooserPortal::SaveFile" << title << options.keys();

    QString acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    acceptLabel.remove(QLatin1Char('_'));

    FmPath* startPath = startPathFromOptions(options, lastDir_);
    OpenDialog dlg(startPath, OpenDialog::SaveMode, false, false, title, acceptLabel);
    if(startPath)
        fm_path_unref(startPath);
    dlg.setCurrentName(options.value(QStringLiteral("current_name")).toString());

    if(dlg.exec() != QDialog::Accepted || dlg.selectedUris().isEmpty())
        return 1; // cancelled

    QStringList uris = dlg.selectedUris();
    results.insert(QStringLiteral("uris"), uris);
    lastDir_ = QFileInfo(QUrl(uris.first()).toLocalFile()).absolutePath();
    return 0;
}

} // namespace Filer
