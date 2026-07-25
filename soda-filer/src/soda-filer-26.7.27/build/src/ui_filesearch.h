/********************************************************************************
** Form generated from reading UI file 'filesearch.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILESEARCH_H
#define UI_FILESEARCH_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SearchDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QLineEdit *namePatterns;
    QCheckBox *nameCaseInsensitive;
    QCheckBox *nameRegExp;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout;
    QListWidget *listView;
    QVBoxLayout *verticalLayout_4;
    QPushButton *addPath;
    QPushButton *removePath;
    QSpacerItem *verticalSpacer;
    QCheckBox *recursiveSearch;
    QCheckBox *searchHidden;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_7;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_6;
    QCheckBox *searchTextFiles;
    QCheckBox *searchImages;
    QCheckBox *searchAudio;
    QCheckBox *searchVideo;
    QCheckBox *searchDocuments;
    QCheckBox *searchFolders;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_9;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_8;
    QLineEdit *contentPattern;
    QCheckBox *contentCaseInsensitive;
    QCheckBox *contentRegExp;
    QSpacerItem *verticalSpacer_3;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_10;
    QGroupBox *groupBox_5;
    QFormLayout *formLayout;
    QCheckBox *largerThan;
    QHBoxLayout *horizontalLayout_2;
    QSpinBox *minSize;
    QComboBox *minSizeUnit;
    QCheckBox *smallerThan;
    QHBoxLayout *horizontalLayout_3;
    QSpinBox *maxSize;
    QComboBox *maxSizeUnit;
    QGroupBox *groupBox_6;
    QFormLayout *formLayout_2;
    QCheckBox *earlierThan;
    QCheckBox *laterThan;
    QDateEdit *maxTime;
    QDateEdit *minTime;
    QSpacerItem *verticalSpacer_4;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SearchDialog)
    {
        if (SearchDialog->objectName().isEmpty())
            SearchDialog->setObjectName(QString::fromUtf8("SearchDialog"));
        SearchDialog->resize(512, 420);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("system-search");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        SearchDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(SearchDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tabWidget = new QTabWidget(SearchDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        namePatterns = new QLineEdit(groupBox);
        namePatterns->setObjectName(QString::fromUtf8("namePatterns"));

        verticalLayout_3->addWidget(namePatterns);

        nameCaseInsensitive = new QCheckBox(groupBox);
        nameCaseInsensitive->setObjectName(QString::fromUtf8("nameCaseInsensitive"));

        verticalLayout_3->addWidget(nameCaseInsensitive);

        nameRegExp = new QCheckBox(groupBox);
        nameRegExp->setObjectName(QString::fromUtf8("nameRegExp"));

        verticalLayout_3->addWidget(nameRegExp);


        verticalLayout_2->addWidget(groupBox);

        groupBox_2 = new QGroupBox(tab);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_5 = new QVBoxLayout(groupBox_2);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        listView = new QListWidget(groupBox_2);
        listView->setObjectName(QString::fromUtf8("listView"));

        horizontalLayout->addWidget(listView);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        addPath = new QPushButton(groupBox_2);
        addPath->setObjectName(QString::fromUtf8("addPath"));
        QIcon icon1;
        iconThemeName = QString::fromUtf8("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        addPath->setIcon(icon1);

        verticalLayout_4->addWidget(addPath);

        removePath = new QPushButton(groupBox_2);
        removePath->setObjectName(QString::fromUtf8("removePath"));
        QIcon icon2;
        iconThemeName = QString::fromUtf8("list-remove");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        removePath->setIcon(icon2);

        verticalLayout_4->addWidget(removePath);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);


        horizontalLayout->addLayout(verticalLayout_4);


        verticalLayout_5->addLayout(horizontalLayout);

        recursiveSearch = new QCheckBox(groupBox_2);
        recursiveSearch->setObjectName(QString::fromUtf8("recursiveSearch"));

        verticalLayout_5->addWidget(recursiveSearch);

        searchHidden = new QCheckBox(groupBox_2);
        searchHidden->setObjectName(QString::fromUtf8("searchHidden"));

        verticalLayout_5->addWidget(searchHidden);


        verticalLayout_2->addWidget(groupBox_2);

        verticalLayout_2->setStretch(1, 1);
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_7 = new QVBoxLayout(tab_2);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        groupBox_3 = new QGroupBox(tab_2);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_6 = new QVBoxLayout(groupBox_3);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        searchTextFiles = new QCheckBox(groupBox_3);
        searchTextFiles->setObjectName(QString::fromUtf8("searchTextFiles"));

        verticalLayout_6->addWidget(searchTextFiles);

        searchImages = new QCheckBox(groupBox_3);
        searchImages->setObjectName(QString::fromUtf8("searchImages"));

        verticalLayout_6->addWidget(searchImages);

        searchAudio = new QCheckBox(groupBox_3);
        searchAudio->setObjectName(QString::fromUtf8("searchAudio"));

        verticalLayout_6->addWidget(searchAudio);

        searchVideo = new QCheckBox(groupBox_3);
        searchVideo->setObjectName(QString::fromUtf8("searchVideo"));

        verticalLayout_6->addWidget(searchVideo);

        searchDocuments = new QCheckBox(groupBox_3);
        searchDocuments->setObjectName(QString::fromUtf8("searchDocuments"));

        verticalLayout_6->addWidget(searchDocuments);

        searchFolders = new QCheckBox(groupBox_3);
        searchFolders->setObjectName(QString::fromUtf8("searchFolders"));

        verticalLayout_6->addWidget(searchFolders);


        verticalLayout_7->addWidget(groupBox_3);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_7->addItem(verticalSpacer_2);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        verticalLayout_9 = new QVBoxLayout(tab_3);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        groupBox_4 = new QGroupBox(tab_3);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_8 = new QVBoxLayout(groupBox_4);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        contentPattern = new QLineEdit(groupBox_4);
        contentPattern->setObjectName(QString::fromUtf8("contentPattern"));

        verticalLayout_8->addWidget(contentPattern);

        contentCaseInsensitive = new QCheckBox(groupBox_4);
        contentCaseInsensitive->setObjectName(QString::fromUtf8("contentCaseInsensitive"));

        verticalLayout_8->addWidget(contentCaseInsensitive);

        contentRegExp = new QCheckBox(groupBox_4);
        contentRegExp->setObjectName(QString::fromUtf8("contentRegExp"));

        verticalLayout_8->addWidget(contentRegExp);


        verticalLayout_9->addWidget(groupBox_4);

        verticalSpacer_3 = new QSpacerItem(20, 186, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_3);

        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        verticalLayout_10 = new QVBoxLayout(tab_4);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        groupBox_5 = new QGroupBox(tab_4);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        formLayout = new QFormLayout(groupBox_5);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        largerThan = new QCheckBox(groupBox_5);
        largerThan->setObjectName(QString::fromUtf8("largerThan"));

        formLayout->setWidget(0, QFormLayout::LabelRole, largerThan);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        minSize = new QSpinBox(groupBox_5);
        minSize->setObjectName(QString::fromUtf8("minSize"));

        horizontalLayout_2->addWidget(minSize);

        minSizeUnit = new QComboBox(groupBox_5);
        minSizeUnit->addItem(QString());
        minSizeUnit->addItem(QString());
        minSizeUnit->addItem(QString());
        minSizeUnit->addItem(QString());
        minSizeUnit->setObjectName(QString::fromUtf8("minSizeUnit"));

        horizontalLayout_2->addWidget(minSizeUnit);


        formLayout->setLayout(0, QFormLayout::FieldRole, horizontalLayout_2);

        smallerThan = new QCheckBox(groupBox_5);
        smallerThan->setObjectName(QString::fromUtf8("smallerThan"));

        formLayout->setWidget(1, QFormLayout::LabelRole, smallerThan);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        maxSize = new QSpinBox(groupBox_5);
        maxSize->setObjectName(QString::fromUtf8("maxSize"));

        horizontalLayout_3->addWidget(maxSize);

        maxSizeUnit = new QComboBox(groupBox_5);
        maxSizeUnit->addItem(QString());
        maxSizeUnit->addItem(QString());
        maxSizeUnit->addItem(QString());
        maxSizeUnit->addItem(QString());
        maxSizeUnit->setObjectName(QString::fromUtf8("maxSizeUnit"));

        horizontalLayout_3->addWidget(maxSizeUnit);


        formLayout->setLayout(1, QFormLayout::FieldRole, horizontalLayout_3);


        verticalLayout_10->addWidget(groupBox_5);

        groupBox_6 = new QGroupBox(tab_4);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        formLayout_2 = new QFormLayout(groupBox_6);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        earlierThan = new QCheckBox(groupBox_6);
        earlierThan->setObjectName(QString::fromUtf8("earlierThan"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, earlierThan);

        laterThan = new QCheckBox(groupBox_6);
        laterThan->setObjectName(QString::fromUtf8("laterThan"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, laterThan);

        maxTime = new QDateEdit(groupBox_6);
        maxTime->setObjectName(QString::fromUtf8("maxTime"));
        maxTime->setCalendarPopup(true);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, maxTime);

        minTime = new QDateEdit(groupBox_6);
        minTime->setObjectName(QString::fromUtf8("minTime"));
        minTime->setCalendarPopup(true);

        formLayout_2->setWidget(1, QFormLayout::FieldRole, minTime);


        verticalLayout_10->addWidget(groupBox_6);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_4);

        tabWidget->addTab(tab_4, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(SearchDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(SearchDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), SearchDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SearchDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);
        minSizeUnit->setCurrentIndex(2);
        maxSizeUnit->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(SearchDialog);
    } // setupUi

    void retranslateUi(QDialog *SearchDialog)
    {
        SearchDialog->setWindowTitle(QCoreApplication::translate("SearchDialog", "Search Files", nullptr));
        groupBox->setTitle(QCoreApplication::translate("SearchDialog", "File Name Patterns:", nullptr));
        namePatterns->setText(QCoreApplication::translate("SearchDialog", "*", nullptr));
        nameCaseInsensitive->setText(QCoreApplication::translate("SearchDialog", "Case insensitive", nullptr));
        nameRegExp->setText(QCoreApplication::translate("SearchDialog", "Use regular expression", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("SearchDialog", "Places to Search:", nullptr));
        addPath->setText(QCoreApplication::translate("SearchDialog", "&Add", nullptr));
        removePath->setText(QCoreApplication::translate("SearchDialog", "&Remove", nullptr));
        recursiveSearch->setText(QCoreApplication::translate("SearchDialog", "Search in sub directories", nullptr));
        searchHidden->setText(QCoreApplication::translate("SearchDialog", "Search for hidden files", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("SearchDialog", "Name/Location", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("SearchDialog", "Only search for files of following types:", nullptr));
        searchTextFiles->setText(QCoreApplication::translate("SearchDialog", "Text files", nullptr));
        searchImages->setText(QCoreApplication::translate("SearchDialog", "Image files", nullptr));
        searchAudio->setText(QCoreApplication::translate("SearchDialog", "Audio files", nullptr));
        searchVideo->setText(QCoreApplication::translate("SearchDialog", "Video files", nullptr));
        searchDocuments->setText(QCoreApplication::translate("SearchDialog", "Documents", nullptr));
        searchFolders->setText(QCoreApplication::translate("SearchDialog", "Folders", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("SearchDialog", "File Type", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("SearchDialog", "File contains:", nullptr));
        contentCaseInsensitive->setText(QCoreApplication::translate("SearchDialog", "Case insensiti&ve", nullptr));
        contentRegExp->setText(QCoreApplication::translate("SearchDialog", "&Use regular expression", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("SearchDialog", "Content", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("SearchDialog", "File Size:", nullptr));
        largerThan->setText(QCoreApplication::translate("SearchDialog", "Larger than:", nullptr));
        minSizeUnit->setItemText(0, QCoreApplication::translate("SearchDialog", "Bytes", nullptr));
        minSizeUnit->setItemText(1, QCoreApplication::translate("SearchDialog", "KiB", nullptr));
        minSizeUnit->setItemText(2, QCoreApplication::translate("SearchDialog", "MiB", nullptr));
        minSizeUnit->setItemText(3, QCoreApplication::translate("SearchDialog", "GiB", nullptr));

        smallerThan->setText(QCoreApplication::translate("SearchDialog", "Smaller than:", nullptr));
        maxSizeUnit->setItemText(0, QCoreApplication::translate("SearchDialog", "Bytes", nullptr));
        maxSizeUnit->setItemText(1, QCoreApplication::translate("SearchDialog", "KiB", nullptr));
        maxSizeUnit->setItemText(2, QCoreApplication::translate("SearchDialog", "MiB", nullptr));
        maxSizeUnit->setItemText(3, QCoreApplication::translate("SearchDialog", "GiB", nullptr));

        groupBox_6->setTitle(QCoreApplication::translate("SearchDialog", "Last Modified Time:", nullptr));
        earlierThan->setText(QCoreApplication::translate("SearchDialog", "Earlier than:", nullptr));
        laterThan->setText(QCoreApplication::translate("SearchDialog", "Later than:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("SearchDialog", "Properties", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SearchDialog: public Ui_SearchDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILESEARCH_H
