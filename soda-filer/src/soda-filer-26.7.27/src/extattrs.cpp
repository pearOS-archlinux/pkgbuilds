#include "extattrs.h"

#include <sys/param.h> // for checking BSD definition
#if defined(BSD)
#include <sys/extattr.h>
#else
#include <sys/types.h>
#include <sys/xattr.h>
#include <string.h>
#include <errno.h>
#endif

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>

#define XATTR_NAMESPACE "user"

using namespace Fm;

namespace Fm {

static const int ATTR_VAL_SIZE = 20480; // FIXME: Can we do without a predetermined size?
// If this size is too small, then reading extattr fails, leading to strange unexpected errors
// including segfaults of 'launch', preventing the desktop from starting up
// 256 was not enough to read, e.g., 'can-open' containing many MIME types;
// 2048 was still not enough to handle, e.g., org.shotcut.Shotcut.desktop

/*
 * get the attibute value from the extended attribute for the path as int
 */
int getAttributeValueInt(const QString& path, const QString& attribute, bool& ok) {
  int value = 0;

  // get the value from the extended attribute for the path
  char data[ATTR_VAL_SIZE];
#if defined(BSD)
  ssize_t bytesRetrieved = extattr_get_file(path.toLatin1().data(), EXTATTR_NAMESPACE_USER,
                                                    attribute.toLatin1().data(), data, ATTR_VAL_SIZE);
#else
  QString namespacedAttr;
  namespacedAttr.append(XATTR_NAMESPACE).append(".").append(attribute);
  ssize_t bytesRetrieved = getxattr(path.toLatin1().data(),
                                            namespacedAttr.toLatin1().data(), data, ATTR_VAL_SIZE);
#endif
  // check if we got the attribute value
  if (bytesRetrieved <= 0)
    ok = false;
  else {
    // convert the value to int via QString
    QString strValue(data);
    bool intOK;
    int val = strValue.toInt(&intOK);
    if (intOK) {
      ok = true;
      value = val;
    }
  }
  return value;
}

/*
 * set the attibute value in the extended attribute for the path as int
 */
bool setAttributeValueInt(const QString& path, const QString& attribute, int value) {
    // set the value from the extended attribute for the path
    const QString data = QString::number(value);
    return setAttributeValueQString(path, attribute, data);
}

/*
 * get the attibute value from the extended attribute for the path as QString
 */
QString getAttributeValueQString(const QString& path, const QString& attribute, bool& ok) {
  // get the value from the extended attribute for the path
  char data[ATTR_VAL_SIZE];
#if defined(BSD)
  ssize_t bytesRetrieved = extattr_get_file(path.toLatin1().data(), EXTATTR_NAMESPACE_USER,
                                                    attribute.toLatin1().data(), data, ATTR_VAL_SIZE);
#else
  QString namespacedAttr;
  namespacedAttr.append(XATTR_NAMESPACE).append(".").append(attribute);
  ssize_t bytesRetrieved = getxattr(path.toLatin1().data(),
                                            namespacedAttr.toLatin1().data(), data, ATTR_VAL_SIZE);
#endif
  // check if we got the attribute value
  if (bytesRetrieved < 0) // If this is 0, then the value is empty but the extattr is set. If this is < 0, extattr is not set
    ok = false;
  else {
    // convert the value to QString
    data[bytesRetrieved] = 0;
    QString strValue;
    strValue = QString::fromStdString(data);
    strValue = strValue.trimmed();
    ok = true;
    return strValue;
  }
  return nullptr;
}

/*
 * set the attibute value in the extended attribute for the path as QString
 */
bool setAttributeValueQString(const QString& path, const QString& attribute, const QString& value) {
#if defined(BSD)
    QString candidateProgram = QStandardPaths::findExecutable("setextattr"); // FreeBSD
    if(candidateProgram.isEmpty()) {
        qCritical() << "Did not find setextattr, cannot set extended attribute";
        return false;
    }
    QProcess p;
    p.setProgram(QStandardPaths::findExecutable(candidateProgram));
    p.setArguments({"user", attribute, value, path});
    p.start();
    p.waitForFinished();
    if(p.exitCode() != 0) {
        qCritical() << "Failed to run command:" << p.program() << p.arguments();
        return false;
    }
    return true;
#else
    QString namespacedAttr;
    namespacedAttr.append(XATTR_NAMESPACE).append(".").append(attribute);
    QByteArray pathBytes = QDir::cleanPath(path).toLocal8Bit();
    QByteArray attrBytes = namespacedAttr.toLocal8Bit();
    QByteArray valueBytes = value.toUtf8();
    int success = setxattr(pathBytes.constData(),
                           attrBytes.constData(),
                           valueBytes.constData(),
                           valueBytes.length(),
                           0);
    if (success != 0) {
        qCritical() << "Failed to setxattr on" << path << "error:" << strerror(errno);
        return false;
    }
    return true;
#endif
}

/*
 * remove the attribute from the extended attributes of the path
 */
bool removeAttributeValue(const QString& path, const QString& attribute) {
#if defined(BSD)
    QString candidateProgram = QStandardPaths::findExecutable("rmextattr"); // FreeBSD
    if(candidateProgram.isEmpty()) {
        qCritical() << "Did not find rmextattr, cannot remove extended attribute";
        return false;
    }
    QProcess p;
    p.setProgram(QStandardPaths::findExecutable(candidateProgram));
    p.setArguments({"user", attribute, path});
    p.start();
    p.waitForFinished();
    if(p.exitCode() != 0) {
        qCritical() << "Failed to run command:" << p.program() << p.arguments();
        return false;
    }
    return true;
#else
    QString namespacedAttr;
    namespacedAttr.append(XATTR_NAMESPACE).append(".").append(attribute);
    QByteArray pathBytes = QDir::cleanPath(path).toLocal8Bit();
    QByteArray attrBytes = namespacedAttr.toLocal8Bit();
    int success = removexattr(pathBytes.constData(), attrBytes.constData());
    if (success != 0) {
        qCritical() << "Failed to removexattr on" << path << "error:" << strerror(errno);
        return false;
    }
    return true;
#endif
}

}

