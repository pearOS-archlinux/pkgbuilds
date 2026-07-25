// File added by probono

#include "bundle.h"
#include "plistparser.h"
#include "icnsloader.h"

#include <QDebug>
#include <QDir>
#include <QIcon>

using namespace Fm;

namespace Fm {

// Return true if _info is an AppDir or .app bundle
bool checkWhetherAppDirOrBundle(FmFileInfo *_info) {

  bool isAppDirOrBundle = false;

  // TODO: Replace by using QFileInfo that we are using anyway below; get rid of
  // fm_...
  if ((fm_file_info_is_dir(_info) || fm_file_info_is_symlink(_info)) == false) {
    return (isAppDirOrBundle);
  }

  QString path = QString(fm_path_to_str(fm_file_info_get_path(_info)));

  // Resolve symlinks and use absolute path; this way symlinks to applications
  // are also recognized as applications even if they have a different suffix
  QDir appDirPath(path);
  path = appDirPath.canonicalPath();


  QFileInfo fileInfo = QFileInfo(QDir(path).canonicalPath());
  QString nameWithoutSuffix = QFileInfo(fileInfo.completeBaseName()).fileName();

  // NOTE: Checking for the prefix speeds up things significantly,
  // e.g., when checking whether /net is an AppDir

  // Check whether we have a GNUstep .app bundle
  if (path.toLower().endsWith(".app")) {
    // TODO: Before falling back to foo.app/foo, parse the
    // Info-gnustep.plist/Info.plist and get the NSExecutable from there as
    // described in
    // http://www.gnustep.org/resources/documentation/Developer/Gui/ProgrammingManual/AppKit_1.html
    QFile executableFile(path.toUtf8() + "/" + nameWithoutSuffix);
    if (QFileInfo(executableFile).isExecutable()) {
      isAppDirOrBundle = true;
    }

    // Check whether we have a macOS .app bundle
    QFile infoPlistFile(path.toUtf8() + "/Contents/Info.plist");
    QFile resourcesDirectory(path.toUtf8() + "/Contents/Resources");
    if (infoPlistFile.exists() && resourcesDirectory.exists()) {
      isAppDirOrBundle = true;
    }
  }

  // Check whether we have a ROX AppDir
  if (path.toLower().endsWith(".appdir")) {
    QFile appRunFile(path.toUtf8() + "/AppRun");
    if (QFileInfo(appRunFile).isExecutable()) {
      isAppDirOrBundle = true;
    }
  }

  // A .kext is a kernel extension bundle: it should be treated as an opaque
  // bundle like .app (not browsed into as a plain folder), but it is never
  // launchable -- getLaunchableExecutable() intentionally has no .kext
  // branch, so it always returns empty for these, which is what makes
  // isAppBundleNotLaunchable() flag them and FileLauncher show the
  // "no application" message instead of trying to run anything.
  if (path.toLower().endsWith(".kext")) {
    isAppDirOrBundle = true;
  }

  return (isAppDirOrBundle);
}

// Return the launchable executable of an AppDir or .app bundle, or an empty
// string if the bundle has no executable that can actually run on this
// platform (the caller is expected to tell the user in that case, rather
// than silently trying to exec something that will fail).
QString getLaunchableExecutable(FmFileInfo *_info) {
  QString launchableExecutable; // empty = not launchable on this platform

  QString path = QString(fm_path_to_str(fm_file_info_get_path(_info)));
  QFileInfo fileInfo = QFileInfo(path);
  QString nameWithoutSuffix = QFileInfo(fileInfo.completeBaseName()).fileName();

  // GNUstep .app bundle
  if (path.toLower().endsWith(".app")) {
    // TODO: Before falling back to foo.app/foo, parse the
    // Info-gnustep.plist and get the NSExecutable from there as
    // described in
    // http://www.gnustep.org/resources/documentation/Developer/Gui/ProgrammingManual/AppKit_1.html
    QFile executableFile(path.toUtf8() + "/" + nameWithoutSuffix);
    if (executableFile.exists() && QFileInfo(executableFile).isExecutable()) {
      launchableExecutable = QString(path + "/" + nameWithoutSuffix);
    }

    // macOS-convention .app bundle (has a Contents/ directory): pearOS
    // ports of macOS apps ship a native Linux binary at
    // Contents/pearOS/<exec>, alongside the original Contents/MacOS/<exec>
    // (a Mach-O binary, which cannot run on Linux at all). Always prefer
    // the pearOS binary; deliberately do NOT fall back to Contents/MacOS/
    // -- if no pearOS binary exists, the caller should tell the user this
    // app won't work here rather than silently trying to exec a Mach-O
    // binary.
    if (QFile::exists(path + "/Contents")) {
      QString plistPath = path + "/Contents/Info.plist";
      QString execName;
      if (QFile::exists(plistPath))
        execName = parseInfoPlist(plistPath).value("CFBundleExecutable");
      if (execName.isEmpty())
        execName = nameWithoutSuffix;

      QFile pearOSExecutableFile(path.toUtf8() + "/Contents/pearOS/" + execName.toUtf8());
      if (pearOSExecutableFile.exists() && QFileInfo(pearOSExecutableFile).isExecutable()) {
        launchableExecutable = QString(path + "/Contents/pearOS/" + execName);
      }
    }
  }

  // ROX AppDir
  if (path.toLower().endsWith(".appdir")) {
    QFile appRunFile(path.toUtf8() + "/AppRun");
    if ((appRunFile.exists()) && QFileInfo(appRunFile).isExecutable()) {
      launchableExecutable = QString(path + "/AppRun");
    }
  }

  return (launchableExecutable);
}

QIcon getIconForBundle(FmFileInfo *_info) {
  QString path = QString(fm_path_to_str(fm_file_info_get_path(_info)));
  QDir appDirPath(path);
  path = appDirPath.canonicalPath(); // Resolve symlinks and get absolute path

  // .kext bundles always use a fixed icon -- they have no Info.plist-driven
  // icon lookup (that's an app concept) and are never launchable, so none
  // of the .app/.AppDir icon-file searching below applies to them.
  if (path.toLower().endsWith(".kext")) {
    return QIcon(":/icons/kext.png");
  }

  QIcon icon = QIcon::fromTheme(
      "do"); // probono: In the elementary theme, this is a folder with an
             // executable icon inside it; TODO: Find more suitable one
  QFileInfo fileInfo = QFileInfo(path);
  QString nameWithoutSuffix = QFileInfo(fileInfo.completeBaseName()).fileName();
  // probono: GNUstep .app bundle
  // http://www.gnustep.org/resources/documentation/Developer/Gui/ProgrammingManual/AppKit_1.html
  // says: To determine the icon for a folder, if the folder has a ’.app’,
  // ’.debug’ or ’.profile’ extension - examine the Info.plist file for an
  // ’NSIcon’ value and try to use that. If there is no value specified - try
  // foo.app/foo.tiff’ or ’foo.app/.dir.tiff’
  // TODO: Implement plist parsing. For now we just check for
  // foo.app/Resources/foo.tiff’ and ’foo.app/.dir.tiff’ Actually there may be
  // foo.app/Resources/foo.desktop files which point to Icon= and we could use
  // that; just be sure to convert the absolute path there into a relative one?
  QFile tiffFile1(path.toUtf8() + "/Resources/" + nameWithoutSuffix.toUtf8() +
                  ".tiff");
  if (QFile::exists((QFileInfo(tiffFile1).canonicalFilePath()))) {
    icon = QIcon(QFileInfo(tiffFile1).canonicalFilePath());
  }
  QFile tiffFile2(path.toUtf8() + "/.dir.tiff");
  if (QFile::exists((QFileInfo(tiffFile2).canonicalFilePath()))) {
    icon = QIcon(QFileInfo(tiffFile2).canonicalFilePath());
  }
  QFile pngFile1(path.toUtf8() + "/Resources/" + nameWithoutSuffix.toUtf8() +
                 ".png");
  QFile svgFile1(path.toUtf8() + "/Resources/" + nameWithoutSuffix.toUtf8() +
                 ".svg");
  if (QFile::exists((QFileInfo(svgFile1).canonicalFilePath()))) {
    // icon = QIcon(QFileInfo(svgFile1).canonicalFilePath());
  }
  if (QFile::exists((QFileInfo(pngFile1).canonicalFilePath()))) {
    icon = QIcon(QFileInfo(pngFile1).canonicalFilePath());
  }

  // probono: ROX AppDir
  QFile dirIconFile(path.toUtf8() + "/.DirIcon");
  if (QFile::exists((QFileInfo(dirIconFile).canonicalFilePath()))) {
    icon = QIcon(QFileInfo(dirIconFile).canonicalFilePath());
  }

  // probono: macOS .app bundle: prefer CFBundleIconFile from Info.plist,
  // falling back to foo.app/Contents/Resources/foo.icns.
  QString plistPath = path + "/Contents/Info.plist";
  QString iconFile;
  if (QFile::exists(plistPath))
    iconFile = parseInfoPlist(plistPath).value("CFBundleIconFile");
  if (!iconFile.isEmpty()) {
    if (!iconFile.endsWith(".icns", Qt::CaseInsensitive))
      iconFile += ".icns";
    QFile plistIcnsFile(path.toUtf8() + "/Contents/Resources/" + iconFile.toUtf8());
    QString plistIcnsCanonical = QFileInfo(plistIcnsFile).canonicalFilePath();
    if (QFile::exists(plistIcnsCanonical)) {
      QIcon loaded = loadIcnsIcon(plistIcnsCanonical);
      if (!loaded.isNull())
        return loaded;
    }
  }
  QFile icnsFile(path.toUtf8() + "/Contents/Resources/" +
                 nameWithoutSuffix.toUtf8() + ".icns");
  QString icnsCanonical = QFileInfo(icnsFile).canonicalFilePath();
  if (QFile::exists(icnsCanonical)) {
    QIcon loaded = loadIcnsIcon(icnsCanonical);
    if (!loaded.isNull())
      icon = loaded;
  }
  return (icon);
}

bool isAppBundleNotLaunchable(FmFileInfo *_info) {
  if (!checkWhetherAppDirOrBundle(_info))
    return false;
  return getLaunchableExecutable(_info).isEmpty();
}

} // namespace Fm
