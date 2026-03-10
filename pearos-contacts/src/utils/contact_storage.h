#ifndef CONTACT_STORAGE_H
#define CONTACT_STORAGE_H

#include "types.h"

namespace ContactStorage {

QString storageDir();
QVector<ContactGroup> load();
bool save(const QVector<ContactGroup>& groups);

} // namespace ContactStorage

#endif // CONTACT_STORAGE_H
