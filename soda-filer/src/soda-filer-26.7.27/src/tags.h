#ifndef FM_TAGS_H
#define FM_TAGS_H

#include <QColor>
#include <QString>
#include <QVector>

namespace Fm {

struct TagInfo {
  QString name;
  QColor color;
};

// Fixed macOS-Finder-like tag palette (color swatches assignable to a file).
const QVector<TagInfo>& tagPalette();

QColor tagColorForName(const QString& tagName);

// Reads/writes the "user.filer.tag" xattr, stored as "Name:#RRGGBB".
QString getFileTag(const QString& path, bool& ok);
bool setFileTag(const QString& path, const QString& tagName);
bool clearFileTag(const QString& path);

}

#endif // FM_TAGS_H
