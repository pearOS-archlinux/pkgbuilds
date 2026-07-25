// File added by probono

#ifndef BUNDLE_H
#define BUNDLE_H

#include <QFileInfo>
#include <libfm/fm.h>

namespace Fm {
bool checkWhetherAppDirOrBundle(FmFileInfo* _info);
QString getLaunchableExecutable(FmFileInfo* _info);
QIcon getIconForBundle(FmFileInfo* _info);
// True only for an app bundle/AppDir that has no executable runnable on
// this platform (e.g. a macOS .app shipping only a Mach-O binary, with no
// pearOS-native binary alongside it) -- used to flag such bundles in icon
// views instead of letting them look launchable and then silently failing.
bool isAppBundleNotLaunchable(FmFileInfo* _info);
}


#endif // BUNDLE_H
