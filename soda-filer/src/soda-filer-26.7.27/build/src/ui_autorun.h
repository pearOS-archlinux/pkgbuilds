/********************************************************************************
** Form generated from reading UI file 'autorun.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTORUN_H
#define UI_AUTORUN_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>

QT_BEGIN_NAMESPACE

class Ui_AutoRunDialog
{
public:
    QFormLayout *formLayout_2;
    QLabel *icon;
    QFormLayout *formLayout;
    QLabel *label;
    QLabel *label_2;
    QLabel *mediumType;
    QLabel *label_3;
    QListWidget *listWidget;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AutoRunDialog)
    {
        if (AutoRunDialog->objectName().isEmpty())
            AutoRunDialog->setObjectName(QString::fromUtf8("AutoRunDialog"));
        AutoRunDialog->resize(334, 312);
        QIcon icon1;
        QString iconThemeName = QString::fromUtf8("drive-removable-media");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8("../../../.designer/backup"), QSize(), QIcon::Normal, QIcon::Off);
        }
        AutoRunDialog->setWindowIcon(icon1);
        formLayout_2 = new QFormLayout(AutoRunDialog);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        icon = new QLabel(AutoRunDialog);
        icon->setObjectName(QString::fromUtf8("icon"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, icon);

        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(AutoRunDialog);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::SpanningRole, label);

        label_2 = new QLabel(AutoRunDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        mediumType = new QLabel(AutoRunDialog);
        mediumType->setObjectName(QString::fromUtf8("mediumType"));

        formLayout->setWidget(1, QFormLayout::FieldRole, mediumType);


        formLayout_2->setLayout(0, QFormLayout::FieldRole, formLayout);

        label_3 = new QLabel(AutoRunDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout_2->setWidget(1, QFormLayout::SpanningRole, label_3);

        listWidget = new QListWidget(AutoRunDialog);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
        listWidget->setSizePolicy(sizePolicy);

        formLayout_2->setWidget(2, QFormLayout::SpanningRole, listWidget);

        buttonBox = new QDialogButtonBox(AutoRunDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        formLayout_2->setWidget(3, QFormLayout::SpanningRole, buttonBox);


        retranslateUi(AutoRunDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AutoRunDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AutoRunDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AutoRunDialog);
    } // setupUi

    void retranslateUi(QDialog *AutoRunDialog)
    {
        AutoRunDialog->setWindowTitle(QCoreApplication::translate("AutoRunDialog", "Removable medium is inserted", nullptr));
        icon->setText(QString());
        label->setText(QCoreApplication::translate("AutoRunDialog", "<b>Removable medium is inserted</b>", nullptr));
        label_2->setText(QCoreApplication::translate("AutoRunDialog", "Type of medium:", nullptr));
        mediumType->setText(QCoreApplication::translate("AutoRunDialog", "Detecting...", nullptr));
        label_3->setText(QCoreApplication::translate("AutoRunDialog", "Please select the action you want to perform:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AutoRunDialog: public Ui_AutoRunDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTORUN_H
