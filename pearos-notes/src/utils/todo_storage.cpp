#include "todo_storage.h"

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
    return defaultStorageDir();
}

QVector<NoteDocument> load() {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        QDir().mkpath(dirPath);
    }

    QFile file(dir.filePath(QStringLiteral("notes.json")));
    if (!file.exists()) {
        return defaultNotes();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return defaultNotes();
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return defaultNotes();
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

    if (notes.isEmpty()) {
        return defaultNotes();
    }
    return notes;
}

bool save(const QVector<NoteDocument>& notes) {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!QDir().mkpath(dirPath)) {
            return false;
        }
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

    QFile file(dir.filePath(QStringLiteral("notes.json")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace NotesStorage

