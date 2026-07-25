/********************************************************************************
** Form generated from reading UI file 'edit-bookmarks.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_2D_BOOKMARKS_H
#define UI_EDIT_2D_BOOKMARKS_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_EditBookmarksDialog
{
public:
    QGridLayout *gridLayout;
    QTreeWidget *treeWidget;
    QDialogButtonBox *buttonBox;
    QVBoxLayout *verticalLayout;
    QPushButton *addItem;
    QPushButton *removeItem;
    QSpacerItem *verticalSpacer;
    QLabel *label;

    void setupUi(QDialog *EditBookmarksDialog)
    {
        if (EditBookmarksDialog->objectName().isEmpty())
            EditBookmarksDialog->setObjectName(QString::fromUtf8("EditBookmarksDialog"));
        EditBookmarksDialog->resize(480, 320);
        gridLayout = new QGridLayout(EditBookmarksDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        treeWidget = new QTreeWidget(EditBookmarksDialog);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        treeWidget->setAcceptDrops(true);
        treeWidget->setDragEnabled(true);
        treeWidget->setDragDropMode(QAbstractItemView::InternalMove);
        treeWidget->setDefaultDropAction(Qt::MoveAction);
        treeWidget->setRootIsDecorated(false);
        treeWidget->setItemsExpandable(false);
        treeWidget->header()->setDefaultSectionSize(100);

        gridLayout->addWidget(treeWidget, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(EditBookmarksDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        addItem = new QPushButton(EditBookmarksDialog);
        addItem->setObjectName(QString::fromUtf8("addItem"));
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("list-add")));
        addItem->setIcon(icon);

        verticalLayout->addWidget(addItem);

        removeItem = new QPushButton(EditBookmarksDialog);
        removeItem->setObjectName(QString::fromUtf8("removeItem"));
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("list-remove")));
        removeItem->setIcon(icon1);

        verticalLayout->addWidget(removeItem);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        gridLayout->addLayout(verticalLayout, 1, 1, 1, 1);

        label = new QLabel(EditBookmarksDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(EditBookmarksDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), EditBookmarksDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), EditBookmarksDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(EditBookmarksDialog);
    } // setupUi

    void retranslateUi(QDialog *EditBookmarksDialog)
    {
        EditBookmarksDialog->setWindowTitle(QCoreApplication::translate("EditBookmarksDialog", "Edit Bookmarks", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(1, QCoreApplication::translate("EditBookmarksDialog", "Location", nullptr));
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("EditBookmarksDialog", "Name", nullptr));
        addItem->setText(QCoreApplication::translate("EditBookmarksDialog", "&Add Item", nullptr));
        removeItem->setText(QCoreApplication::translate("EditBookmarksDialog", "&Remove Item", nullptr));
        label->setText(QCoreApplication::translate("EditBookmarksDialog", "Use drag and drop to reorder the items", nullptr));
    } // retranslateUi

};

namespace Ui {
    class EditBookmarksDialog: public Ui_EditBookmarksDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_2D_BOOKMARKS_H
