/********************************************************************************
** Form generated from reading UI file 'rename-dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RENAME_2D_DIALOG_H
#define UI_RENAME_2D_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_RenameDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QGridLayout *gridLayout;
    QLabel *destIcon;
    QLabel *label_3;
    QLabel *srcInfo;
    QLabel *destInfo;
    QLabel *srcIcon;
    QHBoxLayout *horizontalLayout;
    QLabel *label_6;
    QLineEdit *fileName;
    QCheckBox *applyToAll;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *RenameDialog)
    {
        if (RenameDialog->objectName().isEmpty())
            RenameDialog->setObjectName(QString::fromUtf8("RenameDialog"));
        RenameDialog->resize(398, 220);
        RenameDialog->setSizeGripEnabled(false);
        verticalLayout = new QVBoxLayout(RenameDialog);
#ifndef Q_OS_MAC
        verticalLayout->setSpacing(6);
#endif
        verticalLayout->setContentsMargins(10, 10, 10, 10);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(RenameDialog);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(label);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(12);
        gridLayout->setVerticalSpacing(6);
        destIcon = new QLabel(RenameDialog);
        destIcon->setObjectName(QString::fromUtf8("destIcon"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(destIcon->sizePolicy().hasHeightForWidth());
        destIcon->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(destIcon, 0, 0, 1, 1);

        label_3 = new QLabel(RenameDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 2);

        srcInfo = new QLabel(RenameDialog);
        srcInfo->setObjectName(QString::fromUtf8("srcInfo"));
        sizePolicy.setHeightForWidth(srcInfo->sizePolicy().hasHeightForWidth());
        srcInfo->setSizePolicy(sizePolicy);

        gridLayout->addWidget(srcInfo, 2, 1, 1, 1);

        destInfo = new QLabel(RenameDialog);
        destInfo->setObjectName(QString::fromUtf8("destInfo"));
        sizePolicy.setHeightForWidth(destInfo->sizePolicy().hasHeightForWidth());
        destInfo->setSizePolicy(sizePolicy);

        gridLayout->addWidget(destInfo, 0, 1, 1, 1);

        srcIcon = new QLabel(RenameDialog);
        srcIcon->setObjectName(QString::fromUtf8("srcIcon"));
        sizePolicy1.setHeightForWidth(srcIcon->sizePolicy().hasHeightForWidth());
        srcIcon->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(srcIcon, 2, 0, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(12);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_6 = new QLabel(RenameDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(label_6);

        fileName = new QLineEdit(RenameDialog);
        fileName->setObjectName(QString::fromUtf8("fileName"));

        horizontalLayout->addWidget(fileName);


        verticalLayout->addLayout(horizontalLayout);

        applyToAll = new QCheckBox(RenameDialog);
        applyToAll->setObjectName(QString::fromUtf8("applyToAll"));

        verticalLayout->addWidget(applyToAll);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(RenameDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ignore|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label_6->setBuddy(fileName);
#endif // QT_CONFIG(shortcut)

        retranslateUi(RenameDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), RenameDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), RenameDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(RenameDialog);
    } // setupUi

    void retranslateUi(QDialog *RenameDialog)
    {
        RenameDialog->setWindowTitle(QCoreApplication::translate("RenameDialog", "Confirm to replace files", nullptr));
        label->setText(QCoreApplication::translate("RenameDialog", "<html><head/><body><p><span style=\" font-weight:600;\">There is already a file with the same name in this location.</span></p><p>Do you want to replace the existing file?</p></body></html>", nullptr));
        destIcon->setText(QCoreApplication::translate("RenameDialog", "dest", nullptr));
        label_3->setText(QCoreApplication::translate("RenameDialog", "with the following file?", nullptr));
        srcInfo->setText(QCoreApplication::translate("RenameDialog", "src file info", nullptr));
        destInfo->setText(QCoreApplication::translate("RenameDialog", "dest file info", nullptr));
        srcIcon->setText(QCoreApplication::translate("RenameDialog", "src", nullptr));
        label_6->setText(QCoreApplication::translate("RenameDialog", "&File name:", nullptr));
        applyToAll->setText(QCoreApplication::translate("RenameDialog", "Apply this option to all existing files", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RenameDialog: public Ui_RenameDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENAME_2D_DIALOG_H
