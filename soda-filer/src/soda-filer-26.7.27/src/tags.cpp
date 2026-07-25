#include "tags.h"
#include "extattrs.h"

namespace Fm {

static const QVector<TagInfo> kPalette = {
  { QStringLiteral("Red"),    QColor(255, 102, 92) },
  { QStringLiteral("Orange"), QColor(255, 190, 42) },
  { QStringLiteral("Yellow"), QColor(255, 244, 40) },
  { QStringLiteral("Green"),  QColor(86, 252, 113) },
  { QStringLiteral("Blue"),   QColor(45, 168, 255) },
  { QStringLiteral("Purple"), QColor(225, 123, 255) },
  { QStringLiteral("Gray"),   QColor(179, 181, 190) },
};

const QVector<TagInfo>& tagPalette() {
  return kPalette;
}

QColor tagColorForName(const QString& tagName) {
  for(const TagInfo& t : kPalette) {
    if(t.name == tagName)
      return t.color;
  }
  return QColor();
}

QString getFileTag(const QString& path, bool& ok) {
  QString raw = Fm::getAttributeValueQString(path, "filer.tag", ok);
  if(!ok || raw.isEmpty()) {
    ok = false;
    return QString();
  }
  return raw.section(':', 0, 0);
}

bool setFileTag(const QString& path, const QString& tagName) {
  QColor color = tagColorForName(tagName);
  if(!color.isValid())
    return false;
  return Fm::setAttributeValueQString(path, "filer.tag", tagName + ":" + color.name());
}

bool clearFileTag(const QString& path) {
  return Fm::removeAttributeValue(path, "filer.tag");
}

}
