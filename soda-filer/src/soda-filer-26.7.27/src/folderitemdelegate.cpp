/*

    Copyright (C) 2013  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "folderitemdelegate.h"
#include "foldermodel.h"
#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QIcon>
#include <QTextLayout>
#include <QTextOption>
#include <QTextLine>
#include <QDebug>
#include <QPainterPath>
#include <QColor>
#include <QFileInfo>
#include <QProcess>
#include <QDateTime>
#include <QDir>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include "bundle.h"
#include "application.h"

# include "extattrs.h"
#include "tags.h"

namespace Fm {

FolderItemDelegate::FolderItemDelegate(QAbstractItemView* view, QObject* parent):
  QStyledItemDelegate(parent ? parent : view),
  symlinkIcon_(QIcon::fromTheme("emblem-symbolic-link")),
  view_(view) {
}

FolderItemDelegate::~FolderItemDelegate() {
}

QSize FolderItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QVariant value = index.data(Qt::SizeHintRole);
  if(value.isValid())
    return qvariant_cast<QSize>(value);
  if(option.decorationPosition == QStyleOptionViewItem::Top ||
    option.decorationPosition == QStyleOptionViewItem::Bottom) {

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.decorationAlignment = Qt::AlignHCenter|Qt::AlignTop;
    opt.displayAlignment = Qt::AlignTop|Qt::AlignHCenter;

    // FIXME: there're some problems in this size hint calculation.
    // Q_ASSERT(gridSize_ != QSize()); // probono: Commented this out since it can cause crashes
    int horizontalPadding = gridPaddingLeft_ + gridPaddingRight_;
    int verticalPadding = gridPaddingTop_ + gridPaddingBottom_;
    QRectF textRect(0, 0, gridSize_.width() - horizontalPadding, gridSize_.height() - opt.decorationSize.height() - verticalPadding);
    void* file = index.data(FolderModel::FileInfoRole).value<void*>();
    drawText(NULL, opt, itemCountText(file)); // passing NULL for painter will calculate the bounding rect only.
    int width = qMax((int)textRect.width(), opt.decorationSize.width()) + horizontalPadding;
    int height = opt.decorationSize.height() + textRect.height() + verticalPadding;
    return QSize(width, height);
  }
  return QStyledItemDelegate::sizeHint(option, index);
}

// Finder-style "N items" subtext shown under each directory's name in icon
// view. Only meaningful for directories; a plain QDir::count() is a
// synchronous directory listing (same cost class as the git-status QProcess
// call already done per-repaint just below in paint() for .git folders), so
// it's cached briefly rather than re-listed on every single repaint.
QString FolderItemDelegate::itemCountText(void* filePtr) const {
  if(static_cast<Filer::Application*>(qApp)->settings().showFolderItemCount() != 1)
    return QString();
  FmFileInfo* file = static_cast<FmFileInfo*>(filePtr);
  if(!file || !fm_file_info_is_dir(file))
    return QString();
  QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(file)));
  // Invalidate on the directory's mtime, not a short timer: adding/removing
  // an entry updates the dir's mtime, so this stays correct while costing
  // one stat() per repaint instead of a full directory listing every few
  // seconds (which made scrolling folders-with-many-subfolders visibly lag).
  qint64 mtime = QFileInfo(path).lastModified().toMSecsSinceEpoch();
  auto it = itemCountCache_.constFind(path);
  if(it != itemCountCache_.constEnd() && it.value().second == mtime)
    return it.value().first;
  QDir dir(path);
  int count = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden).size();
  QString text = count == 0 ? tr("Empty") : (count == 1 ? tr("1 item") : tr("%1 items").arg(count));
  itemCountCache_[path] = qMakePair(text, mtime);
  return text;
}

QIcon FolderItemDelegate::taggedFolderIcon(const QIcon& fallback, FmFileInfo* file, const QString& tagName) {
  if(tagName.isEmpty() || !file || !fm_file_info_is_dir(file))
    return fallback;
  QString lower = tagName.toLower();
  QStringList candidates;
  candidates << (QStringLiteral("folder-") + lower);
  if(lower == QLatin1String("gray"))
    candidates << QStringLiteral("folder-grey"); // some icon themes (e.g. Papirus) spell it "grey"
  for(const QString& name : candidates) {
    if(QIcon::hasThemeIcon(name))
      return QIcon::fromTheme(name);
  }
  return fallback;
}

QIcon::Mode FolderItemDelegate::iconModeFromState(QStyle::State state) {
  QIcon::Mode iconMode;
  if(state & QStyle::State_Enabled) {
    if(state & QStyle::State_Selected)
      iconMode = QIcon::Selected;
    else {
      iconMode = QIcon::Normal;
    }
  }
  else
    iconMode = QIcon::Disabled;
  return iconMode;
}

// special thanks to Razor-qt developer Alec Moskvin(amoskvin) for providing the fix!
void FolderItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  Q_ASSERT(index.isValid());
  FmFileInfo* file = static_cast<FmFileInfo*>(index.data(FolderModel::FileInfoRole).value<void*>());
  bool isSymlink = file && fm_file_info_is_symlink(file);

  // Clean Up/Clean Up By slide animation (same mechanism as
  // DesktopItemDelegate): translate the whole cell while a move is in
  // flight. The clip rect must not be applied in that case, or the item
  // would be cut off at its (stationary) layout rect while sliding.
  QPoint animOffset = animationOffsets_.value(QPersistentModelIndex(index), QPoint(0, 0));

  if(option.decorationPosition == QStyleOptionViewItem::Top ||
    option.decorationPosition == QStyleOptionViewItem::Bottom) {
    painter->save();
    if(animOffset.isNull())
      painter->setClipRect(option.rect);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if(!animOffset.isNull())
      opt.rect.translate(animOffset);
    opt.decorationAlignment = Qt::AlignHCenter|Qt::AlignTop;
    opt.displayAlignment = Qt::AlignTop|Qt::AlignHCenter;

    QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(file))); // probono: rgh! So complicated! TODO: Get rid of libfm

    // If the trash is full, use a different icon for the trash can
    if(QFileInfo(path).fileName() == "trash-can.desktop") {

    }

    // Tag lookup, done once up front: a tagged directory gets a color-named
    // themed folder icon (e.g. "folder-red") instead of the plain folder
    // icon, and the tag's color dot is drawn before the name in the label
    // below (see drawText()) rather than on the icon itself.
    bool tagOk = false;
    QString tagName = Fm::getFileTag(path, tagOk);
    QColor tagColor = tagOk ? Fm::tagColorForName(tagName) : QColor();
    if(tagOk)
      opt.icon = taggedFolderIcon(opt.icon, file, tagName);

    // draw the icon
    QIcon::Mode iconMode = iconModeFromState(opt.state);
    QPoint iconPos(opt.rect.x() + (opt.rect.width() - opt.decorationSize.width()) / 2, opt.rect.y());

    // probono: If the icon is selected, draw a light rounded rect in the background
    if(opt.state & QStyle::State_Selected) {
      QRectF boundRect = QRectF();
      boundRect.setTop(iconPos.y());
      boundRect.setLeft(iconPos.x());
      boundRect.setHeight(opt.decorationSize.height());
      boundRect.setWidth(opt.decorationSize.width());
      painter->setRenderHint(QPainter::Antialiasing);
      QPainterPath path = QPainterPath();
      path.addRoundedRect(boundRect, 4, 4);
      painter->fillPath(path, QColor(196, 196, 196)); // Light gray
    }

    QPixmap pixmap = opt.icon.pixmap(opt.decorationSize, iconMode);
    painter->drawPixmap(iconPos, pixmap);

    // If this is a .app/.AppDir bundle with no executable that can actually
    // run on this platform (e.g. a macOS .app shipping only a Mach-O
    // binary), mark it clearly instead of letting it look like any other
    // launchable app. Same check/overlay as DesktopItemDelegate.
    if(file && fm_file_info_is_dir(file)) {
      static const qint64 kNotLaunchableCacheMs = 5000;
      qint64 nowNl = QDateTime::currentMSecsSinceEpoch();
      bool notLaunchable = false;
      auto nlIt = notLaunchableCache_.constFind(path);
      if(nlIt != notLaunchableCache_.constEnd() && (nowNl - nlIt.value().second) < kNotLaunchableCacheMs) {
        notLaunchable = nlIt.value().first;
      } else {
        notLaunchable = Fm::isAppBundleNotLaunchable(file);
        notLaunchableCache_[path] = qMakePair(notLaunchable, nowNl);
      }
      if(notLaunchable) {
        QIcon forbiddenIcon(":/icons/forbidden.png");
        QPixmap forbiddenPix = forbiddenIcon.pixmap(opt.decorationSize * 0.55);
        QPoint forbiddenPos = iconPos;
        forbiddenPos.setX(forbiddenPos.x() + (opt.decorationSize.width() - forbiddenPix.width()) / 2);
        forbiddenPos.setY(forbiddenPos.y() + (opt.decorationSize.height() - forbiddenPix.height()) / 2);
        painter->setOpacity(0.85);
        painter->drawPixmap(forbiddenPos, forbiddenPix);
        painter->setOpacity(1);
      }
    }

    QPoint symlinkPos = iconPos;
    symlinkPos.setY(symlinkPos.y() + opt.decorationSize.height() / 2);

    // draw some emblems for the item if needed
    // we only support symlink emblem at the moment
    if(isSymlink)
      painter->drawPixmap(symlinkPos, symlinkIcon_.pixmap(opt.decorationSize / 2, iconMode));

    // probono: git labels
    // FIXME: Why doesn't this work?
    // Fm::drawGitEmblem(path, painter, iconPos, opt, file ,iconMode);

    // ======================================= Start ugly code duplication

    // probono: git labels
    QString mimetype = QString::fromUtf8(fm_mime_type_get_type(fm_mime_type_ref(fm_file_info_get_mime_type(file))));
    if (mimetype == "inode/directory" && QFileInfo(path + "/.git").exists()) {
        // Check git status -- CACHED. This used to spawn a synchronous
        // `git status` subprocess (fork+exec+wait) on EVERY repaint of every
        // directory containing .git; in a folder full of repositories that
        // made scrolling unusably slow, since paint() runs continuously
        // while scrolling.
        static const qint64 kGitStatusCacheMs = 15000;
        qint64 nowGit = QDateTime::currentMSecsSinceEpoch();
        bool gitHasChanges = false;
        auto gitIt = gitStatusCache_.constFind(path);
        if(gitIt != gitStatusCache_.constEnd() && (nowGit - gitIt.value().second) < kGitStatusCacheMs) {
            gitHasChanges = gitIt.value().first;
        } else {
            QProcess p;
            QProcessEnvironment env;
            env.insert("LANG", "C");
            p.setProcessEnvironment(env);
            p.setProgram("git");
            p.setWorkingDirectory(path);
            p.setArguments({"status", "-s"});
            p.start();
            p.waitForFinished();
            gitHasChanges = p.readAllStandardOutput().length();
            gitStatusCache_[path] = qMakePair(gitHasChanges, nowGit);
        }

        // "emblem-symbolic-git" isn't a standard freedesktop icon name and
        // isn't bundled with the app, so it's missing from most icon themes;
        // QIcon::fromTheme() would then resolve to the theme's generic
        // "unknown" icon (a question mark) instead of drawing nothing.
        if(QIcon::hasThemeIcon("emblem-symbolic-git")) {
            QIcon emblemIcon = QIcon::fromTheme("emblem-symbolic-git");
            QPoint emblemPos = iconPos;
            emblemPos.setX(emblemPos.x() + opt.decorationSize.width()/4);
            emblemPos.setY(emblemPos.y() + opt.decorationSize.height()/2.6);
            if(gitHasChanges) {
                 painter->setOpacity(0.5);
            } else {
                painter->setOpacity(0.1);
            }
            painter->drawPixmap(emblemPos, emblemIcon.pixmap(opt.decorationSize / 2, iconMode));
            painter->setOpacity(1);
        }
    }

    // ======================================= End ugly code duplication

    // probono: Draw label emblems
    bool ok;
    QString emblem = Fm::getAttributeValueQString(path, "EMBLEM", ok);
    if (ok) {
        QIcon emblemIcon = QIcon::fromTheme(emblem);
        QPoint emblemIconPos(opt.decorationSize.width() * 0.75 + opt.rect.x() + (opt.rect.width() - opt.decorationSize.width()) / 2, opt.rect.y());
        painter->drawPixmap(emblemIconPos, emblemIcon.pixmap(opt.decorationSize / 2, iconMode));
    }

    // While this item is being renamed, the transparent QLineEdit editor
    // (see createEditor()) sits exactly where the label is drawn -- same
    // trick as DesktopItemDelegate. Without this check, the static label
    // text kept being repainted underneath the editor on every repaint,
    // looking like a second text field next to the one being typed into.
    if(view_ && view_->indexWidget(index) != nullptr) {
      painter->restore();
      return;
    }

    // draw the text
    drawText(painter, opt, itemCountText(file), tagColor);
    painter->restore();
  }
  else {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QIcon::Mode iconMode = iconModeFromState(opt.state);

    QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(file))); // argh! So complicated!

    // Tag lookup, done before painting: a tagged directory gets a
    // color-named themed icon (e.g. "folder-red") swapped into `opt.icon`
    // below, and the tag's color dot is drawn right before the name label
    // instead of on the icon itself.
    bool tagOk = false;
    QString tagName = Fm::getFileTag(path, tagOk);
    QColor tagColor = tagOk ? Fm::tagColorForName(tagName) : QColor();
    if(tagOk)
      opt.icon = taggedFolderIcon(opt.icon, file, tagName);

    // Can't call QStyledItemDelegate::paint() here since it re-reads the
    // icon straight from the model, ignoring the tag-swapped opt.icon set
    // above -- drawControl() is what QStyledItemDelegate::paint() calls
    // internally anyway, just fed our locally modified style option.
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    // draw emblems if needed
    if(isSymlink) {
      QPoint iconPos(opt.rect.x(), opt.rect.y() + (opt.rect.height() - opt.decorationSize.height()) / 2);
      // draw some emblems for the item if needed
      // we only support symlink emblem at the moment
      QPoint symlinkPos = iconPos;
      symlinkPos.setY(symlinkPos.y() + opt.decorationSize.height() / 2);
      painter->drawPixmap(symlinkPos, symlinkIcon_.pixmap(opt.decorationSize / 2, iconMode));
    }

    // probono: Draw label emblems
    bool ok;
    QString emblem = Fm::getAttributeValueQString(path, "EMBLEM", ok);
    if (ok) {
        QIcon emblemIcon = QIcon::fromTheme(emblem);
        QPoint emblemIconPos(opt.decorationSize.width() * 0.75 + opt.rect.x() + (opt.rect.width() - opt.decorationSize.width()) / 2, opt.rect.y());
        painter->drawPixmap(emblemIconPos, emblemIcon.pixmap(opt.decorationSize / 2, iconMode));
    }

    // Finder-style tag swatch: a small colored dot right before the name,
    // in the label itself rather than on the icon.
    if(tagColor.isValid()) {
      QRect textRect = opt.widget ? opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget) : QRect();
      if(textRect.isNull())
        textRect = opt.rect;
      qreal dotSize = 6;
      qreal dotGap = 4;
      QRectF dot(textRect.x() - dotSize - dotGap,
                 textRect.y() + (textRect.height() - dotSize) / 2.0,
                 dotSize, dotSize);
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setBrush(tagColor);
      painter->setPen(Qt::NoPen);
      painter->drawEllipse(dot);
      painter->restore();
    }
  }
}

void FolderItemDelegate::drawText(QPainter* painter, QStyleOptionViewItem& opt, const QString& subtext, const QColor& tagColor) const {

    // Settings::iconViewTextSize()/iconViewTextWeight() (0 means "leave
    // opt.font's own size/weight alone") and Settings::iconViewTextAnchor() --
    // applied to opt.font itself (rather than a local copy) since opt is
    // used for the QTextLayout below, the "N items" subtext font derived
    // from it further down, and the editor geometry in updateEditorGeometry()
    // all keying off the same font.
    if(textSize_ > 0)
      opt.font.setPointSize(textSize_);
    if(textWeight_ > 0)
      opt.font.setWeight(QFont::Weight(textWeight_));
    Qt::Alignment horizontalAnchor = Qt::AlignHCenter;
    if(textAnchor_ == QLatin1String("left"))
      horizontalAnchor = Qt::AlignLeft;
    else if(textAnchor_ == QLatin1String("right"))
      horizontalAnchor = Qt::AlignRight;

    // Calculate size of textRect
    int leftAndRightSpace = 20;
    QRectF textRect(opt.rect.x() + leftAndRightSpace,
                    opt.rect.y() + 3 + opt.decorationSize.height() + textMarginTop_,
                    opt.rect.width() - leftAndRightSpace * 2,
                    opt.rect.height() - opt.decorationSize.height() - textMarginTop_ - textMarginBottom_);
    QTextLayout layout(opt.text, opt.font);

    QTextOption textOption;
    textOption.setAlignment(horizontalAnchor | (opt.displayAlignment & ~Qt::AlignHorizontal_Mask));
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textOption.setTextDirection(opt.direction);
    layout.setTextOption(textOption);
    qreal height = 0;
    qreal width = 0;
    int visibleLines = 0;
    layout.beginLayout();
    QString elidedText;

  for(;;) {
    QTextLine line = layout.createLine();
    if(!line.isValid())
      break;
    line.setLineWidth(textRect.width());
    QFontMetrics fontMetrics(opt.font);
    height += fontMetrics.leading();
    line.setPosition(QPointF(0, height));
    if((height + line.height() + textRect.y()) + 3 > textRect.bottom()) {
        // if part of this line falls outside the textRect, ignore it and quit.
        QTextLine lastLine = layout.lineAt(visibleLines - 1);
        elidedText = opt.text.mid(lastLine.textStart());
        opt.textElideMode = Qt::ElideMiddle; // probono: Put ... in the middle, not at the end so that we can see the suffix
        elidedText = fontMetrics.elidedText(elidedText, opt.textElideMode, textRect.width());
        break;
    }
    height += line.height();
    ++ visibleLines;
    width = qMax(width, line.naturalTextWidth());
  }
  layout.endLayout();

  // Finder-style "N items" subtext: a second, smaller line below the name,
  // in the accent/link color. Its own font/metrics, tracked separately from
  // the name's layout above so the two can differ in size and color.
  QFont subtextFont(opt.font);
  subtextFont.setPointSizeF(qMax(subtextFont.pointSizeF() - 2.0, 7.0));
  QFontMetrics subtextMetrics(subtextFont);
  qreal subtextHeight = subtext.isEmpty() ? 0 : (subtextMetrics.height() + 2);
  qreal subtextWidth = subtext.isEmpty() ? 0 : subtextMetrics.horizontalAdvance(subtext);
  width = qMax(width, subtextWidth);

  // probono: draw background rounded rect for selected item
  QRectF boundRect = layout.boundingRect();
  int additionalSpace = 1;
  boundRect.setWidth(width + 16 * additionalSpace);

  // boundRect.setHeight(boundRect.height() + 2 * additionalSpace); // probono: This was wrong, resulting in some background rounded rects being too high
  boundRect.setHeight(height + subtextHeight + additionalSpace);
  boundRect.moveTo(textRect.x() - 8*additionalSpace + (textRect.width() - width)/2,
                   textRect.y() - additionalSpace);

  if(!painter) { // no painter, calculate the bounding rect only
    textRect = boundRect;
    return;
  }

  QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
  if(opt.state & QStyle::State_Selected) {
    // painter->fillRect(boundRect, opt.palette.highlight());
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath path = QPainterPath();
    path.addRoundedRect(boundRect, 8, 8);
    painter->fillPath(path, opt.palette.highlight());
    painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
  }
  else
    painter->setPen(opt.palette.color(cg, QPalette::Text));

  // Finder-style tag swatch: a small colored dot right before the name,
  // positioned against the first line's actual (centered) text start
  // rather than the fixed textRect edge, so it reads as part of the name.
  if(tagColor.isValid() && visibleLines > 0) {
    QTextLine firstLine = layout.lineAt(0);
    qreal firstLineStartX = textRect.x() + (textRect.width() - firstLine.naturalTextWidth()) / 2.0;
    qreal dotSize = 6;
    qreal dotGap = 4;
    QRectF dot(firstLineStartX - dotSize - dotGap,
               textRect.y() + firstLine.y() + (firstLine.height() - dotSize) / 2.0,
               dotSize, dotSize);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(tagColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(dot);
    painter->restore();
  }

  // draw text
  for(int i = 0; i < visibleLines; ++i) {
    QTextLine line = layout.lineAt(i);
    if(i == (visibleLines - 1) && !elidedText.isEmpty()) { // the last line, draw elided text
      QPointF pos(textRect.x() + line.position().x(), textRect.y() + line.y() + line.ascent());
      painter->drawText(pos, elidedText);
    }
    else {
      line.draw(painter, textRect.topLeft());
    }
  }

  // draw the "N items" subtext, centered, right below the name
  if(!subtext.isEmpty()) {
    painter->save();
    painter->setFont(subtextFont);
    if(!(opt.state & QStyle::State_Selected))
      // Follow the system accent color (QPalette::Highlight) instead of a
      // fixed pearOS blue, so this subtext re-tints when the user changes
      // the system accent color.
      painter->setPen(opt.palette.color(cg, QPalette::Highlight));
    QPointF pos(textRect.x() + (textRect.width() - subtextWidth) / 2,
                textRect.y() + height + subtextMetrics.ascent());
    painter->drawText(pos, subtext);
    painter->restore();
  }

  /* probono: We don't want FocusRects
  if(opt.state & QStyle::State_HasFocus) {
    // draw focus rect
    QStyleOptionFocusRect o;
    o.QStyleOption::operator=(opt);
    o.rect = boundRect.toRect(); // subElementRect(SE_ItemViewItemFocusRect, vopt, widget);
    o.state |= QStyle::State_KeyboardFocusChange;
    o.state |= QStyle::State_Item;
    QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                  ? QPalette::Normal : QPalette::Disabled;
    o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                  ? QPalette::Highlight : QPalette::Window);
    if (const QWidget* widget = opt.widget) {
      QStyle* style = widget->style() ? widget->style() : qApp->style();
      style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, widget);
    }
  }
  */

}

