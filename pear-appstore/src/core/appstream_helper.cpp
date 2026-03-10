#include "appstream_helper.h"
#include "alpm_wrapper.h"
#include "../utils/logger.h"
#include "../utils/cache_helper.h"
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QLocale>
#include <QProcess>
#include <QDir>
#include <QBuffer>

QString AppStreamHelper::findMetainfoPath(const QString& packageName) {
    QStringList files = AlpmWrapper::instance().getPackageFileList(packageName);
    for (const QString& p : files) {
        if (p.endsWith(QLatin1String(".metainfo.xml"), Qt::CaseInsensitive)
            || p.endsWith(QLatin1String(".appdata.xml"), Qt::CaseInsensitive)) {
            QString full = p.startsWith(QLatin1String("/")) ? p : (QLatin1String("/") + p);
            if (QFile::exists(full)) {
                return full;
            }
        }
    }
    return QString();
}

AppStreamData AppStreamHelper::parseMetainfoFile(const QString& metainfoPath) {
    AppStreamData out;
    QFile f(metainfoPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::warning(QString("AppStream: cannot open %1").arg(metainfoPath));
        return out;
    }

    QXmlStreamReader xml(&f);
    QString currentLang;
    bool inDescription = false;
    QStringList descParagraphs;
    bool inScreenshot = false;
    bool inCategories = false;
    QString defaultLang = QLocale::system().name();
    if (defaultLang.contains(QLatin1Char('_'))) {
        defaultLang = defaultLang.left(defaultLang.indexOf(QLatin1Char('_')));
    }

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType tt = xml.readNext();
        if (tt == QXmlStreamReader::StartElement) {
            currentLang = xml.attributes().value(QLatin1String("xml:lang")).toString();
            if (currentLang.contains(QLatin1Char('_'))) {
                currentLang = currentLang.left(currentLang.indexOf(QLatin1Char('_')));
            }

            if (xml.name() == QLatin1String("summary")) {
                QString summary = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (!summary.isEmpty()) {
                    if (out.summary.isEmpty() || currentLang == defaultLang) {
                        out.summary = summary;
                    }
                }
            } else if (xml.name() == QLatin1String("description")) {
                inDescription = true;
                descParagraphs.clear();
            } else if (inDescription && xml.name() == QLatin1String("p")) {
                QString p = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (!p.isEmpty()) {
                    descParagraphs.append(p);
                }
            } else if (xml.name() == QLatin1String("icon")) {
                QString type = xml.attributes().value(QLatin1String("type")).toString();
                QString value = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (value.isEmpty()) continue;
                if (type == QLatin1String("local")) {
                    QString path = value;
                    if (!path.startsWith(QLatin1Char('/'))) {
                        path = QLatin1String("/") + path;
                    }
                    if (QFile::exists(path)) {
                        out.iconPath = path;
                    }
                } else if (type == QLatin1String("stock")) {
                    out.iconStockName = value;
                }
            } else if (xml.name() == QLatin1String("screenshot")) {
                inScreenshot = true;
            } else if (inScreenshot && xml.name() == QLatin1String("image")) {
                QString url = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (url.isEmpty()) {
                    url = xml.attributes().value(QLatin1String("source")).toString();
                }
                if (!url.isEmpty()) {
                    out.screenshotUrls.append(url);
                }
            } else if (xml.name() == QLatin1String("categories")) {
                inCategories = true;
            } else if (inCategories && xml.name() == QLatin1String("category")) {
                QString cat = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (!cat.isEmpty()) out.categories.append(cat);
            }
        } else if (tt == QXmlStreamReader::EndElement) {
            if (xml.name() == QLatin1String("description")) {
                inDescription = false;
                if (out.description.isEmpty() && !descParagraphs.isEmpty()) {
                    out.description = descParagraphs.join(QLatin1String("\n\n"));
                }
            } else if (xml.name() == QLatin1String("screenshot")) {
                inScreenshot = false;
            } else if (xml.name() == QLatin1String("categories")) {
                inCategories = false;
            }
        }
    }

    if (xml.hasError()) {
        Logger::warning(QString("AppStream parse error in %1: %2").arg(metainfoPath, xml.errorString()));
    }

    if (!descParagraphs.isEmpty() && out.description.isEmpty()) {
        out.description = descParagraphs.join(QLatin1String("\n\n"));
    }

    return out;
}

