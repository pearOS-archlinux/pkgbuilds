#include "contact_widget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QEvent>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>

ContactWidget::ContactWidget(QWidget* parent)
    : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setText(tr("Contacts"));
    mainLayout->addWidget(m_titleLabel);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet("color: #a1a1aa; font-size: 13px;");
    mainLayout->addWidget(m_summaryLabel);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_itemsContainer = new QWidget(this);
    m_itemsLayout = new QVBoxLayout(m_itemsContainer);
    m_itemsLayout->setContentsMargins(0, 16, 0, 0);
    m_itemsLayout->setSpacing(14);
    m_itemsLayout->addStretch();

    m_scrollArea->setWidget(m_itemsContainer);
    mainLayout->addWidget(m_scrollArea, 1);

    auto* addRow = new QHBoxLayout();
    addRow->setContentsMargins(0, 12, 0, 0);
    addRow->setSpacing(8);
    m_newNameEdit = new QLineEdit(this);
    m_newNameEdit->setPlaceholderText(tr("Name"));
    addRow->addWidget(m_newNameEdit, 1);
    m_newPhoneEdit = new QLineEdit(this);
    m_newPhoneEdit->setPlaceholderText(tr("Phone"));
    addRow->addWidget(m_newPhoneEdit, 1);
    m_newEmailEdit = new QLineEdit(this);
    m_newEmailEdit->setPlaceholderText(tr("Email"));
    addRow->addWidget(m_newEmailEdit, 1);
    m_addButton = new QPushButton(tr("Add"), this);
    m_addButton->setCursor(Qt::ArrowCursor);
    addRow->addWidget(m_addButton);
    mainLayout->addLayout(addRow);

    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        const QString name = m_newNameEdit->text().trimmed();
        if (name.isEmpty()) return;
        Contact c(name, m_newPhoneEdit->text().trimmed(), m_newEmailEdit->text().trimmed(), {});
        c.id = name.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
        emit contactAdded(c);
        m_newNameEdit->clear();
        m_newPhoneEdit->clear();
        m_newEmailEdit->clear();
    });

    updateSummary();
}

void ContactWidget::setGroup(const ContactGroup& group) {
    m_groupName = group.name;
    m_titleLabel->setText(group.name);
    m_contacts = group.contacts;
    rebuildItems();
    updateSummary();
}