QWidget* FolderItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
  // Same trick as DesktopItemDelegate::createEditor(): a transparent,
  // borderless QLineEdit matching the static label's font, positioned by
  // updateEditorGeometry() to sit exactly over the label, so it reads as
  // "the label itself became editable" instead of a plain white box
  // dropped on top of the icon+label cell (the default QStyledItemDelegate
  // behavior).
  QLineEdit* editor = new QLineEdit(parent);
  editor->setFrame(false);
  editor->setAlignment(Qt::AlignHCenter);
  editor->setAttribute(Qt::WA_TranslucentBackground);
  editor->setStyleSheet(
      "QLineEdit {"
      "  background: transparent;"
      "  border: none;"
      "  padding: 0px;"
      "}"
  );
  return editor;
}

void FolderItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
  QStyledItemDelegate::setEditorData(editor, index);
  if(QLineEdit* le = qobject_cast<QLineEdit*>(editor))
    le->selectAll();
}

void FolderItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  if(opt.decorationPosition == QStyleOptionViewItem::Top ||
     opt.decorationPosition == QStyleOptionViewItem::Bottom) {
    // Must match drawText()'s textRect exactly (leftAndRightSpace band
    // below the icon), or the editable box lands in the wrong place / size
    // relative to the label it's supposed to replace.
    editor->setFont(opt.font);
    int leftAndRightSpace = 20;
    QRect labelRect(opt.rect.x() + leftAndRightSpace,
                     opt.rect.y() + 3 + opt.decorationSize.height(),
                     opt.rect.width() - leftAndRightSpace * 2,
                     opt.rect.height() - opt.decorationSize.height());
    editor->setGeometry(labelRect);
  }
  else {
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
  }
}

} // namespace Fm
