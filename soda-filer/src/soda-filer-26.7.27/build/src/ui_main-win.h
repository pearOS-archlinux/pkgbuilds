/********************************************************************************
** Form generated from reading UI file 'main-win.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_2D_WIN_H
#define UI_MAIN_2D_WIN_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "sidepane.h"
#include "tabbar.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionGoUp;
    QAction *actionHome;
    QAction *actionReload;
    QAction *actionGo;
    QAction *actionQuit;
    QAction *actionAbout;
    QAction *actionNewWin;
    QAction *actionShowHidden;
    QAction *actionComputer;
    QAction *actionTrash;
    QAction *actionNetwork;
    QAction *actionDesktop;
    QAction *actionAddToBookmarks;
    QAction *actionApplications;
    QAction *actionReload_2;
    QAction *actionIconView;
    QAction *actionDetailedList;
    QAction *actionThumbnailView;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionSelectAll;
    QAction *actionPreferences;
    QAction *actionAscending;
    QAction *actionDescending;
    QAction *actionByFileName;
    QAction *actionByMTime;
    QAction *actionByFileType;
    QAction *actionByOwner;
    QAction *actionFolderFirst;
    QAction *actionNewTab;
    QAction *actionGoBack;
    QAction *actionGoForward;
    QAction *actionInvertSelection;
    QAction *actionDelete;
    QAction *actionRename;
    QAction *actionCloseTab;
    QAction *actionFileProperties;
    QAction *actionCaseSensitive;
    QAction *actionByFileSize;
    QAction *actionCloseWindow;
    QAction *actionEdit_Bookmarks;
    QAction *actionOpenTerminal;
    QAction *actionOpenAsRoot;
    QAction *actionEditBookmarks;
    QAction *actionNewFolder;
    QAction *actionNewBlankFile;
    QAction *actionFindFiles;
    QAction *actionFilter;
    QAction *actionGoToFolder;
    QAction *actionDownloads;
    QAction *actionUtilities;
    QAction *actionDocuments;
    QAction *actionOpen;
    QAction *actionDuplicate;
    QAction *actionEmptyTrash;
    QAction *actionShowContents;
    QAction *actionGoUpAndCloseCurrentWindow;
    QAction *actionOpenWith;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    Fm::SidePane *sidePane;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    Filer::TabBar *tabBar;
    QStackedWidget *stackedWidget;
    QMenuBar *menubar;
    QMenu *menu_File;
    QMenu *menuCreateNew;
    QMenu *menu_Help;
    QMenu *menu_View;
    QMenu *menuSorting;
    QMenu *menu_Editw;
    QMenu *menu_Bookmarks;
    QMenu *menu_Go;
    QMenu *menu_Tool;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(460, 369);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("folder");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        MainWindow->setWindowIcon(icon);
        actionGoUp = new QAction(MainWindow);
        actionGoUp->setObjectName(QString::fromUtf8("actionGoUp"));
        QIcon icon1;
        iconThemeName = QString::fromUtf8("go-up");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionGoUp->setIcon(icon1);
        actionHome = new QAction(MainWindow);
        actionHome->setObjectName(QString::fromUtf8("actionHome"));
        QIcon icon2;
        iconThemeName = QString::fromUtf8("user-home");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionHome->setIcon(icon2);
        actionReload = new QAction(MainWindow);
        actionReload->setObjectName(QString::fromUtf8("actionReload"));
        QIcon icon3;
        iconThemeName = QString::fromUtf8("view-refresh");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionReload->setIcon(icon3);
        actionReload->setVisible(true);
        actionGo = new QAction(MainWindow);
        actionGo->setObjectName(QString::fromUtf8("actionGo"));
        QIcon icon4;
        iconThemeName = QString::fromUtf8("go-jump");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon4 = QIcon::fromTheme(iconThemeName);
        } else {
            icon4.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionGo->setIcon(icon4);
        actionGo->setVisible(false);
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        QIcon icon5;
        iconThemeName = QString::fromUtf8("application-exit");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon5 = QIcon::fromTheme(iconThemeName);
        } else {
            icon5.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionQuit->setIcon(icon5);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        QIcon icon6;
        iconThemeName = QString::fromUtf8("help-about");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon6 = QIcon::fromTheme(iconThemeName);
        } else {
            icon6.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionAbout->setIcon(icon6);
        actionNewWin = new QAction(MainWindow);
        actionNewWin->setObjectName(QString::fromUtf8("actionNewWin"));
        QIcon icon7;
        iconThemeName = QString::fromUtf8("window-new");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon7 = QIcon::fromTheme(iconThemeName);
        } else {
            icon7.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionNewWin->setIcon(icon7);
        actionShowHidden = new QAction(MainWindow);
        actionShowHidden->setObjectName(QString::fromUtf8("actionShowHidden"));
        actionShowHidden->setCheckable(true);
        actionComputer = new QAction(MainWindow);
        actionComputer->setObjectName(QString::fromUtf8("actionComputer"));
        QIcon icon8;
        iconThemeName = QString::fromUtf8("computer");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon8 = QIcon::fromTheme(iconThemeName);
        } else {
            icon8.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionComputer->setIcon(icon8);
        actionTrash = new QAction(MainWindow);
        actionTrash->setObjectName(QString::fromUtf8("actionTrash"));
        QIcon icon9;
        iconThemeName = QString::fromUtf8("user-trash");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon9 = QIcon::fromTheme(iconThemeName);
        } else {
            icon9.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionTrash->setIcon(icon9);
        actionNetwork = new QAction(MainWindow);
        actionNetwork->setObjectName(QString::fromUtf8("actionNetwork"));
        actionDesktop = new QAction(MainWindow);
        actionDesktop->setObjectName(QString::fromUtf8("actionDesktop"));
        QIcon icon10;
        iconThemeName = QString::fromUtf8("user-desktop");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon10 = QIcon::fromTheme(iconThemeName);
        } else {
            icon10.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionDesktop->setIcon(icon10);
        actionAddToBookmarks = new QAction(MainWindow);
        actionAddToBookmarks->setObjectName(QString::fromUtf8("actionAddToBookmarks"));
        QIcon icon11;
        iconThemeName = QString::fromUtf8("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon11 = QIcon::fromTheme(iconThemeName);
        } else {
            icon11.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionAddToBookmarks->setIcon(icon11);
        actionApplications = new QAction(MainWindow);
        actionApplications->setObjectName(QString::fromUtf8("actionApplications"));
        actionApplications->setVisible(true);
        actionReload_2 = new QAction(MainWindow);
        actionReload_2->setObjectName(QString::fromUtf8("actionReload_2"));
        actionIconView = new QAction(MainWindow);
        actionIconView->setObjectName(QString::fromUtf8("actionIconView"));
        actionIconView->setCheckable(true);
        actionDetailedList = new QAction(MainWindow);
        actionDetailedList->setObjectName(QString::fromUtf8("actionDetailedList"));
        actionDetailedList->setCheckable(true);
        actionThumbnailView = new QAction(MainWindow);
        actionThumbnailView->setObjectName(QString::fromUtf8("actionThumbnailView"));
        actionThumbnailView->setCheckable(true);
        actionCut = new QAction(MainWindow);
        actionCut->setObjectName(QString::fromUtf8("actionCut"));
        QIcon icon12;
        iconThemeName = QString::fromUtf8("edit-cut");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon12 = QIcon::fromTheme(iconThemeName);
        } else {
            icon12.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionCut->setIcon(icon12);
        actionCopy = new QAction(MainWindow);
        actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
        QIcon icon13;
        iconThemeName = QString::fromUtf8("edit-copy");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon13 = QIcon::fromTheme(iconThemeName);
        } else {
            icon13.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionCopy->setIcon(icon13);
        actionPaste = new QAction(MainWindow);
        actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
        QIcon icon14;
        iconThemeName = QString::fromUtf8("edit-paste");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon14 = QIcon::fromTheme(iconThemeName);
        } else {
            icon14.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionPaste->setIcon(icon14);
        actionSelectAll = new QAction(MainWindow);
        actionSelectAll->setObjectName(QString::fromUtf8("actionSelectAll"));
        actionPreferences = new QAction(MainWindow);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        actionAscending = new QAction(MainWindow);
        actionAscending->setObjectName(QString::fromUtf8("actionAscending"));
        actionAscending->setCheckable(true);
        actionDescending = new QAction(MainWindow);
        actionDescending->setObjectName(QString::fromUtf8("actionDescending"));
        actionDescending->setCheckable(true);
        actionByFileName = new QAction(MainWindow);
        actionByFileName->setObjectName(QString::fromUtf8("actionByFileName"));
        actionByFileName->setCheckable(true);
        actionByMTime = new QAction(MainWindow);
        actionByMTime->setObjectName(QString::fromUtf8("actionByMTime"));
        actionByMTime->setCheckable(true);
        actionByFileType = new QAction(MainWindow);
        actionByFileType->setObjectName(QString::fromUtf8("actionByFileType"));
        actionByFileType->setCheckable(true);
        actionByOwner = new QAction(MainWindow);
        actionByOwner->setObjectName(QString::fromUtf8("actionByOwner"));
        actionByOwner->setCheckable(true);
        actionFolderFirst = new QAction(MainWindow);
        actionFolderFirst->setObjectName(QString::fromUtf8("actionFolderFirst"));
        actionFolderFirst->setCheckable(true);
        actionNewTab = new QAction(MainWindow);
        actionNewTab->setObjectName(QString::fromUtf8("actionNewTab"));
        actionNewTab->setIcon(icon7);
        actionNewTab->setVisible(false);
        actionGoBack = new QAction(MainWindow);
        actionGoBack->setObjectName(QString::fromUtf8("actionGoBack"));
        QIcon icon15;
        iconThemeName = QString::fromUtf8("go-previous");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon15 = QIcon::fromTheme(iconThemeName);
        } else {
            icon15.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionGoBack->setIcon(icon15);
        actionGoForward = new QAction(MainWindow);
        actionGoForward->setObjectName(QString::fromUtf8("actionGoForward"));
        QIcon icon16;
        iconThemeName = QString::fromUtf8("go-next");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon16 = QIcon::fromTheme(iconThemeName);
        } else {
            icon16.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionGoForward->setIcon(icon16);
        actionGoForward->setVisible(true);
        actionGoForward->setIconVisibleInMenu(true);
        actionGoForward->setShortcutVisibleInContextMenu(true);
        actionInvertSelection = new QAction(MainWindow);
        actionInvertSelection->setObjectName(QString::fromUtf8("actionInvertSelection"));
        actionDelete = new QAction(MainWindow);
        actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
        actionDelete->setIcon(icon9);
        actionRename = new QAction(MainWindow);
        actionRename->setObjectName(QString::fromUtf8("actionRename"));
        actionCloseTab = new QAction(MainWindow);
        actionCloseTab->setObjectName(QString::fromUtf8("actionCloseTab"));
        actionFileProperties = new QAction(MainWindow);
        actionFileProperties->setObjectName(QString::fromUtf8("actionFileProperties"));
        actionCaseSensitive = new QAction(MainWindow);
        actionCaseSensitive->setObjectName(QString::fromUtf8("actionCaseSensitive"));
        actionCaseSensitive->setCheckable(true);
        actionByFileSize = new QAction(MainWindow);
        actionByFileSize->setObjectName(QString::fromUtf8("actionByFileSize"));
        actionByFileSize->setCheckable(true);
        actionCloseWindow = new QAction(MainWindow);
        actionCloseWindow->setObjectName(QString::fromUtf8("actionCloseWindow"));
        actionEdit_Bookmarks = new QAction(MainWindow);
        actionEdit_Bookmarks->setObjectName(QString::fromUtf8("actionEdit_Bookmarks"));
        actionOpenTerminal = new QAction(MainWindow);
        actionOpenTerminal->setObjectName(QString::fromUtf8("actionOpenTerminal"));
        actionOpenAsRoot = new QAction(MainWindow);
        actionOpenAsRoot->setObjectName(QString::fromUtf8("actionOpenAsRoot"));
        actionEditBookmarks = new QAction(MainWindow);
        actionEditBookmarks->setObjectName(QString::fromUtf8("actionEditBookmarks"));
        actionNewFolder = new QAction(MainWindow);
        actionNewFolder->setObjectName(QString::fromUtf8("actionNewFolder"));
        QIcon icon17;
        iconThemeName = QString::fromUtf8("folder-new");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon17 = QIcon::fromTheme(iconThemeName);
        } else {
            icon17.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionNewFolder->setIcon(icon17);
        actionNewBlankFile = new QAction(MainWindow);
        actionNewBlankFile->setObjectName(QString::fromUtf8("actionNewBlankFile"));
        QIcon icon18;
        iconThemeName = QString::fromUtf8("document-new");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon18 = QIcon::fromTheme(iconThemeName);
        } else {
            icon18.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionNewBlankFile->setIcon(icon18);
        actionFindFiles = new QAction(MainWindow);
        actionFindFiles->setObjectName(QString::fromUtf8("actionFindFiles"));
        actionFilter = new QAction(MainWindow);
        actionFilter->setObjectName(QString::fromUtf8("actionFilter"));
        actionGoToFolder = new QAction(MainWindow);
        actionGoToFolder->setObjectName(QString::fromUtf8("actionGoToFolder"));
        QIcon icon19;
        iconThemeName = QString::fromUtf8("folder-open");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon19 = QIcon::fromTheme(iconThemeName);
        } else {
            icon19.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionGoToFolder->setIcon(icon19);
        actionDownloads = new QAction(MainWindow);
        actionDownloads->setObjectName(QString::fromUtf8("actionDownloads"));
        actionDownloads->setVisible(true);
        actionUtilities = new QAction(MainWindow);
        actionUtilities->setObjectName(QString::fromUtf8("actionUtilities"));
        actionUtilities->setVisible(true);
        actionDocuments = new QAction(MainWindow);
        actionDocuments->setObjectName(QString::fromUtf8("actionDocuments"));
        actionDocuments->setVisible(true);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionDuplicate = new QAction(MainWindow);
        actionDuplicate->setObjectName(QString::fromUtf8("actionDuplicate"));
        actionEmptyTrash = new QAction(MainWindow);
        actionEmptyTrash->setObjectName(QString::fromUtf8("actionEmptyTrash"));
        actionShowContents = new QAction(MainWindow);
        actionShowContents->setObjectName(QString::fromUtf8("actionShowContents"));
        actionGoUpAndCloseCurrentWindow = new QAction(MainWindow);
        actionGoUpAndCloseCurrentWindow->setObjectName(QString::fromUtf8("actionGoUpAndCloseCurrentWindow"));
        actionGoUpAndCloseCurrentWindow->setIcon(icon1);
        actionOpenWith = new QAction(MainWindow);
        actionOpenWith->setObjectName(QString::fromUtf8("actionOpenWith"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        sidePane = new Fm::SidePane(splitter);
        sidePane->setObjectName(QString::fromUtf8("sidePane"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(sidePane->sizePolicy().hasHeightForWidth());
        sidePane->setSizePolicy(sizePolicy);
        splitter->addWidget(sidePane);
        frame = new QFrame(splitter);
        frame->setObjectName(QString::fromUtf8("frame"));
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(1, 0, 0, 0);
        tabBar = new Filer::TabBar(frame);
        tabBar->setObjectName(QString::fromUtf8("tabBar"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabBar->sizePolicy().hasHeightForWidth());
        tabBar->setSizePolicy(sizePolicy1);
        tabBar->setAcceptDrops(true);

        verticalLayout_2->addWidget(tabBar);

        stackedWidget = new QStackedWidget(frame);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy2);
        stackedWidget->setFrameShape(QFrame::NoFrame);

        verticalLayout_2->addWidget(stackedWidget);

        splitter->addWidget(frame);

        verticalLayout->addWidget(splitter);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 460, 22));
        menu_File = new QMenu(menubar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menuCreateNew = new QMenu(menu_File);
        menuCreateNew->setObjectName(QString::fromUtf8("menuCreateNew"));
        menu_Help = new QMenu(menubar);
        menu_Help->setObjectName(QString::fromUtf8("menu_Help"));
        menu_View = new QMenu(menubar);
        menu_View->setObjectName(QString::fromUtf8("menu_View"));
        menuSorting = new QMenu(menu_View);
        menuSorting->setObjectName(QString::fromUtf8("menuSorting"));
        menu_Editw = new QMenu(menubar);
        menu_Editw->setObjectName(QString::fromUtf8("menu_Editw"));
        menu_Bookmarks = new QMenu(menubar);
        menu_Bookmarks->setObjectName(QString::fromUtf8("menu_Bookmarks"));
        menu_Go = new QMenu(menubar);
        menu_Go->setObjectName(QString::fromUtf8("menu_Go"));
        menu_Tool = new QMenu(menubar);
        menu_Tool->setObjectName(QString::fromUtf8("menu_Tool"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(16, 16));
        toolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        menubar->addAction(menu_File->menuAction());
        menubar->addAction(menu_Editw->menuAction());
        menubar->addAction(menu_View->menuAction());
        menubar->addAction(menu_Go->menuAction());
        menubar->addAction(menu_Bookmarks->menuAction());
        menubar->addAction(menu_Tool->menuAction());
        menubar->addAction(menu_Help->menuAction());
        menu_File->addAction(actionNewTab);
        menu_File->addAction(actionOpen);
        menu_File->addAction(actionOpenWith);
        menu_File->addAction(actionShowContents);
        menu_File->addSeparator();
        menu_File->addAction(actionNewWin);
        menu_File->addSeparator();
        menu_File->addAction(menuCreateNew->menuAction());
        menu_File->addSeparator();
        menu_File->addAction(actionFileProperties);
        menu_File->addSeparator();
        menu_File->addAction(actionCloseWindow);
        menuCreateNew->addAction(actionNewFolder);
        menuCreateNew->addAction(actionNewBlankFile);
        menu_Help->addAction(actionAbout);
        menu_View->addAction(actionReload);
        menu_View->addAction(actionShowHidden);
        menu_View->addSeparator();
        menu_View->addAction(actionIconView);
        menu_View->addAction(actionDetailedList);
        menu_View->addSeparator();
        menu_View->addAction(menuSorting->menuAction());
        menu_View->addAction(actionFilter);
        menuSorting->addAction(actionByFileName);
        menuSorting->addAction(actionByMTime);
        menuSorting->addAction(actionByFileSize);
        menuSorting->addAction(actionByFileType);
        menuSorting->addAction(actionByOwner);
        menuSorting->addSeparator();
        menuSorting->addAction(actionAscending);
        menuSorting->addAction(actionDescending);
        menuSorting->addSeparator();
        menuSorting->addAction(actionFolderFirst);
        menuSorting->addAction(actionCaseSensitive);
        menu_Editw->addAction(actionCut);
        menu_Editw->addAction(actionCopy);
        menu_Editw->addAction(actionPaste);
        menu_Editw->addAction(actionDuplicate);
        menu_Editw->addAction(actionRename);
        menu_Editw->addAction(actionDelete);
        menu_Editw->addSeparator();
        menu_Editw->addAction(actionSelectAll);
        menu_Editw->addAction(actionInvertSelection);
        menu_Editw->addSeparator();
        menu_Editw->addAction(actionEmptyTrash);
        menu_Editw->addSeparator();
        menu_Editw->addAction(actionPreferences);
        menu_Bookmarks->addAction(actionAddToBookmarks);
        menu_Bookmarks->addAction(actionEditBookmarks);
        menu_Go->addAction(actionGoBack);
        menu_Go->addAction(actionGoForward);
        menu_Go->addAction(actionGoUp);
        menu_Go->addAction(actionGoUpAndCloseCurrentWindow);
        menu_Go->addSeparator();
        menu_Go->addAction(actionHome);
        menu_Go->addAction(actionDocuments);
        menu_Go->addAction(actionDesktop);
        menu_Go->addAction(actionDownloads);
        menu_Go->addAction(actionComputer);
        menu_Go->addAction(actionNetwork);
        menu_Go->addAction(actionApplications);
        menu_Go->addAction(actionUtilities);
        menu_Go->addAction(actionTrash);
        menu_Go->addSeparator();
        menu_Go->addAction(actionGoToFolder);
        menu_Tool->addAction(actionOpenTerminal);
        menu_Tool->addAction(actionOpenAsRoot);
        menu_Tool->addAction(actionFindFiles);
        toolBar->addAction(actionNewTab);
        toolBar->addAction(actionGoUp);
        toolBar->addAction(actionGo);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "File Manager", nullptr));
        actionGoUp->setText(QCoreApplication::translate("MainWindow", "Go &Up", nullptr));
#if QT_CONFIG(tooltip)
        actionGoUp->setToolTip(QCoreApplication::translate("MainWindow", "Go Up", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionGoUp->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Up", nullptr));
#endif // QT_CONFIG(shortcut)
        actionHome->setText(QCoreApplication::translate("MainWindow", "&Home", nullptr));
#if QT_CONFIG(shortcut)
        actionHome->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+H", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReload->setText(QCoreApplication::translate("MainWindow", "&Reload", nullptr));
#if QT_CONFIG(shortcut)
        actionReload->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGo->setText(QCoreApplication::translate("MainWindow", "Go", nullptr));
        actionQuit->setText(QCoreApplication::translate("MainWindow", "Quit", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "&About Filer", nullptr));
        actionNewWin->setText(QCoreApplication::translate("MainWindow", "&New Window", nullptr));
#if QT_CONFIG(tooltip)
        actionNewWin->setToolTip(QCoreApplication::translate("MainWindow", "New Window", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionNewWin->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionShowHidden->setText(QCoreApplication::translate("MainWindow", "Show &Hidden", nullptr));
        actionComputer->setText(QCoreApplication::translate("MainWindow", "&Computer", nullptr));
#if QT_CONFIG(shortcut)
        actionComputer->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionTrash->setText(QCoreApplication::translate("MainWindow", "&Trash", nullptr));
#if QT_CONFIG(shortcut)
        actionTrash->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNetwork->setText(QCoreApplication::translate("MainWindow", "&Network", nullptr));
#if QT_CONFIG(shortcut)
        actionNetwork->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDesktop->setText(QCoreApplication::translate("MainWindow", "&Desktop", nullptr));
#if QT_CONFIG(shortcut)
        actionDesktop->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAddToBookmarks->setText(QCoreApplication::translate("MainWindow", "&Add to Bookmarks", nullptr));
        actionApplications->setText(QCoreApplication::translate("MainWindow", "&Applications", nullptr));
#if QT_CONFIG(shortcut)
        actionApplications->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReload_2->setText(QCoreApplication::translate("MainWindow", "Reload", nullptr));
        actionIconView->setText(QCoreApplication::translate("MainWindow", "&Icon View", nullptr));
#if QT_CONFIG(shortcut)
        actionIconView->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDetailedList->setText(QCoreApplication::translate("MainWindow", "&Detailed List", nullptr));
#if QT_CONFIG(shortcut)
        actionDetailedList->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionThumbnailView->setText(QCoreApplication::translate("MainWindow", "&Thumbnail View", nullptr));
        actionCut->setText(QCoreApplication::translate("MainWindow", "Cu&t", nullptr));
#if QT_CONFIG(shortcut)
        actionCut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCopy->setText(QCoreApplication::translate("MainWindow", "&Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionCopy->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPaste->setText(QCoreApplication::translate("MainWindow", "&Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionPaste->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSelectAll->setText(QCoreApplication::translate("MainWindow", "Select &All", nullptr));
#if QT_CONFIG(shortcut)
        actionSelectAll->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPreferences->setText(QCoreApplication::translate("MainWindow", "Pr&eferences", nullptr));
#if QT_CONFIG(shortcut)
        actionPreferences->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+,", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAscending->setText(QCoreApplication::translate("MainWindow", "&Ascending", nullptr));
        actionDescending->setText(QCoreApplication::translate("MainWindow", "&Descending", nullptr));
        actionByFileName->setText(QCoreApplication::translate("MainWindow", "&By File Name", nullptr));
        actionByMTime->setText(QCoreApplication::translate("MainWindow", "By &Modification Time", nullptr));
        actionByFileType->setText(QCoreApplication::translate("MainWindow", "By File &Type", nullptr));
        actionByOwner->setText(QCoreApplication::translate("MainWindow", "By &Owner", nullptr));
        actionFolderFirst->setText(QCoreApplication::translate("MainWindow", "&Folder First", nullptr));
        actionNewTab->setText(QCoreApplication::translate("MainWindow", "New &Tab", nullptr));
#if QT_CONFIG(tooltip)
        actionNewTab->setToolTip(QCoreApplication::translate("MainWindow", "New Tab", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionNewTab->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGoBack->setText(QCoreApplication::translate("MainWindow", "Go &Back", nullptr));
#if QT_CONFIG(tooltip)
        actionGoBack->setToolTip(QCoreApplication::translate("MainWindow", "Go Back", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionGoBack->setShortcut(QCoreApplication::translate("MainWindow", "Alt+Left", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGoForward->setText(QCoreApplication::translate("MainWindow", "Go &Forward", nullptr));
#if QT_CONFIG(tooltip)
        actionGoForward->setToolTip(QCoreApplication::translate("MainWindow", "Go Forward", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionGoForward->setShortcut(QCoreApplication::translate("MainWindow", "Alt+Right", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInvertSelection->setText(QCoreApplication::translate("MainWindow", "&Invert Selection", nullptr));
        actionDelete->setText(QCoreApplication::translate("MainWindow", "&Move to Trash", nullptr));
#if QT_CONFIG(shortcut)
        actionDelete->setShortcut(QCoreApplication::translate("MainWindow", "Meta+Backspace", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRename->setText(QCoreApplication::translate("MainWindow", "&Rename", nullptr));
#if QT_CONFIG(shortcut)
        actionRename->setShortcut(QCoreApplication::translate("MainWindow", "F2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCloseTab->setText(QCoreApplication::translate("MainWindow", "C&lose Tab", nullptr));
        actionFileProperties->setText(QCoreApplication::translate("MainWindow", "Get &Info", nullptr));
#if QT_CONFIG(shortcut)
        actionFileProperties->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+I", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCaseSensitive->setText(QCoreApplication::translate("MainWindow", "&Case Sensitive", nullptr));
        actionByFileSize->setText(QCoreApplication::translate("MainWindow", "By File &Size", nullptr));
        actionCloseWindow->setText(QCoreApplication::translate("MainWindow", "&Close Window", nullptr));
#if QT_CONFIG(shortcut)
        actionCloseWindow->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEdit_Bookmarks->setText(QCoreApplication::translate("MainWindow", "Edit Bookmarks", nullptr));
        actionOpenTerminal->setText(QCoreApplication::translate("MainWindow", "Open &Terminal", nullptr));
#if QT_CONFIG(shortcut)
        actionOpenTerminal->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpenAsRoot->setText(QCoreApplication::translate("MainWindow", "Open as &Root", nullptr));
        actionEditBookmarks->setText(QCoreApplication::translate("MainWindow", "&Edit Bookmarks", nullptr));
        actionNewFolder->setText(QCoreApplication::translate("MainWindow", "&Folder", nullptr));
#if QT_CONFIG(shortcut)
        actionNewFolder->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNewBlankFile->setText(QCoreApplication::translate("MainWindow", "&Blank File", nullptr));
#if QT_CONFIG(shortcut)
        actionNewBlankFile->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFindFiles->setText(QCoreApplication::translate("MainWindow", "&Find Files", nullptr));
#if QT_CONFIG(shortcut)
        actionFindFiles->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFilter->setText(QCoreApplication::translate("MainWindow", "Filter", nullptr));
        actionGoToFolder->setText(QCoreApplication::translate("MainWindow", "&Go To Folder", nullptr));
#if QT_CONFIG(tooltip)
        actionGoToFolder->setToolTip(QCoreApplication::translate("MainWindow", "Go To Folder", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionGoToFolder->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+G", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDownloads->setText(QCoreApplication::translate("MainWindow", "&Downloads", nullptr));
#if QT_CONFIG(tooltip)
        actionDownloads->setToolTip(QCoreApplication::translate("MainWindow", "Downloads", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDownloads->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUtilities->setText(QCoreApplication::translate("MainWindow", "&Utilities", nullptr));
#if QT_CONFIG(tooltip)
        actionUtilities->setToolTip(QCoreApplication::translate("MainWindow", "Utilities", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionUtilities->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+U", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDocuments->setText(QCoreApplication::translate("MainWindow", "&Documents", nullptr));
#if QT_CONFIG(tooltip)
        actionDocuments->setToolTip(QCoreApplication::translate("MainWindow", "Documents", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDocuments->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDuplicate->setText(QCoreApplication::translate("MainWindow", "&Duplicate", nullptr));
#if QT_CONFIG(shortcut)
        actionDuplicate->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEmptyTrash->setText(QCoreApplication::translate("MainWindow", "Empty Trash", nullptr));
#if QT_CONFIG(shortcut)
        actionEmptyTrash->setShortcut(QCoreApplication::translate("MainWindow", "Meta+Shift+Backspace", nullptr));
#endif // QT_CONFIG(shortcut)
        actionShowContents->setText(QCoreApplication::translate("MainWindow", "Show Contents", nullptr));
#if QT_CONFIG(shortcut)
        actionShowContents->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+Shift+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGoUpAndCloseCurrentWindow->setText(QCoreApplication::translate("MainWindow", "Go Up and Close Current", nullptr));
#if QT_CONFIG(tooltip)
        actionGoUpAndCloseCurrentWindow->setToolTip(QCoreApplication::translate("MainWindow", "Go Up and Close Current", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionGoUpAndCloseCurrentWindow->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+Up", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpenWith->setText(QCoreApplication::translate("MainWindow", "Open With...", nullptr));
#if QT_CONFIG(shortcut)
        actionOpenWith->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+O", nullptr));
#endif // QT_CONFIG(shortcut)
        menu_File->setTitle(QCoreApplication::translate("MainWindow", "&File", nullptr));
        menuCreateNew->setTitle(QCoreApplication::translate("MainWindow", "C&reate New", nullptr));
        menu_Help->setTitle(QCoreApplication::translate("MainWindow", "&Help", nullptr));
        menu_View->setTitle(QCoreApplication::translate("MainWindow", "&View", nullptr));
        menuSorting->setTitle(QCoreApplication::translate("MainWindow", "&Sorting", nullptr));
        menu_Editw->setTitle(QCoreApplication::translate("MainWindow", "&Edit", nullptr));
        menu_Bookmarks->setTitle(QCoreApplication::translate("MainWindow", "&Bookmarks", nullptr));
        menu_Go->setTitle(QCoreApplication::translate("MainWindow", "&Go", nullptr));
        menu_Tool->setTitle(QCoreApplication::translate("MainWindow", "&Tool", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "Main Toolbar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_2D_WIN_H
