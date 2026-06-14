#ifndef NOTES_STORAGE_H
#define NOTES_STORAGE_H

#include "types.h"

namespace NotesStorage {

QString storageDir();
QVector<NoteDocument> load();
bool save(const QVector<NoteDocument>& notes);

} // namespace NotesStorage

#endif // NOTES_STORAGE_H

