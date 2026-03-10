#include "contact_storage.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

namespace {

QString defaultStorageDir() {
    return QStringLiteral("/usr/share/extras/pearos-contacts");
}

QVector<ContactGroup> defaultGroups() {
    QVector<ContactGroup> groups;

    ContactGroup personal;
    personal.id = QStringLiteral("personal");
    personal.name = QObject::tr("Personal");
    personal.contacts = {
        {QObject::tr("John Doe"), QStringLiteral("+1 555 0100"), QStringLiteral("john@example.com"), {}},
        {QObject::tr("Jane Smith"), QStringLiteral("+1 555 0101"), QStringLiteral("jane@example.com"), {}},
    };
    for (Contact& c : personal.contacts) {
        c.id = c.displayName.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
    }

    ContactGroup work;
    work.id = QStringLiteral("work");
    work.name = QObject::tr("Work");

    ContactGroup family;
    family.id = QStringLiteral("family");
    family.name = QObject::tr("Family");

    groups.append(personal);
    groups.append(work);
    groups.append(family);
    return groups;
}

} // namespace

namespace ContactStorage {

QString storageDir() {
    return defaultStorageDir();
}

QVector<ContactGroup> load() {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        QDir().mkpath(dirPath);
    }

    QFile file(dir.filePath(QStringLiteral("contacts.json")));
    if (!file.exists()) {
        return defaultGroups();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return defaultGroups();
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return defaultGroups();
    }

    const QJsonObject root = doc.object();
    const QJsonArray groupsArray = root.value(QStringLiteral("groups")).toArray();
    QVector<ContactGroup> groups;
    groups.reserve(groupsArray.size());

    for (const QJsonValue& groupVal : groupsArray) {
        const QJsonObject groupObj = groupVal.toObject();
        ContactGroup group;
        group.id = groupObj.value(QStringLiteral("id")).toString();
        group.name = groupObj.value(QStringLiteral("name")).toString();
        const QJsonArray contactsArray = groupObj.value(QStringLiteral("contacts")).toArray();
        for (const QJsonValue& contactVal : contactsArray) {
            const QJsonObject contactObj = contactVal.toObject();
            Contact contact;
            contact.id = contactObj.value(QStringLiteral("id")).toString();
            contact.displayName = contactObj.value(QStringLiteral("displayName")).toString();
            contact.phone = contactObj.value(QStringLiteral("phone")).toString();
            contact.email = contactObj.value(QStringLiteral("email")).toString();
            contact.notes = contactObj.value(QStringLiteral("notes")).toString();
            if (!contact.displayName.trimmed().isEmpty()) {
                if (contact.id.isEmpty()) {
                    contact.id = contact.displayName.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
                }
                group.contacts.append(contact);
            }
        }
        if (group.id.isEmpty()) {
            group.id = group.name.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
        }
        if (!group.name.isEmpty()) {
            groups.append(group);
        }
    }

    if (groups.isEmpty()) {
        return defaultGroups();
    }
    return groups;
}

bool save(const QVector<ContactGroup>& groups) {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!QDir().mkpath(dirPath)) {
            return false;
        }
    }

    QJsonArray groupsArray;
    for (const ContactGroup& group : groups) {
        if (group.name.trimmed().isEmpty()) continue;
        QJsonObject groupObj;
        groupObj.insert(QStringLiteral("id"), group.id);
        groupObj.insert(QStringLiteral("name"), group.name);
        QJsonArray contactsArray;
        for (const Contact& contact : group.contacts) {
            if (contact.displayName.trimmed().isEmpty()) continue;
            QJsonObject contactObj;
            contactObj.insert(QStringLiteral("id"), contact.id);
            contactObj.insert(QStringLiteral("displayName"), contact.displayName);
            contactObj.insert(QStringLiteral("phone"), contact.phone);
            contactObj.insert(QStringLiteral("email"), contact.email);
            contactObj.insert(QStringLiteral("notes"), contact.notes);
            contactsArray.append(contactObj);
        }
        groupObj.insert(QStringLiteral("contacts"), contactsArray);
        groupsArray.append(groupObj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("groups"), groupsArray);
    const QJsonDocument doc(root);

    QFile file(dir.filePath(QStringLiteral("contacts.json")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace ContactStorage
