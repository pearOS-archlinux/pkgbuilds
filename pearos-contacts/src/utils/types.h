#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QVector>

struct Contact {
    QString id;
    QString displayName;
    QString phone;
    QString email;
    QString notes;

    Contact() = default;

    Contact(const QString& name, const QString& phoneNum = {}, const QString& emailAddr = {}, const QString& notesText = {})
        : displayName(name), phone(phoneNum), email(emailAddr), notes(notesText) {}
};

struct ContactGroup {
    QString id;
    QString name;
    QVector<Contact> contacts;
};

#endif // TYPES_H
