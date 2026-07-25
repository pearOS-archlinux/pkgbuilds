/********************************************************************************
** Form generated from reading UI file 'desktop-folder.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DESKTOP_2D_FOLDER_H
#define UI_DESKTOP_2D_FOLDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DesktopFolder
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_7;
    QLineEdit *desktopFolder;
    QPushButton *browseDesktopFolder;

    void setupUi(QWidget *DesktopFolder)
    {
        if (DesktopFolder->objectName().isEmpty())
            DesktopFolder->setObjectName(QString::fromUtf8("DesktopFolder"));
        DesktopFolder->resize(352, 81);
        verticalLayout = new QVBoxLayout(DesktopFolder);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox_3 = new QGroupBox(DesktopFolder);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setEnabled(true);
        horizontalLayout_4 = new QHBoxLayout(groupBox_3);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_7 = new QLabel(groupBox_3);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_4->addWidget(label_7);

        desktopFolder = new QLineEdit(groupBox_3);
        desktopFolder->setObjectName(QString::fromUtf8("desktopFolder"));

        horizontalLayout_4->addWidget(desktopFolder);

        browseDesktopFolder = new QPushButton(groupBox_3);
        browseDesktopFolder->setObjectName(QString::fromUtf8("browseDesktopFolder"));

        horizontalLayout_4->addWidget(browseDesktopFolder);


        verticalLayout->addWidget(groupBox_3);


        retranslateUi(DesktopFolder);

        QMetaObject::connectSlotsByName(DesktopFolder);
    } // setupUi

    void retranslateUi(QWidget *DesktopFolder)
    {
        DesktopFolder->setWindowTitle(QCoreApplication::translate("DesktopFolder", "Form", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("DesktopFolder", "Desktop", nullptr));
        label_7->setText(QCoreApplication::translate("DesktopFolder", "Desktop folder:", nullptr));
#if QT_CONFIG(tooltip)
        desktopFolder->setToolTip(QCoreApplication::translate("DesktopFolder", "Image file", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        desktopFolder->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        desktopFolder->setPlaceholderText(QCoreApplication::translate("DesktopFolder", "Folder path", nullptr));
        browseDesktopFolder->setText(QCoreApplication::translate("DesktopFolder", "&Browse", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DesktopFolder: public Ui_DesktopFolder {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DESKTOP_2D_FOLDER_H