AppStreamData AppStreamHelper::parseMetainfoXml(const QByteArray& xml) {
    AppStreamData out;
    QBuffer buf;
    buf.setData(xml);
    if (!buf.open(QIODevice::ReadOnly)) return out;

    QXmlStreamReader reader(&buf);
    QString currentLang;
    bool inDescription = false;
    QStringList descParagraphs;
    bool inScreenshot = false;
    bool inCategories = false;
    QString defaultLang = QLocale::system().name();
    if (defaultLang.contains(QLatin1Char('_'))) {
        defaultLang = defaultLang.left(defaultLang.indexOf(QLatin1Char('_')));
    }

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType tt = reader.readNext();
        if (tt == QXmlStreamReader::StartElement) {
            currentLang = reader.attributes().value(QLatin1String("xml:lang")).toString();
            if (currentLang.contains(QLatin1Char('_'))) {
                currentLang = currentLang.left(currentLang.indexOf(QLatin1Char('_')));
            }
            if (reader.name() == QLatin1String("summary")) {
                QString summary = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (!summary.isEmpty() && (out.summary.isEmpty() || currentLang == defaultLang)) {
                    out.summary = summary;
                }
            } else if (reader.name() == QLatin1String("description")) {
                inDescription = true;
                descParagraphs.clear();
            } else if (inDescription && reader.name() == QLatin1String("p")) {
                QString p = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (!p.isEmpty()) descParagraphs.append(p);
            } else if (reader.name() == QLatin1String("icon")) {
                QString type = reader.attributes().value(QLatin1String("type")).toString();
                QString value = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (value.isEmpty()) continue;
                if (type == QLatin1String("local")) {
                    out.iconArchiveMember = value.startsWith(QLatin1Char('/')) ? value.mid(1) : value;
                } else if (type == QLatin1String("stock")) {
                    out.iconStockName = value;
                }
            } else if (reader.name() == QLatin1String("screenshot")) {
                inScreenshot = true;
            } else if (inScreenshot && reader.name() == QLatin1String("image")) {
                QString url = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (url.isEmpty()) url = reader.attributes().value(QLatin1String("source")).toString();
                if (!url.isEmpty()) out.screenshotUrls.append(url);
            } else if (reader.name() == QLatin1String("categories")) {
                inCategories = true;
            } else if (inCategories && reader.name() == QLatin1String("category")) {
                QString cat = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                if (!cat.isEmpty()) out.categories.append(cat);
            }
        } else if (tt == QXmlStreamReader::EndElement) {
            if (reader.name() == QLatin1String("description")) {
                inDescription = false;
                if (out.description.isEmpty() && !descParagraphs.isEmpty()) {
                    out.description = descParagraphs.join(QLatin1String("\n\n"));
                }
            } else if (reader.name() == QLatin1String("screenshot")) {
                inScreenshot = false;
            } else if (reader.name() == QLatin1String("categories")) {
                inCategories = false;
            }
        }
    }
    if (!descParagraphs.isEmpty() && out.description.isEmpty()) {
        out.description = descParagraphs.join(QLatin1String("\n\n"));
    }
    return out;
}

