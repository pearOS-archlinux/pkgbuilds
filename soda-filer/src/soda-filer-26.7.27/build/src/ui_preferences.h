/********************************************************************************
** Form generated from reading UI file 'preferences.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREFERENCES_H
#define UI_PREFERENCES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PreferencesDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QWidget *tabWidgetPage1;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout;
    QLabel *label_11;
    QComboBox *viewMode;
    QCheckBox *dirInfoWrite;
    QCheckBox *spatialMode;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_9;
    QCheckBox *configmDelete;
    QCheckBox *noUsbTrash;
    QCheckBox *confirmTrash;
    QCheckBox *quickExec;
    QSpacerItem *verticalSpacer_2;
    QWidget *tabWidgetPage2;
    QVBoxLayout *verticalLayout_7;
    QGroupBox *groupBox_2;
    QFormLayout *formLayout_2;
    QLabel *iconThemeLabel;
    QComboBox *iconTheme;
    QLabel *label_6;
    QComboBox *bigIconSize;
    QLabel *label_7;
    QComboBox *smallIconSize;
    QLabel *label_8;
    QComboBox *thumbnailIconSize;
    QLabel *label_9;
    QComboBox *sidePaneIconSize;
    QGroupBox *groupBox_9;
    QVBoxLayout *verticalLayout_8;
    QCheckBox *siUnit;
    QCheckBox *backupAsHidden;
    QCheckBox *showFullNames;
    QCheckBox *shadowHidden;
    QSpacerItem *verticalSpacer_5;
    QWidget *tabWidgetPage3;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox_5;
    QFormLayout *formLayout_7;
    QCheckBox *rememberWindowSize;
    QLabel *label_12;
    QSpinBox *fixedWindowWidth;
    QLabel *label_13;
    QSpinBox *fixedWindowHeight;
    QGroupBox *groupBox_10;
    QVBoxLayout *verticalLayout_10;
    QListWidget *showInPlaces;
    QSpacerItem *verticalSpacer_4;
    QWidget *tabWidgetPage4;
    QVBoxLayout *verticalLayout_11;
    QGroupBox *groupBox_8;
    QFormLayout *formLayout_3;
    QLabel *label_10;
    QSpinBox *maxThumbnailFileSize;
    QCheckBox *thumbnailLocal;
    QCheckBox *showThumbnails;
    QSpacerItem *verticalSpacer_6;
    QWidget *tabWidgetPage5;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *programsGroupBox;
    QFormLayout *formLayout;
    QLabel *label_14;
    QLabel *label_3;
    QLineEdit *suCommand;
    QLabel *label_4;
    QComboBox *terminal;
    QGroupBox *groupBox_11;
    QVBoxLayout *verticalLayout_12;
    QCheckBox *onlyUserTemplates;
    QCheckBox *templateTypeOnce;
    QCheckBox *templateRunApp;
    QSpacerItem *verticalSpacer_3;
    QWidget *tabWidgetPage6;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout_4;
    QCheckBox *mountOnStartup;
    QCheckBox *mountRemovable;
    QCheckBox *autoRun;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_6;
    QRadioButton *closeOnUnmount;
    QRadioButton *goHomeOnUnmount;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *PreferencesDialog)
    {
        if (PreferencesDialog->objectName().isEmpty())
            PreferencesDialog->setObjectName(QString::fromUtf8("PreferencesDialog"));
        PreferencesDialog->resize(524, 402);
        verticalLayout = new QVBoxLayout(PreferencesDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tabWidget = new QTabWidget(PreferencesDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        tabWidgetPage1 = new QWidget();
        tabWidgetPage1->setObjectName(QString::fromUtf8("tabWidgetPage1"));
        verticalLayout_3 = new QVBoxLayout(tabWidgetPage1);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox_4 = new QGroupBox(tabWidgetPage1);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout = new QGridLayout(groupBox_4);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_11 = new QLabel(groupBox_4);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout->addWidget(label_11, 4, 0, 1, 1);

        viewMode = new QComboBox(groupBox_4);
        viewMode->setObjectName(QString::fromUtf8("viewMode"));

        gridLayout->addWidget(viewMode, 4, 1, 1, 1);

        dirInfoWrite = new QCheckBox(groupBox_4);
        dirInfoWrite->setObjectName(QString::fromUtf8("dirInfoWrite"));
        dirInfoWrite->setChecked(false);

        gridLayout->addWidget(dirInfoWrite, 3, 0, 1, 2);

        spatialMode = new QCheckBox(groupBox_4);
        spatialMode->setObjectName(QString::fromUtf8("spatialMode"));

        gridLayout->addWidget(spatialMode, 2, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox_4);

        groupBox_6 = new QGroupBox(tabWidgetPage1);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        verticalLayout_9 = new QVBoxLayout(groupBox_6);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        configmDelete = new QCheckBox(groupBox_6);
        configmDelete->setObjectName(QString::fromUtf8("configmDelete"));

        verticalLayout_9->addWidget(configmDelete);

        noUsbTrash = new QCheckBox(groupBox_6);
        noUsbTrash->setObjectName(QString::fromUtf8("noUsbTrash"));

        verticalLayout_9->addWidget(noUsbTrash);

        confirmTrash = new QCheckBox(groupBox_6);
        confirmTrash->setObjectName(QString::fromUtf8("confirmTrash"));

        verticalLayout_9->addWidget(confirmTrash);

        quickExec = new QCheckBox(groupBox_6);
        quickExec->setObjectName(QString::fromUtf8("quickExec"));

        verticalLayout_9->addWidget(quickExec);


        verticalLayout_3->addWidget(groupBox_6);

        verticalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        verticalLayout_3->setStretch(2, 1);
        tabWidget->addTab(tabWidgetPage1, QString());
        tabWidgetPage2 = new QWidget();
        tabWidgetPage2->setObjectName(QString::fromUtf8("tabWidgetPage2"));
        verticalLayout_7 = new QVBoxLayout(tabWidgetPage2);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        groupBox_2 = new QGroupBox(tabWidgetPage2);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        formLayout_2 = new QFormLayout(groupBox_2);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        iconThemeLabel = new QLabel(groupBox_2);
        iconThemeLabel->setObjectName(QString::fromUtf8("iconThemeLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(iconThemeLabel->sizePolicy().hasHeightForWidth());
        iconThemeLabel->setSizePolicy(sizePolicy1);

        formLayout_2->setWidget(0, QFormLayout::LabelRole, iconThemeLabel);

        iconTheme = new QComboBox(groupBox_2);
        iconTheme->setObjectName(QString::fromUtf8("iconTheme"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(iconTheme->sizePolicy().hasHeightForWidth());
        iconTheme->setSizePolicy(sizePolicy2);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, iconTheme);

        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_6);

        bigIconSize = new QComboBox(groupBox_2);
        bigIconSize->setObjectName(QString::fromUtf8("bigIconSize"));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, bigIconSize);

        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_7);

        smallIconSize = new QComboBox(groupBox_2);
        smallIconSize->setObjectName(QString::fromUtf8("smallIconSize"));

        formLayout_2->setWidget(2, QFormLayout::FieldRole, smallIconSize);

        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_8);

        thumbnailIconSize = new QComboBox(groupBox_2);
        thumbnailIconSize->setObjectName(QString::fromUtf8("thumbnailIconSize"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, thumbnailIconSize);

        label_9 = new QLabel(groupBox_2);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_9);

        sidePaneIconSize = new QComboBox(groupBox_2);
        sidePaneIconSize->setObjectName(QString::fromUtf8("sidePaneIconSize"));

        formLayout_2->setWidget(4, QFormLayout::FieldRole, sidePaneIconSize);


        verticalLayout_7->addWidget(groupBox_2);

        groupBox_9 = new QGroupBox(tabWidgetPage2);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        verticalLayout_8 = new QVBoxLayout(groupBox_9);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        siUnit = new QCheckBox(groupBox_9);
        siUnit->setObjectName(QString::fromUtf8("siUnit"));

        verticalLayout_8->addWidget(siUnit);

        backupAsHidden = new QCheckBox(groupBox_9);
        backupAsHidden->setObjectName(QString::fromUtf8("backupAsHidden"));

        verticalLayout_8->addWidget(backupAsHidden);

        showFullNames = new QCheckBox(groupBox_9);
        showFullNames->setObjectName(QString::fromUtf8("showFullNames"));
        showFullNames->setEnabled(false);

        verticalLayout_8->addWidget(showFullNames);

        shadowHidden = new QCheckBox(groupBox_9);
        shadowHidden->setObjectName(QString::fromUtf8("shadowHidden"));
        shadowHidden->setEnabled(false);

        verticalLayout_8->addWidget(shadowHidden);


        verticalLayout_7->addWidget(groupBox_9);

        verticalSpacer_5 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_7->addItem(verticalSpacer_5);

        verticalLayout_7->setStretch(2, 1);
        tabWidget->addTab(tabWidgetPage2, QString());
        tabWidgetPage3 = new QWidget();
        tabWidgetPage3->setObjectName(QString::fromUtf8("tabWidgetPage3"));
        verticalLayout_5 = new QVBoxLayout(tabWidgetPage3);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        groupBox_5 = new QGroupBox(tabWidgetPage3);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        formLayout_7 = new QFormLayout(groupBox_5);
        formLayout_7->setObjectName(QString::fromUtf8("formLayout_7"));
        formLayout_7->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        rememberWindowSize = new QCheckBox(groupBox_5);
        rememberWindowSize->setObjectName(QString::fromUtf8("rememberWindowSize"));

        formLayout_7->setWidget(0, QFormLayout::SpanningRole, rememberWindowSize);

        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        formLayout_7->setWidget(1, QFormLayout::LabelRole, label_12);

        fixedWindowWidth = new QSpinBox(groupBox_5);
        fixedWindowWidth->setObjectName(QString::fromUtf8("fixedWindowWidth"));
        fixedWindowWidth->setMaximum(32768);

        formLayout_7->setWidget(1, QFormLayout::FieldRole, fixedWindowWidth);

        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        formLayout_7->setWidget(2, QFormLayout::LabelRole, label_13);

        fixedWindowHeight = new QSpinBox(groupBox_5);
        fixedWindowHeight->setObjectName(QString::fromUtf8("fixedWindowHeight"));
        fixedWindowHeight->setMaximum(32768);

        formLayout_7->setWidget(2, QFormLayout::FieldRole, fixedWindowHeight);


        verticalLayout_5->addWidget(groupBox_5);

        groupBox_10 = new QGroupBox(tabWidgetPage3);
        groupBox_10->setObjectName(QString::fromUtf8("groupBox_10"));
        verticalLayout_10 = new QVBoxLayout(groupBox_10);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        showInPlaces = new QListWidget(groupBox_10);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("user-home");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem->setCheckState(Qt::Unchecked);
        __qlistwidgetitem->setIcon(icon);
        QIcon icon1;
        iconThemeName = QString::fromUtf8("user-desktop");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem1 = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem1->setCheckState(Qt::Unchecked);
        __qlistwidgetitem1->setIcon(icon1);
        QIcon icon2;
        iconThemeName = QString::fromUtf8("user-trash");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem2 = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem2->setCheckState(Qt::Unchecked);
        __qlistwidgetitem2->setIcon(icon2);
        QIcon icon3;
        iconThemeName = QString::fromUtf8("computer");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem3 = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem3->setCheckState(Qt::Unchecked);
        __qlistwidgetitem3->setIcon(icon3);
        QListWidgetItem *__qlistwidgetitem4 = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem4->setCheckState(Qt::Unchecked);
        QListWidgetItem *__qlistwidgetitem5 = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem5->setCheckState(Qt::Unchecked);
        QIcon icon4;
        iconThemeName = QString::fromUtf8("folder-network");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon4 = QIcon::fromTheme(iconThemeName);
        } else {
            icon4.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem6 = new QListWidgetItem(showInPlaces);
        __qlistwidgetitem6->setCheckState(Qt::Unchecked);
        __qlistwidgetitem6->setIcon(icon4);
        showInPlaces->setObjectName(QString::fromUtf8("showInPlaces"));

        verticalLayout_10->addWidget(showInPlaces);


        verticalLayout_5->addWidget(groupBox_10);

        verticalSpacer_4 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_4);

        verticalLayout_5->setStretch(2, 1);
        tabWidget->addTab(tabWidgetPage3, QString());
        tabWidgetPage4 = new QWidget();
        tabWidgetPage4->setObjectName(QString::fromUtf8("tabWidgetPage4"));
        verticalLayout_11 = new QVBoxLayout(tabWidgetPage4);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        groupBox_8 = new QGroupBox(tabWidgetPage4);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        formLayout_3 = new QFormLayout(groupBox_8);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        label_10 = new QLabel(groupBox_8);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        formLayout_3->setWidget(2, QFormLayout::LabelRole, label_10);

        maxThumbnailFileSize = new QSpinBox(groupBox_8);
        maxThumbnailFileSize->setObjectName(QString::fromUtf8("maxThumbnailFileSize"));
        maxThumbnailFileSize->setMaximum(1048576);

        formLayout_3->setWidget(2, QFormLayout::FieldRole, maxThumbnailFileSize);

        thumbnailLocal = new QCheckBox(groupBox_8);
        thumbnailLocal->setObjectName(QString::fromUtf8("thumbnailLocal"));

        formLayout_3->setWidget(1, QFormLayout::SpanningRole, thumbnailLocal);

        showThumbnails = new QCheckBox(groupBox_8);
        showThumbnails->setObjectName(QString::fromUtf8("showThumbnails"));

        formLayout_3->setWidget(0, QFormLayout::SpanningRole, showThumbnails);


        verticalLayout_11->addWidget(groupBox_8);

        verticalSpacer_6 = new QSpacerItem(0, 225, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_11->addItem(verticalSpacer_6);

        tabWidget->addTab(tabWidgetPage4, QString());
        tabWidgetPage5 = new QWidget();
        tabWidgetPage5->setObjectName(QString::fromUtf8("tabWidgetPage5"));
        verticalLayout_2 = new QVBoxLayout(tabWidgetPage5);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        programsGroupBox = new QGroupBox(tabWidgetPage5);
        programsGroupBox->setObjectName(QString::fromUtf8("programsGroupBox"));
        formLayout = new QFormLayout(programsGroupBox);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label_14 = new QLabel(programsGroupBox);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_14);

        label_3 = new QLabel(programsGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

        suCommand = new QLineEdit(programsGroupBox);
        suCommand->setObjectName(QString::fromUtf8("suCommand"));

        formLayout->setWidget(1, QFormLayout::FieldRole, suCommand);

        label_4 = new QLabel(programsGroupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setTextFormat(Qt::PlainText);

        formLayout->setWidget(2, QFormLayout::SpanningRole, label_4);

        terminal = new QComboBox(programsGroupBox);
        terminal->setObjectName(QString::fromUtf8("terminal"));
        terminal->setEditable(true);

        formLayout->setWidget(0, QFormLayout::FieldRole, terminal);


        verticalLayout_2->addWidget(programsGroupBox);

        groupBox_11 = new QGroupBox(tabWidgetPage5);
        groupBox_11->setObjectName(QString::fromUtf8("groupBox_11"));
        verticalLayout_12 = new QVBoxLayout(groupBox_11);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        onlyUserTemplates = new QCheckBox(groupBox_11);
        onlyUserTemplates->setObjectName(QString::fromUtf8("onlyUserTemplates"));

        verticalLayout_12->addWidget(onlyUserTemplates);

        templateTypeOnce = new QCheckBox(groupBox_11);
        templateTypeOnce->setObjectName(QString::fromUtf8("templateTypeOnce"));

        verticalLayout_12->addWidget(templateTypeOnce);

        templateRunApp = new QCheckBox(groupBox_11);
        templateRunApp->setObjectName(QString::fromUtf8("templateRunApp"));

        verticalLayout_12->addWidget(templateRunApp);


        verticalLayout_2->addWidget(groupBox_11);

        verticalSpacer_3 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_3);

        verticalLayout_2->setStretch(2, 1);
        tabWidget->addTab(tabWidgetPage5, QString());
        tabWidgetPage6 = new QWidget();
        tabWidgetPage6->setObjectName(QString::fromUtf8("tabWidgetPage6"));
        tabWidgetPage6->setEnabled(false);
        verticalLayout_4 = new QVBoxLayout(tabWidgetPage6);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        groupBox_3 = new QGroupBox(tabWidgetPage6);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        formLayout_4 = new QFormLayout(groupBox_3);
        formLayout_4->setObjectName(QString::fromUtf8("formLayout_4"));
        mountOnStartup = new QCheckBox(groupBox_3);
        mountOnStartup->setObjectName(QString::fromUtf8("mountOnStartup"));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, mountOnStartup);

        mountRemovable = new QCheckBox(groupBox_3);
        mountRemovable->setObjectName(QString::fromUtf8("mountRemovable"));

        formLayout_4->setWidget(1, QFormLayout::LabelRole, mountRemovable);

        autoRun = new QCheckBox(groupBox_3);
        autoRun->setObjectName(QString::fromUtf8("autoRun"));

        formLayout_4->setWidget(2, QFormLayout::LabelRole, autoRun);


        verticalLayout_4->addWidget(groupBox_3);

        groupBox_7 = new QGroupBox(tabWidgetPage6);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        verticalLayout_6 = new QVBoxLayout(groupBox_7);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        closeOnUnmount = new QRadioButton(groupBox_7);
        closeOnUnmount->setObjectName(QString::fromUtf8("closeOnUnmount"));

        verticalLayout_6->addWidget(closeOnUnmount);

        goHomeOnUnmount = new QRadioButton(groupBox_7);
        goHomeOnUnmount->setObjectName(QString::fromUtf8("goHomeOnUnmount"));

        verticalLayout_6->addWidget(goHomeOnUnmount);


        verticalLayout_4->addWidget(groupBox_7);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        verticalLayout_4->setStretch(2, 1);
        tabWidget->addTab(tabWidgetPage6, QString());

        horizontalLayout->addWidget(tabWidget);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(PreferencesDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        QWidget::setTabOrder(rememberWindowSize, fixedWindowWidth);
        QWidget::setTabOrder(fixedWindowWidth, fixedWindowHeight);
        QWidget::setTabOrder(fixedWindowHeight, configmDelete);
        QWidget::setTabOrder(configmDelete, mountOnStartup);
        QWidget::setTabOrder(mountOnStartup, mountRemovable);
        QWidget::setTabOrder(mountRemovable, autoRun);
        QWidget::setTabOrder(autoRun, closeOnUnmount);
        QWidget::setTabOrder(closeOnUnmount, goHomeOnUnmount);
        QWidget::setTabOrder(goHomeOnUnmount, terminal);
        QWidget::setTabOrder(terminal, suCommand);

        retranslateUi(PreferencesDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), PreferencesDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), PreferencesDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(PreferencesDialog);
    } // setupUi

    void retranslateUi(QDialog *PreferencesDialog)
    {
        PreferencesDialog->setWindowTitle(QCoreApplication::translate("PreferencesDialog", "Preferences", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("PreferencesDialog", "Browsing", nullptr));
        label_11->setText(QCoreApplication::translate("PreferencesDialog", "Default view mode:", nullptr));
        dirInfoWrite->setText(QCoreApplication::translate("PreferencesDialog", "Save metadata to directories (.DirInfo files)", nullptr));
        spatialMode->setText(QCoreApplication::translate("PreferencesDialog", "Spatial mode (folders open in a new window)", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("PreferencesDialog", "File Operations", nullptr));
        configmDelete->setText(QCoreApplication::translate("PreferencesDialog", "Confirm before deleting files", nullptr));
        noUsbTrash->setText(QCoreApplication::translate("PreferencesDialog", "Erase files on removable media instead of \"trash can\" creation", nullptr));
        confirmTrash->setText(QCoreApplication::translate("PreferencesDialog", "Confirm before moving files into \"trash can\"", nullptr));
        quickExec->setText(QCoreApplication::translate("PreferencesDialog", "Don't ask options on launch executable file", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage1), QCoreApplication::translate("PreferencesDialog", "Behavior", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("PreferencesDialog", "Icons", nullptr));
        iconThemeLabel->setText(QCoreApplication::translate("PreferencesDialog", "Icon theme:", nullptr));
        label_6->setText(QCoreApplication::translate("PreferencesDialog", "Size of big icons:", nullptr));
        label_7->setText(QCoreApplication::translate("PreferencesDialog", "Size of small icons:", nullptr));
        label_8->setText(QCoreApplication::translate("PreferencesDialog", "Size of thumbnails:", nullptr));
        label_9->setText(QCoreApplication::translate("PreferencesDialog", "Size of side pane icons:", nullptr));
        groupBox_9->setTitle(QCoreApplication::translate("PreferencesDialog", "User interface", nullptr));
        siUnit->setText(QCoreApplication::translate("PreferencesDialog", "Use SI decimal prefixes instead of IEC binary prefixes", nullptr));
        backupAsHidden->setText(QCoreApplication::translate("PreferencesDialog", "Treat backup files as hidden", nullptr));
        showFullNames->setText(QCoreApplication::translate("PreferencesDialog", "Always show full file names", nullptr));
        shadowHidden->setText(QCoreApplication::translate("PreferencesDialog", "Show icons of hidden files shadowed", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage2), QCoreApplication::translate("PreferencesDialog", "Display", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("PreferencesDialog", "Window", nullptr));
        rememberWindowSize->setText(QCoreApplication::translate("PreferencesDialog", "Remember the size of the last closed window", nullptr));
        label_12->setText(QCoreApplication::translate("PreferencesDialog", "Default width of new windows:", nullptr));
        label_13->setText(QCoreApplication::translate("PreferencesDialog", "Default height of new windows:", nullptr));
        groupBox_10->setTitle(QCoreApplication::translate("PreferencesDialog", "Show in places", nullptr));

        const bool __sortingEnabled = showInPlaces->isSortingEnabled();
        showInPlaces->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = showInPlaces->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("PreferencesDialog", "Home", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = showInPlaces->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("PreferencesDialog", "Desktop", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = showInPlaces->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("PreferencesDialog", "Trash can", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = showInPlaces->item(3);
        ___qlistwidgetitem3->setText(QCoreApplication::translate("PreferencesDialog", "Computer", nullptr));
        QListWidgetItem *___qlistwidgetitem4 = showInPlaces->item(4);
        ___qlistwidgetitem4->setText(QCoreApplication::translate("PreferencesDialog", "Applications", nullptr));
        QListWidgetItem *___qlistwidgetitem5 = showInPlaces->item(5);
        ___qlistwidgetitem5->setText(QCoreApplication::translate("PreferencesDialog", "Devices", nullptr));
        QListWidgetItem *___qlistwidgetitem6 = showInPlaces->item(6);
        ___qlistwidgetitem6->setText(QCoreApplication::translate("PreferencesDialog", "Network", nullptr));
        showInPlaces->setSortingEnabled(__sortingEnabled);

        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage3), QCoreApplication::translate("PreferencesDialog", "User Interface", nullptr));
        groupBox_8->setTitle(QCoreApplication::translate("PreferencesDialog", "Thumbnail", nullptr));
        label_10->setText(QCoreApplication::translate("PreferencesDialog", "Do not generate thumbnails for image files exceeding this size:", nullptr));
        maxThumbnailFileSize->setSuffix(QCoreApplication::translate("PreferencesDialog", " KB", nullptr));
        thumbnailLocal->setText(QCoreApplication::translate("PreferencesDialog", "Only show thumbnails for local files", nullptr));
        showThumbnails->setText(QCoreApplication::translate("PreferencesDialog", "Show thumbnails of files", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage4), QCoreApplication::translate("PreferencesDialog", "Thumbnail", nullptr));
        programsGroupBox->setTitle(QCoreApplication::translate("PreferencesDialog", "Programs", nullptr));
        label_14->setText(QCoreApplication::translate("PreferencesDialog", "Terminal emulator:", nullptr));
        label_3->setText(QCoreApplication::translate("PreferencesDialog", "Switch &user command:", nullptr));
        label_4->setText(QCoreApplication::translate("PreferencesDialog", "Examples: \"xterm -e %s\" for terminal or \"gksu %s\" for switching user.\n"
"%s = the command line you want to execute with terminal or su.", nullptr));
        groupBox_11->setTitle(QCoreApplication::translate("PreferencesDialog", "Templates", nullptr));
        onlyUserTemplates->setText(QCoreApplication::translate("PreferencesDialog", "Show only user defined templates in menu", nullptr));
        templateTypeOnce->setText(QCoreApplication::translate("PreferencesDialog", "Show only one template for each MIME type", nullptr));
        templateRunApp->setText(QCoreApplication::translate("PreferencesDialog", "Run default application after creation from template", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage5), QCoreApplication::translate("PreferencesDialog", "Advanced", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("PreferencesDialog", "Auto Mount", nullptr));
        mountOnStartup->setText(QCoreApplication::translate("PreferencesDialog", "Mount mountable volumes automatically on program startup", nullptr));
        mountRemovable->setText(QCoreApplication::translate("PreferencesDialog", "Mount removable media automatically when they are inserted", nullptr));
        autoRun->setText(QCoreApplication::translate("PreferencesDialog", "Show available options for removable media when they are inserted", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("PreferencesDialog", "When removable medium unmounted:", nullptr));
        closeOnUnmount->setText(QCoreApplication::translate("PreferencesDialog", "Close &tab containing removable medium", nullptr));
        goHomeOnUnmount->setText(QCoreApplication::translate("PreferencesDialog", "Chan&ge folder in the tab to home folder", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage6), QCoreApplication::translate("PreferencesDialog", "Mount", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PreferencesDialog: public Ui_PreferencesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREFERENCES_H
