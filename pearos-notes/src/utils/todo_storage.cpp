#include "todo_storage.h"
#include "logger.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString defaultStorageDir() {
    return QStringLiteral("/usr/share/extras/pearos-notes");
}

QVector<NoteDocument> defaultNotes() {
    QVector<NoteDocument> notes;

    NoteDocument personal;
    personal.id = QStringLiteral("personal");
    personal.name = QObject::tr("Personal");
    personal.content = QObject::tr("Try pearOS Notes\n\nAdd your own notes");

    NoteDocument work;
    work.id = QStringLiteral("work");
    work.name = QObject::tr("Work");
    work.content.clear();

    NoteDocument shopping;
    shopping.id = QStringLiteral("shopping");
    shopping.name = QObject::tr("Shopping");
    shopping.content.clear();

    notes.append(personal);
    notes.append(work);
    notes.append(shopping);
    return notes;
}

} // namespace

namespace NotesStorage {

QString storageDir() {
    const QString dir = defaultStorageDir();
    Logger::info(QStringLiteral("Notes storage directory: %1").arg(dir));
    return dir;
}

QVector<NoteDocument> load() {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        Logger::warning(QStringLiteral("Notes storage directory does not exist, attempting to create: %1").arg(dirPath));
        if (!QDir().mkpath(dirPath)) {
            Logger::error(QStringLiteral("Failed to create notes storage directory: %1").arg(dirPath));
            // Nu putem crea directorul: revenim cu listă goală, fără notele default.
            return {};
        }
        Logger::info(QStringLiteral("Created notes storage directory: %1").arg(dirPath));
    }

    QFile file(dir.filePath(QStringLiteral("notes.json")));
    if (!file.exists()) {
        // Prima rulare: nu există fișier, afișăm notele default.
        Logger::info(QStringLiteral("No existing notes file, loading default notes (first run) from: %1").arg(file.fileName()));
        return defaultNotes();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::error(QStringLiteral("Failed to open notes file for reading: %1").arg(file.fileName()));
        // Avem fișier dar nu îl putem deschide: nu mai arătăm notele default,
        // ci listă goală ca să nu „rescriem” peste alegerea userului.
        return {};
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        Logger::error(QStringLiteral("Failed to parse notes JSON (%1) from: %2")
                          .arg(err.errorString(), file.fileName()));
        // JSON corupt: nu mai re-populăm cu notele default, ci pornim cu listă goală.
        return {};
    }

    const QJsonObject root = doc.object();
    const QJsonArray listsArray = root.value(QStringLiteral("lists")).toArray();
    QVector<NoteDocument> notes;
    notes.reserve(listsArray.size());

    for (const QJsonValue& listVal : listsArray) {
        const QJsonObject noteObj = listVal.toObject();
        NoteDocument note;
        note.id = noteObj.value(QStringLiteral("id")).toString();
        note.name = noteObj.value(QStringLiteral("name")).toString();
        note.content = noteObj.value(QStringLiteral("content")).toString();
        if (note.id.isEmpty()) {
            note.id = note.name.toLower().replace(' ', '-');
        }
        if (!note.name.isEmpty()) {
            notes.append(note);
        }
    }

    // Dacă vectorul e gol, respectăm alegerea userului (nici o notiță).
    Logger::info(QStringLiteral("Successfully loaded %1 notes from: %2")
                     .arg(QString::number(notes.size()), file.fileName()));
    return notes;
}

bool save(const QVector<NoteDocument>& notes) {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!QDir().mkpath(dirPath)) {
            Logger::error(QStringLiteral("Failed to create notes storage directory for saving: %1").arg(dirPath));
            return false;
        }
        Logger::info(QStringLiteral("Created notes storage directory for saving: %1").arg(dirPath));
    }

    QJsonArray notesArray;
    for (const NoteDocument& note : notes) {
        if (note.name.trimmed().isEmpty()) continue;
        QJsonObject noteObj;
        noteObj.insert(QStringLiteral("id"), note.id);
        noteObj.insert(QStringLiteral("name"), note.name);
        noteObj.insert(QStringLiteral("content"), note.content);
        notesArray.append(noteObj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("lists"), notesArray);
    const QJsonDocument doc(root);

    const QString filePath = dir.filePath(QStringLiteral("notes.json"));
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        Logger::error(QStringLiteral("Failed to open notes file for writing: %1").arg(filePath));
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    Logger::info(QStringLiteral("Saved %1 notes to: %2")
                     .arg(QString::number(notesArray.size()), filePath));
    return true;
}

} // namespace NotesStorage