void ContactWidget::rebuildItems() {
    while (m_itemsLayout->count() > 0) {
        QLayoutItem* item = m_itemsLayout->takeAt(0);
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    struct RowHoverFilter : QObject {
        QWidget* row = nullptr;
        QPushButton* deleteBtn = nullptr;
        QPushButton* editBtn = nullptr;
        bool isOver() const {
            if (!row) return false;
            const QPoint gpos = QCursor::pos();
            if (row->rect().contains(row->mapFromGlobal(gpos))) return true;
            if (deleteBtn && deleteBtn->rect().contains(deleteBtn->mapFromGlobal(gpos))) return true;
            if (editBtn && editBtn->rect().contains(editBtn->mapFromGlobal(gpos))) return true;
            return false;
        }
        bool eventFilter(QObject* o, QEvent* e) override {
            if (!row || (!deleteBtn && !editBtn)) return QObject::eventFilter(o, e);
            if (o == row || o == deleteBtn || o == editBtn) {
                if (e->type() == QEvent::Enter) {
                    if (deleteBtn) deleteBtn->setVisible(true);
                    if (editBtn) editBtn->setVisible(true);
                } else if (e->type() == QEvent::Leave) {
                    if (!isOver()) {
                        if (deleteBtn) deleteBtn->setVisible(false);
                        if (editBtn) editBtn->setVisible(false);
                    }
                }
            }
            return QObject::eventFilter(o, e);
        }
    };

    for (int i = 0; i < m_contacts.size(); ++i) {
        const Contact& contact = m_contacts.at(i);
        auto* rowWidget = new QWidget(m_itemsContainer);
        rowWidget->setAttribute(Qt::WA_Hover);
        auto* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 2, 0, 2);
        rowLayout->setSpacing(10);

        auto* nameLabel = new QLabel(contact.displayName, rowWidget);
        nameLabel->setStyleSheet("font-weight: 500;");
        nameLabel->setWordWrap(true);
        rowLayout->addWidget(nameLabel, 0);

        QString sub = contact.phone;
        if (!contact.email.isEmpty()) {
            if (!sub.isEmpty()) sub += QStringLiteral(" · ");
            sub += contact.email;
        }
        if (sub.isEmpty()) sub = tr("No phone or email");
        auto* subLabel = new QLabel(sub, rowWidget);
        subLabel->setStyleSheet("color: #a1a1aa; font-size: 13px;");
        subLabel->setWordWrap(true);
        rowLayout->addWidget(subLabel, 1);

        auto* btnContainer = new QWidget(rowWidget);
        btnContainer->setFixedSize(24, 24);
        auto* btnLayout = new QHBoxLayout(btnContainer);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        auto* editButton = new QPushButton(btnContainer);
        editButton->setToolTip(tr("Edit"));
        editButton->setFlat(true);
        editButton->setCursor(Qt::PointingHandCursor);
        editButton->setFixedSize(24, 24);
        QIcon editIcon(QStringLiteral(":/icons/edit.svg"));
        QPixmap editPx = editIcon.pixmap(18, 18);
        if (!editPx.isNull()) {
            QImage editImg = editPx.toImage().convertToFormat(QImage::Format_ARGB32);
            const QColor editColor(0xa1, 0xa1, 0xaa); // muted gray to match sidebar
            const int ea = editColor.alpha();
            const int er = editColor.red();
            const int eg = editColor.green();
            const int eb = editColor.blue();
            for (int y = 0; y < editImg.height(); ++y) {
                QRgb* line = reinterpret_cast<QRgb*>(editImg.scanLine(y));
                for (int x = 0; x < editImg.width(); ++x) {
                    const int alpha = qAlpha(line[x]);
                    line[x] = qRgba(er, eg, eb, (alpha * ea) / 255);
                }
            }
            editButton->setIcon(QPixmap::fromImage(editImg));
            editButton->setIconSize(QSize(18, 18));
        }
        editButton->setStyleSheet("QPushButton { background: transparent; border: none; }"
                                  "QPushButton:hover { background-color: rgba(113, 113, 122, 0.3); border-radius: 12px; }");
        editButton->setVisible(false);
        btnLayout->addWidget(editButton);

        auto* deleteContainer = new QWidget(rowWidget);
        deleteContainer->setFixedSize(24, 24);
        auto* deleteLayout = new QHBoxLayout(deleteContainer);
        deleteLayout->setContentsMargins(0, 0, 0, 0);
        auto* deleteButton = new QPushButton(deleteContainer);
        deleteButton->setFlat(true);
        deleteButton->setCursor(Qt::PointingHandCursor);
        deleteButton->setFixedSize(24, 24);
        QIcon delIcon(QStringLiteral(":/icons/delete.svg"));
        QPixmap px = delIcon.pixmap(18, 18);
        if (!px.isNull()) {
            QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
            const QColor color(0xF9, 0x73, 0x73);
            const int a = color.alpha();
            const int r = color.red();
            const int g = color.green();
            const int b = color.blue();
            for (int y = 0; y < img.height(); ++y) {
                QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
                for (int x = 0; x < img.width(); ++x) {
                    const int alpha = qAlpha(line[x]);
                    line[x] = qRgba(r, g, b, (alpha * a) / 255);
                }
            }
            deleteButton->setIcon(QPixmap::fromImage(img));
            deleteButton->setIconSize(QSize(18, 18));
        }
        deleteButton->setStyleSheet("QPushButton { background: transparent; border: none; }"
                                    "QPushButton:hover { background-color: rgba(248, 113, 113, 0.16); border-radius: 12px; }");
        deleteButton->setVisible(false);
        deleteLayout->addWidget(deleteButton);
        rowLayout->addWidget(btnContainer, 0);
        rowLayout->addWidget(deleteContainer, 0);

        connect(editButton, &QPushButton::clicked, this, [this, i]() {
            if (i < 0 || i >= m_contacts.size()) return;
            const Contact& c = m_contacts.at(i);
            QDialog dlg(this);
            dlg.setWindowTitle(tr("Edit contact"));
            auto* form = new QFormLayout(&dlg);
            QLineEdit* nameEdit = new QLineEdit(c.displayName, &dlg);
            QLineEdit* phoneEdit = new QLineEdit(c.phone, &dlg);
            QLineEdit* emailEdit = new QLineEdit(c.email, &dlg);
            QLineEdit* notesEdit = new QLineEdit(c.notes, &dlg);
            form->addRow(tr("Name:"), nameEdit);
            form->addRow(tr("Phone:"), phoneEdit);
            form->addRow(tr("Email:"), emailEdit);
            form->addRow(tr("Notes:"), notesEdit);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            form->addRow(buttons);
            connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            if (dlg.exec() != QDialog::Accepted) return;
            const QString name = nameEdit->text().trimmed();
            if (name.isEmpty()) return;
            Contact updated(name, phoneEdit->text().trimmed(), emailEdit->text().trimmed(), notesEdit->text().trimmed());
            updated.id = name.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
            emit contactUpdated(i, updated);
        });

        connect(deleteButton, &QPushButton::clicked, this, [this, i]() {
            if (i < 0 || i >= m_contacts.size()) return;
            emit contactDeleted(i);
        });

        auto* hoverFilter = new RowHoverFilter();
        hoverFilter->row = rowWidget;
        hoverFilter->deleteBtn = deleteButton;
        hoverFilter->editBtn = editButton;
        hoverFilter->setParent(rowWidget);
        rowWidget->installEventFilter(hoverFilter);
        deleteButton->installEventFilter(hoverFilter);
        editButton->installEventFilter(hoverFilter);

        m_itemsLayout->addWidget(rowWidget);
    }

    m_itemsLayout->addStretch();
}

void ContactWidget::updateSummary() {
    if (m_contacts.isEmpty()) {
        m_summaryLabel->setText(tr("No contacts in this group"));
        return;
    }
    m_summaryLabel->setText(tr("%1 contact(s)").arg(m_contacts.size()));
}
