/********************************************************************************
** Form generated from reading UI file 'app-chooser-dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_APP_2D_CHOOSER_2D_DIALOG_H
#define UI_APP_2D_CHOOSER_2D_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "appmenuview.h"

QT_BEGIN_NAMESPACE

class Ui_AppChooserDialog
{
public:
    QFormLayout *formLayout;
    QLabel *fileTypeHeader;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout;
    Fm::AppMenuView *appMenuView;
    QWidget *tab_2;
    QFormLayout *formLayout_2;
    QLabel *label_3;
    QLineEdit *cmdLine;
    QLabel *label_4;
    QLineEdit *appName;
    QLabel *label_5;
    QCheckBox *keepTermOpen;
    QCheckBox *useTerminal;
    QCheckBox *setDefault;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AppChooserDialog)
    {
        if (AppChooserDialog->objectName().isEmpty())
            AppChooserDialog->setObjectName(QString::fromUtf8("AppChooserDialog"));
        AppChooserDialog->resize(432, 387);
        formLayout = new QFormLayout(AppChooserDialog);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        fileTypeHeader = new QLabel(AppChooserDialog);
        fileTypeHeader->setObjectName(QString::fromUtf8("fileTypeHeader"));

        formLayout->setWidget(0, QFormLayout::FieldRole, fileTypeHeader);

        tabWidget = new QTabWidget(AppChooserDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout = new QVBoxLayout(tab);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        appMenuView = new Fm::AppMenuView(tab);
        appMenuView->setObjectName(QString::fromUtf8("appMenuView"));

        verticalLayout->addWidget(appMenuView);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        formLayout_2 = new QFormLayout(tab_2);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_3 = new QLabel(tab_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout_2->setWidget(0, QFormLayout::SpanningRole, label_3);

        cmdLine = new QLineEdit(tab_2);
        cmdLine->setObjectName(QString::fromUtf8("cmdLine"));

        formLayout_2->setWidget(1, QFormLayout::SpanningRole, cmdLine);

        label_4 = new QLabel(tab_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_4);

        appName = new QLineEdit(tab_2);
        appName->setObjectName(QString::fromUtf8("appName"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, appName);

        label_5 = new QLabel(tab_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setTextFormat(Qt::RichText);

        formLayout_2->setWidget(2, QFormLayout::SpanningRole, label_5);

        keepTermOpen = new QCheckBox(tab_2);
        keepTermOpen->setObjectName(QString::fromUtf8("keepTermOpen"));
        keepTermOpen->setEnabled(false);

        formLayout_2->setWidget(5, QFormLayout::SpanningRole, keepTermOpen);

        useTerminal = new QCheckBox(tab_2);
        useTerminal->setObjectName(QString::fromUtf8("useTerminal"));

        formLayout_2->setWidget(4, QFormLayout::SpanningRole, useTerminal);

        tabWidget->addTab(tab_2, QString());

        formLayout->setWidget(1, QFormLayout::SpanningRole, tabWidget);

        setDefault = new QCheckBox(AppChooserDialog);
        setDefault->setObjectName(QString::fromUtf8("setDefault"));

        formLayout->setWidget(2, QFormLayout::SpanningRole, setDefault);

        buttonBox = new QDialogButtonBox(AppChooserDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        formLayout->setWidget(3, QFormLayout::SpanningRole, buttonBox);


        retranslateUi(AppChooserDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AppChooserDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AppChooserDialog, SLOT(reject()));
        QObject::connect(useTerminal, SIGNAL(toggled(bool)), keepTermOpen, SLOT(setEnabled(bool)));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AppChooserDialog);
    } // setupUi

    void retranslateUi(QDialog *AppChooserDialog)
    {
        AppChooserDialog->setWindowTitle(QCoreApplication::translate("AppChooserDialog", "Choose an Application", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("AppChooserDialog", "Installed Applications", nullptr));
        label_3->setText(QCoreApplication::translate("AppChooserDialog", "Command line to execute:", nullptr));
        label_4->setText(QCoreApplication::translate("AppChooserDialog", "Application name:", nullptr));
        label_5->setText(QCoreApplication::translate("AppChooserDialog", "<b>These special codes can be used in the command line:</b>\n"
"<ul>\n"
"<li><b>%f</b>: Represents a single file name</li>\n"
"<li><b>%F</b>: Represents multiple file names</li>\n"
"<li><b>%u</b>: Represents a single URI of the file</li>\n"
"<li><b>%U</b>: Represents multiple URIs</li>\n"
"</ul>", nullptr));
        keepTermOpen->setText(QCoreApplication::translate("AppChooserDialog", "Keep terminal window open after command execution", nullptr));
        useTerminal->setText(QCoreApplication::translate("AppChooserDialog", "Execute in terminal emulator", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("AppChooserDialog", "Custom Command", nullptr));
        setDefault->setText(QCoreApplication::translate("AppChooserDialog", "Set selected application as default action of this file type", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AppChooserDialog: public Ui_AppChooserDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APP_2D_CHOOSER_2D_DIALOG_H
