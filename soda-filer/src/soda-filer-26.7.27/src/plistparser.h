#ifndef FM_PLISTPARSER_H
#define FM_PLISTPARSER_H

#include <QHash>
#include <QString>

namespace Fm {
// Minimal XML plist parser: reads the top-level <dict> as key/string pairs.
// Only <key> followed by <string> is captured; nested <array>/<dict> and
// non-string value types (<true/>, <false/>, <integer>, <real>, <date>,
// <data>) are skipped along with their key, since only flat string-valued
// keys such as CFBundleExecutable/CFBundleIconFile/CFBundleName are needed.
QHash<QString, QString> parseInfoPlist(const QString& plistPath);
}

#endif // FM_PLISTPARSER_H
