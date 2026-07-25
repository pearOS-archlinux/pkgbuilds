#ifndef FM_ICNSLOADER_H
#define FM_ICNSLOADER_H

#include <QIcon>
#include <QString>

namespace Fm {
// Minimal .icns reader. Qt ships no ICNS image-format plugin on most Linux
// installs, so QIcon(path)/QImage(path) silently fail to load .icns files.
// ICNS is a simple TLV container; since macOS 10.7, most of its per-size
// entries just embed a plain PNG, so we scan the chunks and decode the
// largest embedded PNG we find via QImage::fromData() instead of relying on
// a system codec for the container format itself.
QIcon loadIcnsIcon(const QString& path);
}

#endif // FM_ICNSLOADER_H
