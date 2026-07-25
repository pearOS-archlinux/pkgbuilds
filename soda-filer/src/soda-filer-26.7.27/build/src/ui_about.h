/********************************************************************************
** Form generated from reading UI file 'about.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label_4;
    QLabel *label;
    QLabel *version;
    QLabel *label_2;
    QLabel *label_3;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_4;
    QPlainTextEdit *plainTextEdit_2;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_3;
    QPlainTextEdit *plainTextEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(543, 520);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("help-about");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        AboutDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(AboutDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_4 = new QLabel(AboutDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setPixmap(QPixmap(QString::fromUtf8("Resources/Filer.png")));
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_4);

        label = new QLabel(AboutDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setTextFormat(Qt::RichText);
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        version = new QLabel(AboutDialog);
        version->setObjectName(QString::fromUtf8("version"));
        version->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(version);

        label_2 = new QLabel(AboutDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_2);

        label_3 = new QLabel(AboutDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setTextFormat(Qt::RichText);
        label_3->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_3);

        tabWidget = new QTabWidget(AboutDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setTabsClosable(false);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_4 = new QVBoxLayout(tab);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        plainTextEdit_2 = new QPlainTextEdit(tab);
        plainTextEdit_2->setObjectName(QString::fromUtf8("plainTextEdit_2"));
        plainTextEdit_2->setReadOnly(true);
        plainTextEdit_2->setBackgroundVisible(false);

        verticalLayout_4->addWidget(plainTextEdit_2);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_3 = new QVBoxLayout(tab_2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        plainTextEdit = new QPlainTextEdit(tab_2);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setReadOnly(true);

        verticalLayout_3->addWidget(plainTextEdit);

        tabWidget->addTab(tab_2, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(AboutDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(AboutDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AboutDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AboutDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QCoreApplication::translate("AboutDialog", "About", nullptr));
        label_4->setText(QString());
        label->setText(QCoreApplication::translate("AboutDialog", "<html><head/><body><p><span style=\" font-size:16pt; font-weight:600;\">Filer</span></p></body></html>", nullptr));
        version->setText(QString());
        label_2->setText(QCoreApplication::translate("AboutDialog", "The Desktop Experience", nullptr));
        label_3->setText(QCoreApplication::translate("AboutDialog", "<html><head/><body><p><a href=\"https://github.com/helloSystem/Filer/\"><span style=\" text-decoration: underline; color:#0000ff;\">https://github.com/helloSystem/Filer/</span></a></p></body></html>", nullptr));
        plainTextEdit_2->setPlainText(QCoreApplication::translate("AboutDialog", "Programming:\n"
"* Simon Peter (probono)\n"
"* Chris Moore (moochris)\n"
"* Hong Jen Yee (PCMan) <pcman.tw@gmail.com>\n"
"\n"
"Application icon:\n"
"* Raphael Lopes (https://raphaellopes.me/)", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("AboutDialog", "Authors", nullptr));
        plainTextEdit->setPlainText(QCoreApplication::translate("AboutDialog", "Filer\n"
"\n"
"Copyright (C) 2020-21 Simon Peter\n"
"Copyright (C) 2021 Chris Moore\n"
"\n"
"Originally based on PCMan File Manager\n"
"Portions Copyright (C) 2009-14 \346\264\252\344\273\273\350\253\255 (Hong Jen Yee)\n"
"\n"
"This program is free software; you can redistribute it and/or\n"
"modify it under the terms of the GNU General Public License\n"
"as published by the Free Software Foundation; either version 2\n"
"of the License, or (at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program; if not, write to the Free Software\n"
"Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
"\n"
"Application icon\n"
"\n"
"https://dribbble.com/shots/25412"
                        "11--Pirate-Finder-icon#\n"
"\n"
"Copyright (C) 2016 Raphael Lopes <raphaellopes8@gmail.com>\n"
"\n"
"Used with permission of the creator https://raphaellopes.me/", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("AboutDialog", "License", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
