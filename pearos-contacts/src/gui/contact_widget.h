#ifndef CONTACT_WIDGET_H
#define CONTACT_WIDGET_H

#include <QWidget>
#include <QVector>
#include "../utils/types.h"

class QLabel;
class QScrollArea;
class QVBoxLayout;
class QLineEdit;
class QPushButton;

class ContactWidget : public QWidget {
    Q_OBJECT

public:
    explicit ContactWidget(QWidget* parent = nullptr);

    void setGroup(const ContactGroup& group);

signals:
    void contactAdded(const Contact& contact);
    void contactUpdated(int index, const Contact& contact);
    void contactDeleted(int index);

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_summaryLabel = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_itemsContainer = nullptr;
    QVBoxLayout* m_itemsLayout = nullptr;
    QLineEdit* m_newNameEdit = nullptr;
    QLineEdit* m_newPhoneEdit = nullptr;
    QLineEdit* m_newEmailEdit = nullptr;
    QPushButton* m_addButton = nullptr;

    QVector<Contact> m_contacts;
    QString m_groupName;

    void rebuildItems();
    void updateSummary();
};

#endif // CONTACT_WIDGET_H
