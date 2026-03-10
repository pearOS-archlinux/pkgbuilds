#ifndef TODO_STORAGE_H
#define TODO_STORAGE_H

#include "types.h"

namespace TodoStorage {

QString storageDir();
QVector<TodoList> load();
bool save(const QVector<TodoList>& lists);

} // namespace TodoStorage

#endif // TODO_STORAGE_H

