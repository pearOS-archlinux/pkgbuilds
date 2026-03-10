#include "todo_widget.h"

#include <QCheckBox>
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

TodoWidget::TodoWidget(QWidget* parent)
    : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setText(tr("To‑Do"));
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
    m_newItemEdit = new QLineEdit(this);
    m_newItemEdit->setPlaceholderText(tr("Add a new task"));
    addRow->addWidget(m_newItemEdit, 1);

    auto* addButton = new QPushButton(tr("Add"), this);
    addButton->setCursor(Qt::ArrowCursor);
    addRow->addWidget(addButton);
    mainLayout->addLayout(addRow);

    connect(addButton, &QPushButton::clicked, this, [this]() {
        const QString text = m_newItemEdit->text().trimmed();
        if (text.isEmpty()) return;
        emit itemAdded(text);
        m_newItemEdit->clear();
    });
    connect(m_newItemEdit, &QLineEdit::returnPressed, [this]() {
        const QString text = m_newItemEdit->text().trimmed();
        if (text.isEmpty()) return;
        emit itemAdded(text);
        m_newItemEdit->clear();
    });

    updateSummary();
}

void TodoWidget::setList(const TodoList& list) {
    m_titleLabel->setText(list.name);
    m_items = list.items;
    rebuildItems();
    updateSummary();
}

void TodoWidget::rebuildItems() {
    // Remove all item widgets except the stretch at the end
    while (m_itemsLayout->count() > 0) {
        QLayoutItem* item = m_itemsLayout->takeAt(0);
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    struct RowHoverFilter : QObject {
        QWidget* row = nullptr;
        QPushButton* button = nullptr;
        bool isOver() const {
            if (!row || !button) return false;
            const QPoint gpos = QCursor::pos();
            if (row->rect().contains(row->mapFromGlobal(gpos))) return true;
            if (button->rect().contains(button->mapFromGlobal(gpos))) return true;
            return false;
        }
        bool eventFilter(QObject* o, QEvent* e) override {
            if (!row || !button) return QObject::eventFilter(o, e);
            if (o == row || o == button) {
                if (e->type() == QEvent::Enter) {
                    button->setVisible(true);
                } else if (e->type() == QEvent::Leave) {
                    if (!isOver())
                        button->setVisible(false);
                }
            }
            return QObject::eventFilter(o, e);
        }
    };

    for (int i = 0; i < m_items.size(); ++i) {
        const TodoItem& todo = m_items.at(i);
        auto* rowWidget = new QWidget(m_itemsContainer);
        rowWidget->setAttribute(Qt::WA_Hover);
        auto* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 2, 0, 2);
        rowLayout->setSpacing(10);

        auto* check = new QCheckBox(rowWidget);
        check->setChecked(todo.completed);
        rowLayout->addWidget(check, 0);

        auto* label = new QLabel(todo.text, rowWidget);
        label->setWordWrap(true);
        if (todo.completed) {
            label->setStyleSheet("color: #a1a1aa; text-decoration: line-through;");
        }
        rowLayout->addWidget(label, 1);

        // Delete button inside fixed-size container so layout doesn't move on hover
        auto* deleteContainer = new QWidget(rowWidget);
        deleteContainer->setFixedSize(24, 24);
        auto* deleteLayout = new QHBoxLayout(deleteContainer);
        deleteLayout->setContentsMargins(0, 0, 0, 0);
        deleteLayout->setSpacing(0);

        auto* deleteButton = new QPushButton(deleteContainer);
        deleteButton->setFlat(true);
        deleteButton->setCursor(Qt::PointingHandCursor);
        deleteButton->setFixedSize(24, 24);
        QIcon delIcon(QStringLiteral(":/icons/delete.svg"));
        QPixmap px = delIcon.pixmap(18, 18);
        if (!px.isNull()) {
            QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
            const QColor color(0xF9, 0x73, 0x73); // soft red
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
        rowLayout->addWidget(deleteContainer, 0);

        connect(check, &QCheckBox::toggled, this, [this, i, label](bool checked) {
            if (i < 0 || i >= m_items.size()) return;
            m_items[i].completed = checked;
            if (checked) {
                label->setStyleSheet("color: #a1a1aa; text-decoration: line-through;");
            } else {
                label->setStyleSheet("");
            }
            emit itemToggled(i, checked);
            updateSummary();
        });

        connect(deleteButton, &QPushButton::clicked, this, [this, i]() {
            if (i < 0 || i >= m_items.size()) return;
            emit itemDeleted(i);
        });

        auto* hoverFilter = new RowHoverFilter();
        hoverFilter->row = rowWidget;
        hoverFilter->button = deleteButton;
        hoverFilter->setParent(rowWidget);
        rowWidget->installEventFilter(hoverFilter);
        deleteButton->installEventFilter(hoverFilter);


        m_itemsLayout->addWidget(rowWidget);
    }

    m_itemsLayout->addStretch();
}

void TodoWidget::updateSummary() {
    if (m_items.isEmpty()) {
        m_summaryLabel->setText(tr("No tasks yet"));
        return;
    }
    int completedCount = 0;
    for (const TodoItem& item : m_items) {
        if (item.completed) ++completedCount;
    }
    m_summaryLabel->setText(tr("%1 completed").arg(completedCount));
}

