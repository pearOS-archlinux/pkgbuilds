/********************************************************************************
** Form generated from reading UI file 'exec-file.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXEC_2D_FILE_H
#define UI_EXEC_2D_FILE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ExecFileDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *icon;
    QLabel *msg;
    QHBoxLayout *horizontalLayout;
    QPushButton *open;
    QPushButton *exec;
    QPushButton *execTerm;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancel;

    void setupUi(QDialog *ExecFileDialog)
    {
        if (ExecFileDialog->objectName().isEmpty())
            ExecFileDialog->setObjectName(QString::fromUtf8("ExecFileDialog"));
        ExecFileDialog->resize(487, 58);
        verticalLayout = new QVBoxLayout(ExecFileDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        icon = new QLabel(ExecFileDialog);
        icon->setObjectName(QString::fromUtf8("icon"));

        horizontalLayout_2->addWidget(icon);

        msg = new QLabel(ExecFileDialog);
        msg->setObjectName(QString::fromUtf8("msg"));
        msg->setWordWrap(true);

        horizontalLayout_2->addWidget(msg);

        horizontalLayout_2->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        open = new QPushButton(ExecFileDialog);
        open->setObjectName(QString::fromUtf8("open"));
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("document-open")));
        open->setIcon(icon1);

        horizontalLayout->addWidget(open);

        exec = new QPushButton(ExecFileDialog);
        exec->setObjectName(QString::fromUtf8("exec"));
        QIcon icon2(QIcon::fromTheme(QString::fromUtf8("system-run")));
        exec->setIcon(icon2);

        horizontalLayout->addWidget(exec);

        execTerm = new QPushButton(ExecFileDialog);
        execTerm->setObjectName(QString::fromUtf8("execTerm"));
        QIcon icon3(QIcon::fromTheme(QString::fromUtf8("utilities-terminal")));
        execTerm->setIcon(icon3);

        horizontalLayout->addWidget(execTerm);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancel = new QPushButton(ExecFileDialog);
        cancel->setObjectName(QString::fromUtf8("cancel"));
        QIcon icon4(QIcon::fromTheme(QString::fromUtf8("dialog-cancel")));
        cancel->setIcon(icon4);

        horizontalLayout->addWidget(cancel);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ExecFileDialog);
        QObject::connect(cancel, SIGNAL(clicked()), ExecFileDialog, SLOT(reject()));
        QObject::connect(exec, SIGNAL(clicked()), ExecFileDialog, SLOT(accept()));
        QObject::connect(execTerm, SIGNAL(clicked()), ExecFileDialog, SLOT(accept()));
        QObject::connect(open, SIGNAL(clicked()), ExecFileDialog, SLOT(accept()));

        open->setDefault(true);


        QMetaObject::connectSlotsByName(ExecFileDialog);
    } // setupUi

    void retranslateUi(QDialog *ExecFileDialog)
    {
        ExecFileDialog->setWindowTitle(QCoreApplication::translate("ExecFileDialog", "Execute file", nullptr));
        msg->setText(QString());
        open->setText(QCoreApplication::translate("ExecFileDialog", "&Open", nullptr));
        exec->setText(QCoreApplication::translate("ExecFileDialog", "E&xecute", nullptr));
        execTerm->setText(QCoreApplication::translate("ExecFileDialog", "Execute in &Terminal", nullptr));
        cancel->setText(QCoreApplication::translate("ExecFileDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ExecFileDialog: public Ui_ExecFileDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXEC_2D_FILE_H
