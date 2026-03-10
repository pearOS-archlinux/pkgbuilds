#ifndef NOTES_WIDGET_H
#define NOTES_WIDGET_H

#include <QWidget>
#include "../utils/types.h"
#include <functional>

class QLabel;
class QHBoxLayout;
class QToolButton;
class QComboBox;
class QTextCharFormat;
class NotesEditor;

class NotesWidget : public QWidget {
    Q_OBJECT

public:
    explicit NotesWidget(QWidget* parent = nullptr);

    void setList(const NoteDocument& note);
    void setContent(const QString& content);

signals:
    void contentChanged(const QString& html);

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_summaryLabel = nullptr;
    NotesEditor* m_editor = nullptr;
    QToolButton* m_boldButton = nullptr;
    QToolButton* m_underlineButton = nullptr;
    QToolButton* m_strikeButton = nullptr;
    QComboBox* m_fontSizeBox = nullptr;

    bool m_blockContentSignal = false;

    void updateSummary();
    void setupToolbar(QHBoxLayout* toolbarLayout);
    void applyFormat(std::function<void(QTextCharFormat&)> formatter);
};

#endif // NOTES_WIDGET_H

