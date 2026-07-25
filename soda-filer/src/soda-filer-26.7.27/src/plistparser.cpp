#include "plistparser.h"

#include <QFile>
#include <QXmlStreamReader>

namespace Fm {

QHash<QString, QString> parseInfoPlist(const QString& plistPath) {
  QHash<QString, QString> result;
  QFile file(plistPath);
  if(!file.open(QIODevice::ReadOnly))
    return result;

  QXmlStreamReader xml(&file);
  QString pendingKey;
  bool inDict = false;
  while(!xml.atEnd()) {
    xml.readNext();
    if(!xml.isStartElement())
      continue;
    const QString name = xml.name().toString();
    if(name == QLatin1String("dict")) {
      inDict = true;
      continue;
    }
    if(!inDict)
      continue;
    if(name == QLatin1String("key")) {
      pendingKey = xml.readElementText();
    }
    else if(name == QLatin1String("string") && !pendingKey.isEmpty()) {
      result[pendingKey] = xml.readElementText();
      pendingKey.clear();
    }
    else if(name != QLatin1String("plist")) {
      // array/dict/true/false/integer/real/date/data: skip this value's key.
      pendingKey.clear();
    }
  }
  return result;
}

} // namespace Fm
