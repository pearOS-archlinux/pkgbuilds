/********************************************************************************
** Form generated from reading UI file 'file-operation-dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILE_2D_OPERATION_2D_DIALOG_H
#define UI_FILE_2D_OPERATION_2D_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>

QT_BEGIN_NAMESPACE

class Ui_FileOperationDialog
{
public:
    QFormLayout *formLayout_2;
    QFormLayout *formLayout;
    QLabel *destLabel;
    QLabel *dest;
    QLabel *label_3;
    QLabel *curFile;
    QLabel *label_4;
    QProgressBar *progressBar;
    QLabel *label_5;
    QLabel *timeRemaining;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FileOperationDialog)
    {
        if (FileOperationDialog->objectName().isEmpty())
            FileOperationDialog->setObjectName(QString::fromUtf8("FileOperationDialog"));
        FileOperationDialog->resize(450, 131);
        formLayout_2 = new QFormLayout(FileOperationDialog);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        destLabel = new QLabel(FileOperationDialog);
        destLabel->setObjectName(QString::fromUtf8("destLabel"));

        formLayout->setWidget(0, QFormLayout::LabelRole, destLabel);

        dest = new QLabel(FileOperationDialog);
        dest->setObjectName(QString::fromUtf8("dest"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(dest->sizePolicy().hasHeightForWidth());
        dest->setSizePolicy(sizePolicy);
        dest->setWordWrap(true);

        formLayout->setWidget(0, QFormLayout::FieldRole, dest);

        label_3 = new QLabel(FileOperationDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

        curFile = new QLabel(FileOperationDialog);
        curFile->setObjectName(QString::fromUtf8("curFile"));
        sizePolicy.setHeightForWidth(curFile->sizePolicy().hasHeightForWidth());
        curFile->setSizePolicy(sizePolicy);

        formLayout->setWidget(1, QFormLayout::FieldRole, curFile);

        label_4 = new QLabel(FileOperationDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_4);

        progressBar = new QProgressBar(FileOperationDialog);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setAlignment(Qt::AlignCenter);

        formLayout->setWidget(2, QFormLayout::FieldRole, progressBar);

        label_5 = new QLabel(FileOperationDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);

        formLayout->setWidget(3, QFormLayout::LabelRole, label_5);

        timeRemaining = new QLabel(FileOperationDialog);
        timeRemaining->setObjectName(QString::fromUtf8("timeRemaining"));
        sizePolicy.setHeightForWidth(timeRemaining->sizePolicy().hasHeightForWidth());
        timeRemaining->setSizePolicy(sizePolicy);

        formLayout->setWidget(3, QFormLayout::FieldRole, timeRemaining);


        formLayout_2->setLayout(0, QFormLayout::FieldRole, formLayout);

        buttonBox = new QDialogButtonBox(FileOperationDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

        formLayout_2->setWidget(1, QFormLayout::SpanningRole, buttonBox);


        retranslateUi(FileOperationDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FileOperationDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FileOperationDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FileOperationDialog);
    } // setupUi

    void retranslateUi(QDialog *FileOperationDialog)
    {
        FileOperationDialog->setWindowTitle(QString());
        destLabel->setText(QCoreApplication::translate("FileOperationDialog", "Destination:", nullptr));
        dest->setText(QString());
        label_3->setText(QCoreApplication::translate("FileOperationDialog", "Processing:", nullptr));
        curFile->setText(QCoreApplication::translate("FileOperationDialog", "Preparing...", nullptr));
        label_4->setText(QCoreApplication::translate("FileOperationDialog", "Progress", nullptr));
        label_5->setText(QCoreApplication::translate("FileOperationDialog", "Time remaining:", nullptr));
        timeRemaining->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class FileOperationDialog: public Ui_FileOperationDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILE_2D_OPERATION_2D_DIALOG_H
