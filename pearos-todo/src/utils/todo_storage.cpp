#include "todo_storage.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString defaultStorageDir() {
    return QStringLiteral("/usr/share/extras/pearos-todo");
}

QVector<TodoList> defaultLists() {
    QVector<TodoList> lists;

    TodoList personal;
    personal.id = QStringLiteral("personal");
    personal.name = QObject::tr("Personal");
    personal.items = {
        {QObject::tr("Try pearOS ToDo"), false},
        {QObject::tr("Add your own tasks"), false},
    };

    TodoList work;
    work.id = QStringLiteral("work");
    work.name = QObject::tr("Work");

    TodoList shopping;
    shopping.id = QStringLiteral("shopping");
    shopping.name = QObject::tr("Shopping");

    lists.append(personal);
    lists.append(work);
    lists.append(shopping);
    return lists;
}

} // namespace

namespace TodoStorage {

QString storageDir() {
    return defaultStorageDir();
}

QVector<TodoList> load() {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        QDir().mkpath(dirPath);
    }

    QFile file(dir.filePath(QStringLiteral("todos.json")));
    if (!file.exists()) {
        return defaultLists();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return defaultLists();
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return defaultLists();
    }

    const QJsonObject root = doc.object();
    const QJsonArray listsArray = root.value(QStringLiteral("lists")).toArray();
    QVector<TodoList> lists;
    lists.reserve(listsArray.size());

    for (const QJsonValue& listVal : listsArray) {
        const QJsonObject listObj = listVal.toObject();
        TodoList list;
        list.id = listObj.value(QStringLiteral("id")).toString();
        list.name = listObj.value(QStringLiteral("name")).toString();
        const QJsonArray itemsArray = listObj.value(QStringLiteral("items")).toArray();
        for (const QJsonValue& itemVal : itemsArray) {
            const QJsonObject itemObj = itemVal.toObject();
            TodoItem item;
            item.text = itemObj.value(QStringLiteral("text")).toString();
            item.completed = itemObj.value(QStringLiteral("completed")).toBool(false);
            if (!item.text.trimmed().isEmpty()) {
                list.items.append(item);
            }
        }
        if (list.id.isEmpty()) {
            list.id = list.name.toLower().replace(' ', '-');
        }
        if (!list.name.isEmpty()) {
            lists.append(list);
        }
    }

    if (lists.isEmpty()) {
        return defaultLists();
    }
    return lists;
}

bool save(const QVector<TodoList>& lists) {
    const QString dirPath = storageDir();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!QDir().mkpath(dirPath)) {
            return false;
        }
    }

    QJsonArray listsArray;
    for (const TodoList& list : lists) {
        if (list.name.trimmed().isEmpty()) continue;
        QJsonObject listObj;
        listObj.insert(QStringLiteral("id"), list.id);
        listObj.insert(QStringLiteral("name"), list.name);
        QJsonArray itemsArray;
        for (const TodoItem& item : list.items) {
            if (item.text.trimmed().isEmpty()) continue;
            QJsonObject itemObj;
            itemObj.insert(QStringLiteral("text"), item.text);
            itemObj.insert(QStringLiteral("completed"), item.completed);
            itemsArray.append(itemObj);
        }
        listObj.insert(QStringLiteral("items"), itemsArray);
        listsArray.append(listObj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("lists"), listsArray);
    const QJsonDocument doc(root);

    QFile file(dir.filePath(QStringLiteral("todos.json")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace TodoStorage

