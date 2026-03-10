#ifndef TODO_WIDGET_H
#define TODO_WIDGET_H

#include <QWidget>
#include <QVector>
#include "../utils/types.h"

class QLabel;
class QScrollArea;
class QVBoxLayout;
class QLineEdit;

class TodoWidget : public QWidget {
    Q_OBJECT

public:
    explicit TodoWidget(QWidget* parent = nullptr);

    void setList(const TodoList& list);

signals:
    void itemToggled(int index, bool completed);
    void itemAdded(const QString& text);
    void itemDeleted(int index);

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_summaryLabel = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_itemsContainer = nullptr;
    QVBoxLayout* m_itemsLayout = nullptr;
    QLineEdit* m_newItemEdit = nullptr;

    QVector<TodoItem> m_items;

    void rebuildItems();
    void updateSummary();
};

#endif // TODO_WIDGET_H

