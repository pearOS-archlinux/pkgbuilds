/********************************************************************************
** Form generated from reading UI file 'desktop-preferences.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DESKTOP_2D_PREFERENCES_H
#define UI_DESKTOP_2D_PREFERENCES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include "colorbutton.h"

QT_BEGIN_NAMESPACE

class Ui_DesktopPreferencesDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *imageFile;
    QPushButton *browse;
    QComboBox *wallpaperMode;
    QLabel *label;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_3;
    Fm::ColorButton *shadowColor;
    Fm::ColorButton *textColor;
    QLabel *label_4;
    QLabel *label_5;
    Fm::ColorButton *backgroundColor;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DesktopPreferencesDialog)
    {
        if (DesktopPreferencesDialog->objectName().isEmpty())
            DesktopPreferencesDialog->setObjectName(QString::fromUtf8("DesktopPreferencesDialog"));
        DesktopPreferencesDialog->resize(328, 337);
        verticalLayout = new QVBoxLayout(DesktopPreferencesDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox_2 = new QGroupBox(DesktopPreferencesDialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout = new QGridLayout(groupBox_2);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        imageFile = new QLineEdit(groupBox_2);
        imageFile->setObjectName(QString::fromUtf8("imageFile"));
        imageFile->setEnabled(false);

        horizontalLayout->addWidget(imageFile);

        browse = new QPushButton(groupBox_2);
        browse->setObjectName(QString::fromUtf8("browse"));

        horizontalLayout->addWidget(browse);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 2);

        wallpaperMode = new QComboBox(groupBox_2);
        wallpaperMode->setObjectName(QString::fromUtf8("wallpaperMode"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(wallpaperMode->sizePolicy().hasHeightForWidth());
        wallpaperMode->setSizePolicy(sizePolicy);

        gridLayout->addWidget(wallpaperMode, 2, 1, 1, 1);

        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label, 2, 0, 1, 1);


        verticalLayout->addWidget(groupBox_2);

        groupBox = new QGroupBox(DesktopPreferencesDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy2);

        gridLayout_2->addWidget(label_3, 1, 0, 1, 1);

        shadowColor = new Fm::ColorButton(groupBox);
        shadowColor->setObjectName(QString::fromUtf8("shadowColor"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(shadowColor->sizePolicy().hasHeightForWidth());
        shadowColor->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(shadowColor, 3, 1, 1, 1);

        textColor = new Fm::ColorButton(groupBox);
        textColor->setObjectName(QString::fromUtf8("textColor"));
        sizePolicy3.setHeightForWidth(textColor->sizePolicy().hasHeightForWidth());
        textColor->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(textColor, 1, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy2.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy2);

        gridLayout_2->addWidget(label_4, 3, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 0, 0, 1, 1);

        backgroundColor = new Fm::ColorButton(groupBox);
        backgroundColor->setObjectName(QString::fromUtf8("backgroundColor"));
        sizePolicy3.setHeightForWidth(backgroundColor->sizePolicy().hasHeightForWidth());
        backgroundColor->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(backgroundColor, 0, 1, 1, 1);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(DesktopPreferencesDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DesktopPreferencesDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), DesktopPreferencesDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DesktopPreferencesDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(DesktopPreferencesDialog);
    } // setupUi

    void retranslateUi(QDialog *DesktopPreferencesDialog)
    {
        DesktopPreferencesDialog->setWindowTitle(QCoreApplication::translate("DesktopPreferencesDialog", "Desktop Preferences", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("DesktopPreferencesDialog", "Desktop Picture", nullptr));
#if QT_CONFIG(tooltip)
        imageFile->setToolTip(QCoreApplication::translate("DesktopPreferencesDialog", "Image file", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        imageFile->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        imageFile->setPlaceholderText(QCoreApplication::translate("DesktopPreferencesDialog", "Picture file path", nullptr));
        browse->setText(QCoreApplication::translate("DesktopPreferencesDialog", "&Browse", nullptr));
        label->setText(QCoreApplication::translate("DesktopPreferencesDialog", "Mode:", nullptr));
        groupBox->setTitle(QCoreApplication::translate("DesktopPreferencesDialog", "Colors", nullptr));
        label_3->setText(QCoreApplication::translate("DesktopPreferencesDialog", "Text color:", nullptr));
        shadowColor->setText(QString());
        textColor->setText(QString());
        label_4->setText(QCoreApplication::translate("DesktopPreferencesDialog", "Shadow color:", nullptr));
        label_5->setText(QCoreApplication::translate("DesktopPreferencesDialog", "Background color:", nullptr));
        backgroundColor->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class DesktopPreferencesDialog: public Ui_DesktopPreferencesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DESKTOP_2D_PREFERENCES_H
