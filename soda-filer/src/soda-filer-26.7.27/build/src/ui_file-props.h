/********************************************************************************
** Form generated from reading UI file 'file-props.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILE_2D_PROPS_H
#define UI_FILE_2D_PROPS_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "appchoosercombobox.h"

QT_BEGIN_NAMESPACE

class Ui_FilePropsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget_4;
    QGridLayout *gridLayout;
    QLabel *fileName;
    QPushButton *iconButton;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout_3;
    QLabel *label_4;
    QLabel *fileType;
    QLabel *label_7;
    QLabel *fileSize;
    QLabel *label_6;
    QLabel *onDiskSize;
    QLabel *label_2;
    QLabel *location;
    QLabel *targetLabel;
    QLabel *target;
    QLabel *label_3;
    QLabel *lastModified;
    QLabel *label_12;
    QLabel *lastAccessed;
    QGroupBox *groupBox_4;
    QFormLayout *formLayout;
    QLabel *label;
    QLabel *mimeType;
    QLabel *openWithLabel;
    Fm::AppChooserComboBox *openWith;
    QGroupBox *groupBox_5;
    QFormLayout *formLayout_4;
    QLabel *owner;
    QComboBox *ownerPerm;
    QLabel *ownerGroup;
    QComboBox *groupPerm;
    QLabel *otherLabel;
    QComboBox *otherPerm;
    QCheckBox *executable;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FilePropsDialog)
    {
        if (FilePropsDialog->objectName().isEmpty())
            FilePropsDialog->setObjectName(QString::fromUtf8("FilePropsDialog"));
        FilePropsDialog->resize(400, 522);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(FilePropsDialog->sizePolicy().hasHeightForWidth());
        FilePropsDialog->setSizePolicy(sizePolicy);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("document-properties");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        FilePropsDialog->setWindowIcon(icon);
        FilePropsDialog->setModal(false);
        verticalLayout = new QVBoxLayout(FilePropsDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(10, 10, 10, 10);
        widget_4 = new QWidget(FilePropsDialog);
        widget_4->setObjectName(QString::fromUtf8("widget_4"));
        sizePolicy.setHeightForWidth(widget_4->sizePolicy().hasHeightForWidth());
        widget_4->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(widget_4);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        fileName = new QLabel(widget_4);
        fileName->setObjectName(QString::fromUtf8("fileName"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        fileName->setFont(font);

        gridLayout->addWidget(fileName, 0, 1, 1, 1, Qt::AlignBottom);

        iconButton = new QPushButton(widget_4);
        iconButton->setObjectName(QString::fromUtf8("iconButton"));
        iconButton->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(iconButton->sizePolicy().hasHeightForWidth());
        iconButton->setSizePolicy(sizePolicy1);
        iconButton->setMinimumSize(QSize(36, 36));
        iconButton->setMaximumSize(QSize(36, 48));
        iconButton->setFocusPolicy(Qt::NoFocus);
        iconButton->setStyleSheet(QString::fromUtf8("border-radius: 0px;\n"
"background: transparent;\n"
"border: 0px;	"));
        QIcon icon1;
        iconThemeName = QString::fromUtf8("unknown");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        iconButton->setIcon(icon1);
        iconButton->setIconSize(QSize(36, 36));
        iconButton->setCheckable(false);
        iconButton->setFlat(false);

        gridLayout->addWidget(iconButton, 0, 0, 1, 1);


        verticalLayout->addWidget(widget_4);

        groupBox_3 = new QGroupBox(FilePropsDialog);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy);
        QFont font1;
        font1.setPointSize(8);
        groupBox_3->setFont(font1);
        formLayout_3 = new QFormLayout(groupBox_3);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        label_4 = new QLabel(groupBox_3);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_4);

        fileType = new QLabel(groupBox_3);
        fileType->setObjectName(QString::fromUtf8("fileType"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(fileType->sizePolicy().hasHeightForWidth());
        fileType->setSizePolicy(sizePolicy2);
        fileType->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(0, QFormLayout::FieldRole, fileType);

        label_7 = new QLabel(groupBox_3);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, label_7);

        fileSize = new QLabel(groupBox_3);
        fileSize->setObjectName(QString::fromUtf8("fileSize"));
        sizePolicy2.setHeightForWidth(fileSize->sizePolicy().hasHeightForWidth());
        fileSize->setSizePolicy(sizePolicy2);
        fileSize->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(1, QFormLayout::FieldRole, fileSize);

        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout_3->setWidget(2, QFormLayout::LabelRole, label_6);

        onDiskSize = new QLabel(groupBox_3);
        onDiskSize->setObjectName(QString::fromUtf8("onDiskSize"));
        sizePolicy2.setHeightForWidth(onDiskSize->sizePolicy().hasHeightForWidth());
        onDiskSize->setSizePolicy(sizePolicy2);
        onDiskSize->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(2, QFormLayout::FieldRole, onDiskSize);

        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout_3->setWidget(3, QFormLayout::LabelRole, label_2);

        location = new QLabel(groupBox_3);
        location->setObjectName(QString::fromUtf8("location"));
        sizePolicy2.setHeightForWidth(location->sizePolicy().hasHeightForWidth());
        location->setSizePolicy(sizePolicy2);
        location->setWordWrap(true);
        location->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(3, QFormLayout::FieldRole, location);

        targetLabel = new QLabel(groupBox_3);
        targetLabel->setObjectName(QString::fromUtf8("targetLabel"));

        formLayout_3->setWidget(4, QFormLayout::LabelRole, targetLabel);

        target = new QLabel(groupBox_3);
        target->setObjectName(QString::fromUtf8("target"));
        sizePolicy2.setHeightForWidth(target->sizePolicy().hasHeightForWidth());
        target->setSizePolicy(sizePolicy2);
        target->setWordWrap(true);
        target->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(4, QFormLayout::FieldRole, target);

        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout_3->setWidget(5, QFormLayout::LabelRole, label_3);

        lastModified = new QLabel(groupBox_3);
        lastModified->setObjectName(QString::fromUtf8("lastModified"));
        sizePolicy2.setHeightForWidth(lastModified->sizePolicy().hasHeightForWidth());
        lastModified->setSizePolicy(sizePolicy2);
        lastModified->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(5, QFormLayout::FieldRole, lastModified);

        label_12 = new QLabel(groupBox_3);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        formLayout_3->setWidget(6, QFormLayout::LabelRole, label_12);

        lastAccessed = new QLabel(groupBox_3);
        lastAccessed->setObjectName(QString::fromUtf8("lastAccessed"));
        lastAccessed->setEnabled(true);
        lastAccessed->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_3->setWidget(6, QFormLayout::FieldRole, lastAccessed);


        verticalLayout->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(FilePropsDialog);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        sizePolicy.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy);
        groupBox_4->setFont(font1);
        formLayout = new QFormLayout(groupBox_4);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(groupBox_4);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        mimeType = new QLabel(groupBox_4);
        mimeType->setObjectName(QString::fromUtf8("mimeType"));
        sizePolicy2.setHeightForWidth(mimeType->sizePolicy().hasHeightForWidth());
        mimeType->setSizePolicy(sizePolicy2);
        mimeType->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout->setWidget(0, QFormLayout::FieldRole, mimeType);

        openWithLabel = new QLabel(groupBox_4);
        openWithLabel->setObjectName(QString::fromUtf8("openWithLabel"));
        openWithLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        formLayout->setWidget(1, QFormLayout::LabelRole, openWithLabel);

        openWith = new Fm::AppChooserComboBox(groupBox_4);
        openWith->setObjectName(QString::fromUtf8("openWith"));
        QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(openWith->sizePolicy().hasHeightForWidth());
        openWith->setSizePolicy(sizePolicy3);

        formLayout->setWidget(1, QFormLayout::FieldRole, openWith);


        verticalLayout->addWidget(groupBox_4);

        groupBox_5 = new QGroupBox(FilePropsDialog);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        sizePolicy.setHeightForWidth(groupBox_5->sizePolicy().hasHeightForWidth());
        groupBox_5->setSizePolicy(sizePolicy);
        groupBox_5->setFont(font1);
        formLayout_4 = new QFormLayout(groupBox_5);
        formLayout_4->setObjectName(QString::fromUtf8("formLayout_4"));
        owner = new QLabel(groupBox_5);
        owner->setObjectName(QString::fromUtf8("owner"));
        owner->setTextInteractionFlags(Qt::TextSelectableByKeyboard);

        formLayout_4->setWidget(0, QFormLayout::LabelRole, owner);

        ownerPerm = new QComboBox(groupBox_5);
        ownerPerm->setObjectName(QString::fromUtf8("ownerPerm"));
        sizePolicy3.setHeightForWidth(ownerPerm->sizePolicy().hasHeightForWidth());
        ownerPerm->setSizePolicy(sizePolicy3);

        formLayout_4->setWidget(0, QFormLayout::FieldRole, ownerPerm);

        ownerGroup = new QLabel(groupBox_5);
        ownerGroup->setObjectName(QString::fromUtf8("ownerGroup"));
        ownerGroup->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayout_4->setWidget(1, QFormLayout::LabelRole, ownerGroup);

        groupPerm = new QComboBox(groupBox_5);
        groupPerm->setObjectName(QString::fromUtf8("groupPerm"));
        sizePolicy3.setHeightForWidth(groupPerm->sizePolicy().hasHeightForWidth());
        groupPerm->setSizePolicy(sizePolicy3);

        formLayout_4->setWidget(1, QFormLayout::FieldRole, groupPerm);

        otherLabel = new QLabel(groupBox_5);
        otherLabel->setObjectName(QString::fromUtf8("otherLabel"));
        otherLabel->setEnabled(false);

        formLayout_4->setWidget(2, QFormLayout::LabelRole, otherLabel);

        otherPerm = new QComboBox(groupBox_5);
        otherPerm->setObjectName(QString::fromUtf8("otherPerm"));
        sizePolicy3.setHeightForWidth(otherPerm->sizePolicy().hasHeightForWidth());
        otherPerm->setSizePolicy(sizePolicy3);

        formLayout_4->setWidget(2, QFormLayout::FieldRole, otherPerm);

        executable = new QCheckBox(groupBox_5);
        executable->setObjectName(QString::fromUtf8("executable"));
        executable->setTristate(true);

        formLayout_4->setWidget(3, QFormLayout::LabelRole, executable);


        verticalLayout->addWidget(groupBox_5);

        buttonBox = new QDialogButtonBox(FilePropsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
        buttonBox->setSizePolicy(sizePolicy4);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(FilePropsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FilePropsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FilePropsDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FilePropsDialog);
    } // setupUi

    void retranslateUi(QDialog *FilePropsDialog)
    {
        FilePropsDialog->setWindowTitle(QCoreApplication::translate("FilePropsDialog", "Info", nullptr));
        fileName->setText(QCoreApplication::translate("FilePropsDialog", "TextLabel", nullptr));
        iconButton->setText(QString());
        groupBox_3->setTitle(QCoreApplication::translate("FilePropsDialog", "General", nullptr));
        label_4->setText(QCoreApplication::translate("FilePropsDialog", "File type:", nullptr));
        fileType->setText(QString());
        label_7->setText(QCoreApplication::translate("FilePropsDialog", "File size:", nullptr));
        fileSize->setText(QString());
        label_6->setText(QCoreApplication::translate("FilePropsDialog", "On-disk size:", nullptr));
        onDiskSize->setText(QString());
        label_2->setText(QCoreApplication::translate("FilePropsDialog", "Location:", nullptr));
        location->setText(QString());
        targetLabel->setText(QCoreApplication::translate("FilePropsDialog", "Link target:", nullptr));
        target->setText(QString());
        label_3->setText(QCoreApplication::translate("FilePropsDialog", "Last modified:", nullptr));
        lastModified->setText(QString());
        label_12->setText(QCoreApplication::translate("FilePropsDialog", "Last accessed:", nullptr));
        lastAccessed->setText(QString());
        groupBox_4->setTitle(QCoreApplication::translate("FilePropsDialog", "Open with", nullptr));
        label->setText(QCoreApplication::translate("FilePropsDialog", "Mime type:", nullptr));
        mimeType->setText(QString());
        openWithLabel->setText(QCoreApplication::translate("FilePropsDialog", "Open With:", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("FilePropsDialog", "Access Control", nullptr));
        otherLabel->setText(QCoreApplication::translate("FilePropsDialog", "Everyone", nullptr));
        executable->setText(QCoreApplication::translate("FilePropsDialog", "Make the file executable", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FilePropsDialog: public Ui_FilePropsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILE_2D_PROPS_H