QString AppStreamHelper::extractIconFromPackage(const QString& pkgPath, const QString& memberPath, const QString& packageName) {
    QString cacheDir = CacheHelper::iconsDir();
    QDir().mkpath(cacheDir);
    QFileInfo fi(memberPath);
    QString ext = fi.suffix().isEmpty() ? QLatin1String("png") : fi.suffix();
    QString outPath = cacheDir + QLatin1Char('/') + packageName + QLatin1Char('.') + ext;

    QProcess proc;
    proc.setProgram(QLatin1String("bsdtar"));
    proc.setArguments({QLatin1String("-xOf"), pkgPath, memberPath});
    proc.setStandardOutputFile(outPath);
    proc.start(QProcess::ReadOnly);
    if (!proc.waitForFinished(5000) || proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        return QString();
    }
    return QFile::exists(outPath) ? outPath : QString();
}

AppStreamData AppStreamHelper::getDataFromCache(const QString& packageName) {
    QString filename = AlpmWrapper::instance().getPackageFilename(packageName);
    if (filename.isEmpty()) return AppStreamData();

    const QString cacheRoot = QLatin1String("/var/cache/pacman/pkg/");
    QString pkgPath = cacheRoot + filename;
    if (!QFile::exists(pkgPath)) return AppStreamData();

    QProcess listProc;
    listProc.setProgram(QLatin1String("bsdtar"));
    listProc.setArguments({QLatin1String("-tf"), pkgPath});
    listProc.start(QProcess::ReadOnly);
    if (!listProc.waitForFinished(3000) || listProc.exitCode() != 0) {
        return AppStreamData();
    }
    QByteArray listOut = listProc.readAllStandardOutput();
    QStringList lines = QString::fromUtf8(listOut).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QString metainfoMember;
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.endsWith(QLatin1String(".metainfo.xml"), Qt::CaseInsensitive)
            || trimmed.endsWith(QLatin1String(".appdata.xml"), Qt::CaseInsensitive)) {
            metainfoMember = trimmed;
            break;
        }
    }
    if (metainfoMember.isEmpty()) return AppStreamData();

    QProcess extractProc;
    extractProc.setProgram(QLatin1String("bsdtar"));
    extractProc.setArguments({QLatin1String("-xOf"), pkgPath, metainfoMember});
    extractProc.start(QProcess::ReadOnly);
    if (!extractProc.waitForFinished(5000) || extractProc.exitCode() != 0) {
        return AppStreamData();
    }
    QByteArray xml = extractProc.readAllStandardOutput();
    if (xml.isEmpty()) return AppStreamData();

    AppStreamData out = parseMetainfoXml(xml);
    if (!out.iconArchiveMember.isEmpty()) {
        QString localPath = extractIconFromPackage(pkgPath, out.iconArchiveMember, packageName);
        if (!localPath.isEmpty()) {
            out.iconPath = localPath;
        }
    }
    return out;
}

/** Copiază icoana de pe disc în cache; returnează calea din cache sau QString() la eșec. */
static QString copyLocalIconToCache(const QString& packageName, const QString& localPath) {
    if (localPath.isEmpty() || !localPath.startsWith(QLatin1Char('/')) || !QFile::exists(localPath)) {
        return QString();
    }
    QFileInfo fi(localPath);
    QString ext = fi.suffix();
    if (ext.isEmpty()) ext = QLatin1String("png");
    QString cacheDir = CacheHelper::iconsDir();
    QDir().mkpath(cacheDir);
    QString destPath = cacheDir + QLatin1Char('/') + packageName + QLatin1Char('.') + ext;
    if (QFile::exists(destPath)) return destPath;
    if (QFile::copy(localPath, destPath)) return destPath;
    return QString();
}

AppStreamData AppStreamHelper::getDataForPackage(const QString& packageName) {
    QString path = findMetainfoPath(packageName);
    AppStreamData out;
    if (!path.isEmpty()) {
        out = parseMetainfoFile(path);
        if (!out.iconPath.isEmpty() && out.iconPath.startsWith(QLatin1Char('/'))) {
            QString cached = copyLocalIconToCache(packageName, out.iconPath);
            if (!cached.isEmpty()) out.iconPath = cached;
        }
        return out;
    }
    return getDataFromCache(packageName);
}
