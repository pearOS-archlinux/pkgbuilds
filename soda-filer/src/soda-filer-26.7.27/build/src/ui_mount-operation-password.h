/********************************************************************************
** Form generated from reading UI file 'mount-operation-password.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MOUNT_2D_OPERATION_2D_PASSWORD_H
#define UI_MOUNT_2D_OPERATION_2D_PASSWORD_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MountOperationPasswordDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *message;
    QRadioButton *Anonymous;
    QRadioButton *asUser;
    QGridLayout *gridLayout;
    QLineEdit *username;
    QLabel *label;
    QLineEdit *password;
    QLabel *label_2;
    QLabel *domainLabel;
    QLineEdit *domain;
    QRadioButton *forgetPassword;
    QRadioButton *sessionPassword;
    QRadioButton *storePassword;
    QDialogButtonBox *buttonBox;
    QButtonGroup *usernameGroup;
    QButtonGroup *passwordGroup;

    void setupUi(QDialog *MountOperationPasswordDialog)
    {
        if (MountOperationPasswordDialog->objectName().isEmpty())
            MountOperationPasswordDialog->setObjectName(QString::fromUtf8("MountOperationPasswordDialog"));
        MountOperationPasswordDialog->resize(244, 302);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MountOperationPasswordDialog->sizePolicy().hasHeightForWidth());
        MountOperationPasswordDialog->setSizePolicy(sizePolicy);
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("dialog-password")));
        MountOperationPasswordDialog->setWindowIcon(icon);
        MountOperationPasswordDialog->setSizeGripEnabled(false);
        MountOperationPasswordDialog->setModal(false);
        verticalLayout = new QVBoxLayout(MountOperationPasswordDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        message = new QLabel(MountOperationPasswordDialog);
        message->setObjectName(QString::fromUtf8("message"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(message->sizePolicy().hasHeightForWidth());
        message->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(message);

        Anonymous = new QRadioButton(MountOperationPasswordDialog);
        usernameGroup = new QButtonGroup(MountOperationPasswordDialog);
        usernameGroup->setObjectName(QString::fromUtf8("usernameGroup"));
        usernameGroup->addButton(Anonymous);
        Anonymous->setObjectName(QString::fromUtf8("Anonymous"));

        verticalLayout->addWidget(Anonymous);

        asUser = new QRadioButton(MountOperationPasswordDialog);
        usernameGroup->addButton(asUser);
        asUser->setObjectName(QString::fromUtf8("asUser"));

        verticalLayout->addWidget(asUser);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        username = new QLineEdit(MountOperationPasswordDialog);
        username->setObjectName(QString::fromUtf8("username"));

        gridLayout->addWidget(username, 0, 1, 1, 1);

        label = new QLabel(MountOperationPasswordDialog);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        password = new QLineEdit(MountOperationPasswordDialog);
        password->setObjectName(QString::fromUtf8("password"));
        password->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(password, 3, 1, 1, 1);

        label_2 = new QLabel(MountOperationPasswordDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(label_2, 3, 0, 1, 1);

        domainLabel = new QLabel(MountOperationPasswordDialog);
        domainLabel->setObjectName(QString::fromUtf8("domainLabel"));

        gridLayout->addWidget(domainLabel, 2, 0, 1, 1);

        domain = new QLineEdit(MountOperationPasswordDialog);
        domain->setObjectName(QString::fromUtf8("domain"));

        gridLayout->addWidget(domain, 2, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        forgetPassword = new QRadioButton(MountOperationPasswordDialog);
        passwordGroup = new QButtonGroup(MountOperationPasswordDialog);
        passwordGroup->setObjectName(QString::fromUtf8("passwordGroup"));
        passwordGroup->addButton(forgetPassword);
        forgetPassword->setObjectName(QString::fromUtf8("forgetPassword"));

        verticalLayout->addWidget(forgetPassword);

        sessionPassword = new QRadioButton(MountOperationPasswordDialog);
        passwordGroup->addButton(sessionPassword);
        sessionPassword->setObjectName(QString::fromUtf8("sessionPassword"));

        verticalLayout->addWidget(sessionPassword);

        storePassword = new QRadioButton(MountOperationPasswordDialog);
        passwordGroup->addButton(storePassword);
        storePassword->setObjectName(QString::fromUtf8("storePassword"));

        verticalLayout->addWidget(storePassword);

        buttonBox = new QDialogButtonBox(MountOperationPasswordDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label->setBuddy(username);
        label_2->setBuddy(password);
        domainLabel->setBuddy(domain);
#endif // QT_CONFIG(shortcut)

        retranslateUi(MountOperationPasswordDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), MountOperationPasswordDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), MountOperationPasswordDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(MountOperationPasswordDialog);
    } // setupUi

    void retranslateUi(QDialog *MountOperationPasswordDialog)
    {
        MountOperationPasswordDialog->setWindowTitle(QCoreApplication::translate("MountOperationPasswordDialog", "Mount", nullptr));
        message->setText(QString());
        Anonymous->setText(QCoreApplication::translate("MountOperationPasswordDialog", "Connect &anonymously", nullptr));
        asUser->setText(QCoreApplication::translate("MountOperationPasswordDialog", "Connect as u&ser:", nullptr));
        label->setText(QCoreApplication::translate("MountOperationPasswordDialog", "&Username:", nullptr));
        label_2->setText(QCoreApplication::translate("MountOperationPasswordDialog", "&Password:", nullptr));
        domainLabel->setText(QCoreApplication::translate("MountOperationPasswordDialog", "&Domain:", nullptr));
        forgetPassword->setText(QCoreApplication::translate("MountOperationPasswordDialog", "Forget password &immediately", nullptr));
        sessionPassword->setText(QCoreApplication::translate("MountOperationPasswordDialog", "Remember password until you &logout", nullptr));
        storePassword->setText(QCoreApplication::translate("MountOperationPasswordDialog", "Remember &forever", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MountOperationPasswordDialog: public Ui_MountOperationPasswordDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MOUNT_2D_OPERATION_2D_PASSWORD_H
