#include "icnsloader.h"

#include <QFile>
#include <QImage>
#include <QPixmap>
#include <cstring>

namespace Fm {

static const char kPngMagic[8] = {
  (char)0x89, 'P', 'N', 'G', '\r', '\n', (char)0x1a, '\n'
};

QIcon loadIcnsIcon(const QString& path) {
  QFile file(path);
  if(!file.open(QIODevice::ReadOnly))
    return QIcon();

  QByteArray data = file.readAll();
  if(data.size() < 8 || !data.startsWith("icns"))
    return QIcon();

  QImage best;
  int offset = 8; // skip "icns" + 4-byte total length
  while(offset + 8 <= data.size()) {
    quint32 chunkLength =
      (quint8(data[offset + 4]) << 24) | (quint8(data[offset + 5]) << 16) |
      (quint8(data[offset + 6]) << 8)  |  quint8(data[offset + 7]);
    if(chunkLength < 8 || offset + (int)chunkLength > data.size())
      break; // malformed/truncated chunk, stop rather than read out of bounds

    int payloadOffset = offset + 8;
    int payloadLength = (int)chunkLength - 8;
    if(payloadLength >= 8 && memcmp(data.constData() + payloadOffset, kPngMagic, 8) == 0) {
      QImage candidate = QImage::fromData(reinterpret_cast<const uchar*>(data.constData() + payloadOffset), payloadLength, "PNG");
      if(!candidate.isNull() && candidate.width() > best.width())
        best = candidate;
    }
    offset += (int)chunkLength;
  }

  if(best.isNull())
    return QIcon();
  return QIcon(QPixmap::fromImage(best));
}

}
