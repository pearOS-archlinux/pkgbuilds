/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filelauncher.h"
#include "applaunchcontext.h"
#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include "execfiledialog_p.h"
#include "appchooserdialog.h"
#include "utilities.h"
#include "bundle.h"
#include "application.h"
#include "desktopwindow.h"
#include "recentfilesmanager.h"
#include "dmgmountregistry.h"

using namespace Fm;

FmFileLauncher FileLauncher::funcs = {
    FileLauncher::_getApp,
    /* gboolean (*before_open)(GAppLaunchContext* ctx, GList* folder_infos, gpointer user_data); */
    (FmLaunchFolderFunc)FileLauncher::_openFolder,
    FileLauncher::_execFile,
    FileLauncher::_error,
    FileLauncher::_ask
};

FileLauncher::FileLauncher():
    quickExec_(false) {
}

FileLauncher::~FileLauncher() {
}

//static
bool FileLauncher::launchFiles(QWidget* parent, GList* file_infos, bool show_contents) {
    // probono: This gets invoked when an icon is double clicked or "Open" is selected from the context menu
    // but not if "Open with..." is selected from the context menu
    // Here used to be a lot of libfm style code.
    // If this code causes trouble, check this function in
    // https://github.com/helloSystem/Filer/blob/c257312ca5b6039b21b32677a3b90bec6c34ee62/src/filelauncher.cpp#L53
    Filer::Application* app = static_cast<Filer::Application*>(qApp);
    for(GList* l = file_infos; l; l = l->next) {
        FmFileInfo* info = FM_FILE_INFO(l->data);
        bool isAppDirOrBundle = checkWhetherAppDirOrBundle(info);
        QString path = QString(fm_path_to_str(fm_file_info_get_path(info)));
        if(!fm_file_info_is_dir(info) && QFileInfo(path).fileName() != "trash-can.desktop") {
            Filer::RecentFilesManager::instance().addFile(path);
        }
        if(QFileInfo(path).fileName() == "trash-can.desktop"){
            // app->launchFiles(NULL, {"trash:///"}, true);
            app->launchFiles(NULL, { QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files"}, true); // TODO: Use QFileSystemEngine
        } else if (QFileInfo(path).isExecutable() && QFileInfo(path).isFile()
                   && !path.endsWith(".zip", Qt::CaseInsensitive)) {
            // .zip is excluded: archives on FAT/NTFS mounts carry a bogus
            // executable bit and must still reach the extraction branch below.
            qDebug() << "Launching using the 'launch' command";
            QProcess::startDetached("launch", {path});
        } else if(fm_file_info_is_dir(info) == true && (isAppDirOrBundle == false || show_contents == true)) {
            // Open folders directly (including "Show Contents" on a bundle,
            // which should browse it like a normal folder). Using Filer's
            // own internal launchFiles() rather than shelling out to the
            // external 'open' tool below avoids depending on that tool
            // being able to reach a running Filer instance over D-Bus --
            // when it can't, it falls back to opening a terminal instead.
            if (app->settings().spatialMode()) {
                app->launchFiles(NULL, {path}, true);
            }  else {
                // Route through this launcher's own openFolder() so a
                // launcher owned by a MainWindow can chdir in place;
                // Application::launchFiles() would construct a parentless
                // Launcher and always open a new window.
                GList* folderList = g_list_append(NULL, info);
                openFolder(NULL, folderList, NULL);
                g_list_free(folderList);
            }
        } else if(path.endsWith(".zip", Qt::CaseInsensitive)) {
            // Extract in place, macOS Archive Utility style:
            // a.zip -> folder "a" next to it, holding the archive contents.
            QFileInfo zipInfo(path);
            QDir parentDir = zipInfo.dir();
            QString baseName = zipInfo.completeBaseName();
            QString targetName = baseName;
            int suffix = 2;
            while(parentDir.exists(targetName))
                targetName = QString("%1 %2").arg(baseName).arg(suffix++);

            // Extract into a hidden staging dir so a half-finished extraction
            // never appears, then flatten a lone top-level folder (so a zip
            // that already wraps everything in one folder doesn't yield a/a).
            QString stagingPath = parentDir.filePath("." + targetName + ".extracting");
            QDir(stagingPath).removeRecursively();
            if(QProcess::execute("7z", {"x", "-y", path, "-o" + stagingPath}) == 0) {
                QDir staging(stagingPath);
                QStringList entries = staging.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
                if(entries.size() == 1 && QFileInfo(staging.filePath(entries.first())).isDir()) {
                    QDir().rename(staging.filePath(entries.first()), parentDir.filePath(targetName));
                    QDir(stagingPath).removeRecursively();
                } else {
                    QDir().rename(stagingPath, parentDir.filePath(targetName));
                }
                // On the desktop, pin the new folder slightly below-right of
                // the zip. pinNewItemNear() is a no-op on desktops that don't
                // contain the zip.
                Q_FOREACH(Filer::DesktopWindow* desktopWindow, app->desktopWindows()) {
                    desktopWindow->pinNewItemNear(zipInfo.fileName().toUtf8(),
                                                  targetName.toUtf8(), QPoint(10, 10));
                }
            } else {
                QDir(stagingPath).removeRecursively();
                QMessageBox::warning(parent, QObject::tr("Extract"),
                                     QObject::tr("Could not extract \"%1\".").arg(zipInfo.fileName()));
            }
        } else if (path.endsWith(".dmg", Qt::CaseInsensitive)) {
            // DMG handler logic
            QString dmgName = QFileInfo(path).baseName();
            QString mountPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/pearos_dmg_mounts/" + dmgName;
            
            // Clean up and recreate
            QProcess::execute("rm", {"-rf", mountPath});
            QProcess::execute("mkdir", {"-p", mountPath});
            
            // Extract the dmg
            qDebug() << "Extracting DMG to" << mountPath;
            QProcess::execute("7z", {"x", "-y", path, "-o" + mountPath});
            
            // Find the top-level directory extracted (usually the app name folder)
            QDir dir(mountPath);
            QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            QString finalPath = mountPath;
            if(!dirs.isEmpty()) {
                finalPath = mountPath + "/" + dirs.first();
            }
            
            // Delete HFS+ Private Data
            QDir(finalPath + "/[HFS+ Private Data]").removeRecursively();
            QDir(finalPath + "/.HFS+ Private Directory Data\r").removeRecursively();
            QFile::remove(finalPath + "/.VolumeIcon.icns");
            
            // Fix Applications symlink
            QFile appSymlink(finalPath + "/Applications");
            if(appSymlink.exists() || appSymlink.symLinkTarget().size() > 0) {
                appSymlink.remove();
                QString appsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
                if (appsPath.isEmpty()) appsPath = QDir::homePath() + "/Applications";
                QDir().mkpath(appsPath); // Ensure it exists
                QFile::link(appsPath, finalPath + "/Applications");
            }
            
            // Convert TIFF background to PNG if present
            QDir bgDir(finalPath + "/.background");
            if(bgDir.exists()) {
                QStringList tiffFiles = bgDir.entryList(QStringList() << "*.tiff" << "*.tif", QDir::Files);
                if(!tiffFiles.isEmpty()) {
                    QString tiffPath = bgDir.absoluteFilePath(tiffFiles.first());
                    QString pngPath = bgDir.absoluteFilePath("background.png");
                    if(!QFile::exists(pngPath)) {
                        QProcess::execute("convert", {tiffPath, pngPath});
                    }
                }
            }
            
            // Register with DmgMountRegistry so the sidebar (and the
            // desktop) can present this extraction as a mounted volume
            // with a real "Eject" action instead of an ordinary temp
            // folder nobody cleans up. Named after the bundled .app, like
            // a real DMG's volume label.
            QDir finalDir(finalPath);
            QStringList appDirs = finalDir.entryList(QStringList() << "*.app", QDir::Dirs);
            QString volumeName = !appDirs.isEmpty()
                ? QFileInfo(appDirs.first()).completeBaseName() : dmgName;

            // A real DMG shows the mounted volume itself as a desktop icon;
            // the closest we can do without a real block-device mount is a
            // symlink into the extracted folder, named after the volume, so
            // the existing (real-file-backed) desktop icon pipeline in
            // desktopwindow.cpp picks it up like any other file.
            QString desktopDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString desktopLinkPath = desktopDir + "/" + volumeName;
            QFile::remove(desktopLinkPath); // clear a stale link from a previous mount of the same name
            QFile::link(finalPath, desktopLinkPath);
            // Without this the symlink just inherits the plain folder icon.
            // GVFS metadata (the same mechanism Nautilus's "change folder
            // icon" uses) is a sidecar store independent of the underlying
            // filesystem, so this works on any local path without needing a
            // real GVolume/block-device mount.
            QProcess::execute("gio", {"set", desktopLinkPath, "metadata::custom-icon-name", "drive-removable-media"});

            Filer::DmgMountRegistry::instance().addMount(volumeName, finalPath, mountPath, desktopLinkPath);

            app->launchFiles(NULL, {finalPath}, true);

        } else if((isAppDirOrBundle == false or (show_contents == true)) && (! path.endsWith(".desktop"))) {
            qDebug() << "Opening using the 'open' command";
            QProcess::startDetached("open", {path});
        } else if(isAppDirOrBundle) {
            // Resolve and execute the bundle's binary directly (rather than
            // delegating to the external 'launch' tool, whose own bundle
            // handling we don't control) so a pearOS-native executable at
            // Contents/pearOS/<exec> is always preferred over the original
            // Contents/MacOS/<exec> Mach-O binary, which cannot run here.
            QString execPath = getLaunchableExecutable(info);
            if(!execPath.isEmpty()) {
                qDebug() << "Launching bundle executable directly:" << execPath;
                QProcess::startDetached(execPath, QStringList());
            }
            else if(path.endsWith(".kext", Qt::CaseInsensitive)) {
                // A kernel extension isn't an application at all -- there's
                // nothing that could ever "run" it, unlike a .app that just
                // needs a pearOS-native binary, so this gets its own message
                // rather than the macOS-porting one below.
                QMessageBox::warning(parent, QObject::tr("Cannot Open"),
                    QObject::tr("There is no application installed to open this document."));
            }
            else {
                QMessageBox::warning(parent, QObject::tr("Cannot Open Application"),
                    QObject::tr("This Application is meant for macOS, it will not work on pearOS"));
            }
        } else {
            qDebug() << "Launching using the 'launch' command";
            QProcess::startDetached("launch", {path});
        }
    }
    return true;
}

bool FileLauncher::launchPaths(QWidget* parent, GList* paths) {
    FmAppLaunchContext* context = fm_app_launch_context_new_for_widget(parent);
    bool ret = fm_launch_paths(G_APP_LAUNCH_CONTEXT(context), paths, &funcs, this);
    g_object_unref(context);
    return ret;
}

GAppInfo* FileLauncher::getApp(GList* file_infos, FmMimeType* mime_type, GError** err) {
    AppChooserDialog dlg(NULL);
    if(mime_type)
        dlg.setMimeType(mime_type);
    else
        dlg.setCanSetDefault(false);
    // FIXME: show error properly?
    if(execModelessDialog(&dlg) == QDialog::Accepted) {
        return dlg.selectedApp();
    }
    return NULL;
}

bool FileLauncher::openFolder(GAppLaunchContext* ctx, GList* folder_infos, GError** err) {

    for(GList* l = folder_infos; l; l = l->next) {
        FmFileInfo* fi = FM_FILE_INFO(l->data);
        qDebug() << "  folder:" << QString::fromUtf8(fm_file_info_get_disp_name(fi));
    }
    return false;
}

FmFileLauncherExecAction FileLauncher::execFile(FmFileInfo* file) {

    if (quickExec_) {
        /* SF bug#838: open terminal for each script may be just a waste.
       User should open a terminal and start the script there
       in case if user wants to see the script output anyway.
    if (fm_file_info_is_text(file))
        return FM_FILE_LAUNCHER_EXEC_IN_TERMINAL; */
        return FM_FILE_LAUNCHER_EXEC;
    }

    FmFileLauncherExecAction res = FM_FILE_LAUNCHER_EXEC_CANCEL;
    ExecFileDialog dlg(file);
    if(execModelessDialog(&dlg) == QDialog::Accepted) {
        res = dlg.result();
    }
    return res;
}

int FileLauncher::ask(const char* msg, char* const* btn_labels, int default_btn) {
    /* FIXME: set default button properly */
    // return fm_askv(data->parent, NULL, msg, btn_labels);
    return -1;
}

bool FileLauncher::error(GAppLaunchContext* ctx, GError* err, FmPath* path) {
    /* ask for mount if trying to launch unmounted path */
    if(err->domain == G_IO_ERROR) {
        if(path && err->code == G_IO_ERROR_NOT_MOUNTED) {
            //if(fm_mount_path(data->parent, path, TRUE))
            //  return FALSE; /* ask to retry */
        }
        else if(err->code == G_IO_ERROR_FAILED_HANDLED)
            return true; /* don't show error message */
    }
    QMessageBox dlg(QMessageBox::Critical, QObject::tr("Error"), QString::fromUtf8(err->message), QMessageBox::Ok);
    execModelessDialog(&dlg);
    return true;
}

