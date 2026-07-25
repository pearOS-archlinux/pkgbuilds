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


#include "desktopitemdelegate.h"
#include "foldermodel.h"
#include <QApplication>
#include <QListView>
#include <QPainter>
#include <QIcon>
#include <QTextLayout>
#include <QTextOption>
#include <QTextLine>
#include <QPainterPath>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QStorageInfo>
#include <QDateTime>
#include <QVector>
#include <QLineEdit>
#include <QFontMetrics>

#include "extattrs.h"
#include "itemdelegatefunctions.h"
#include "tags.h"
#include "bundle.h"

using namespace Filer;

namespace {
  const int kMaxTextLines = 2;
  const qreal kLineHeight = 16.0; // fixed line height for desktop icon labels
  // Approximates the CSS spec "0px 1px 2px rgba(0,0,0,0.5)" (0 horizontal /
  // 1px vertical offset, 2px blur, 50% peak opacity): QPainter has no text
  // blur primitive, so this stamps the shadow glyph at several offsets in a
  // small kernel centered on (0,1), each at low alpha, so the overlapping
  // draws build up to roughly 0.5 alpha at the core and fall off by ~2px --
  // a soft halo rather than the hard-edged single-offset copy this used to be.
  struct ShadowSample { QPoint offset; qreal alpha; };
  const ShadowSample kShadowKernel[] = {
    { QPoint(0, 1), 0.30 },
    { QPoint(-1, 1), 0.14 }, { QPoint(1, 1), 0.14 },
    { QPoint(0, 0), 0.14 }, { QPoint(0, 2), 0.14 },
    { QPoint(-1, 0), 0.07 }, { QPoint(1, 0), 0.07 },
    { QPoint(-1, 2), 0.07 }, { QPoint(1, 2), 0.07 },
  };
}

DesktopItemDelegate::DesktopItemDelegate(QListView* view, QObject* parent):
  QStyledItemDelegate(parent ? parent : view),
  view_(view),
  symlinkIcon_(QIcon::fromTheme("emblem-symbolic-link")),
  shadowColor_(0, 0, 0) {
}

// FIXME: we need to figure out a way to derive from Fm::FolderItemDelegate to avoid code duplication.
void DesktopItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  Q_ASSERT(index.isValid());
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QPoint offset = animationOffsets_.value(QPersistentModelIndex(index), QPoint(0,0));
  if (!offset.isNull()) {
      opt.rect.translate(offset);
  }
  /*
  // Draw shadow
  // qDebug() << "index:" << index;
  QPen origPen = painter->pen();
  QRect shadowRect(QPoint(0, 0), QSize(1600, 33));
  QLinearGradient linearGradient(0,0,0,33);
  linearGradient.setColorAt(0.00, QColor::fromRgbF(0, 0, 0, 0.3));
  linearGradient.setColorAt(0.33, QColor::fromRgbF(0, 0, 0, 0.2));
  linearGradient.setColorAt(1.00, QColor::fromRgbF(0, 0, 0, 0.0));
  painter->fillRect(shadowRect,  linearGradient);
  painter->setPen(origPen);
  painter->save();
  painter->setClipRect(option.rect);
  */

  opt.decorationAlignment = Qt::AlignHCenter | Qt::AlignTop;
  opt.displayAlignment = Qt::AlignTop | Qt::AlignHCenter;

  // draw the icon
  QIcon::Mode iconMode;
  if(opt.state & QStyle::State_Enabled) {
    if(opt.state & QStyle::State_Selected)
      iconMode = QIcon::Selected;
    else {
      iconMode = QIcon::Normal;
    }
  }
  else
    iconMode = QIcon::Disabled;
  
  QPoint iconPos;
  if(labelRight_) {
      opt.decorationAlignment = Qt::AlignVCenter | Qt::AlignLeft;
      opt.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;
      iconPos = QPoint(opt.rect.x() + gridSpacing_ / 2, opt.rect.y() + (opt.rect.height() - opt.decorationSize.height()) / 2);
  } else {
      opt.decorationAlignment = Qt::AlignHCenter | Qt::AlignTop;
      opt.displayAlignment = Qt::AlignTop | Qt::AlignHCenter;
      iconPos = QPoint(opt.rect.x() + (opt.rect.width() - opt.decorationSize.width()) / 2, opt.rect.y());
  }

  // If the icon is selected, draw a light rounded outline around it,
  // expanded slightly beyond the icon's raw pixels (macOS-style selection halo).
  if(opt.state & QStyle::State_Selected) {
    QRectF boundRect;
    boundRect.setTop(iconPos.y());
    boundRect.setLeft(iconPos.x());
    boundRect.setHeight(opt.decorationSize.height());
    boundRect.setWidth(opt.decorationSize.width());
    boundRect.adjust(-6, -6, 6, 6);
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath bgPath = QPainterPath();
    bgPath.addRoundedRect(boundRect, 4, 4);
    painter->setPen(QPen(QColor::fromRgbF(196 / 255.0, 196 / 255.0, 196 / 255.0, 0.9), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(bgPath);
  }

  FmFileInfo* file = static_cast<FmFileInfo*>(index.data(Fm::FolderModel::FileInfoRole).value<void*>());
  QString path;
  if(file) {
      path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(file))); // argh! So complicated!
      QString iconCacheKey = path + "_" + QString::number(iconMode) + "_" + QString::number(opt.decorationSize.width());
      QPixmap pixmap;
      auto iconIt = iconPixmapCache_.constFind(iconCacheKey);
      if (iconIt != iconPixmapCache_.constEnd()) {
          pixmap = iconIt.value();
      } else {
          pixmap = opt.icon.pixmap(opt.decorationSize, iconMode);
          iconPixmapCache_[iconCacheKey] = pixmap;
      }
      painter->drawPixmap(iconPos, pixmap);
  }

  // If this is a .app/.AppDir bundle with no executable that can actually
  // run on this platform (e.g. a macOS .app shipping only a Mach-O binary),
  // mark it clearly instead of letting it look like any other launchable
  // app and then silently failing to open.
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
          QString cacheKey = "forbidden_" + QString::number(opt.decorationSize.width());
          QPixmap forbiddenPix;
          auto it = iconPixmapCache_.constFind(cacheKey);
          if(it != iconPixmapCache_.constEnd()) forbiddenPix = it.value();
          else {
              QIcon forbiddenIcon(":/icons/forbidden.png");
              forbiddenPix = forbiddenIcon.pixmap(opt.decorationSize * 0.55);
              iconPixmapCache_[cacheKey] = forbiddenPix;
          }
          QPoint forbiddenPos = iconPos;
          forbiddenPos.setX(forbiddenPos.x() + (opt.decorationSize.width() - forbiddenPix.width()) / 2);
          forbiddenPos.setY(forbiddenPos.y() + (opt.decorationSize.height() - forbiddenPix.height()) / 2);
          painter->setOpacity(0.85);
          painter->drawPixmap(forbiddenPos, forbiddenPix);
          painter->setOpacity(1);
      }
  }

  if(file) {
      if(fm_file_info_is_symlink(file)) {
          // draw some emblems for the item if needed
      // we only support symlink emblem at the moment
      QPoint symlinkPos = iconPos;
      symlinkPos.setY(symlinkPos.y() + opt.decorationSize.height() / 2);
      
      QString cacheKey = "symlink_" + QString::number(iconMode) + "_" + QString::number(opt.decorationSize.width() / 2);
      QPixmap pixmap;
      auto it = iconPixmapCache_.constFind(cacheKey);
      if (it != iconPixmapCache_.constEnd()) pixmap = it.value();
      else {
          pixmap = symlinkIcon_.pixmap(opt.decorationSize / 2, iconMode);
          iconPixmapCache_[cacheKey] = pixmap;
      }
      painter->drawPixmap(symlinkPos, pixmap);
    }
  }

  // probono: git labels
  // FIXME: Why doesn't this work?
  // Fm::drawGitEmblem(path, painter, iconPos, opt, file ,iconMode);

  // ======================================= Start ugly code duplication

  static const qint64 kCacheMs = 5000;
  qint64 now = QDateTime::currentMSecsSinceEpoch();

  // probono: git labels
  QString mimetype = QString::fromUtf8(fm_mime_type_get_type(fm_mime_type_ref(fm_file_info_get_mime_type(file))));
  bool isGit = false;
  bool gitHasChanges = false;
  
  if (mimetype == "inode/directory") {
      auto cacheIt = gitStatusCache_.constFind(path);
      if (cacheIt != gitStatusCache_.constEnd() && (now - cacheIt.value().second) < kCacheMs) {
          int val = cacheIt.value().first;
          isGit = val > 0;
          gitHasChanges = val == 2;
      } else {
          isGit = QFileInfo(path + "/.git").exists();
          if (isGit) {
              QProcess p;
              QProcessEnvironment env;
              env.insert("LANG", "C");
              p.setProcessEnvironment(env);
              p.setProgram("git");
              p.setWorkingDirectory(path);
              p.setArguments({"status", "-s"});
              p.start();
              p.waitForFinished();
              gitHasChanges = p.readAllStandardOutput().length() > 0;
          }
          gitStatusCache_[path] = qMakePair(isGit ? (gitHasChanges ? 2 : 1) : 0, now);
      }
  }

  // "emblem-symbolic-git" isn't a standard freedesktop icon name and isn't
  // bundled with the app, so it's missing from most icon themes;
  // QIcon::fromTheme() would then resolve to the theme's generic "unknown"
  // icon (a question mark) instead of drawing nothing.
  if (isGit && QIcon::hasThemeIcon("emblem-symbolic-git")) {
      QString cacheKey = "git_" + QString::number(iconMode) + "_" + QString::number(opt.decorationSize.width() / 2);
      QPixmap pixmap;
      auto it = iconPixmapCache_.constFind(cacheKey);
      if (it != iconPixmapCache_.constEnd()) pixmap = it.value();
      else {
          QIcon emblemIcon = QIcon::fromTheme("emblem-symbolic-git");
          pixmap = emblemIcon.pixmap(opt.decorationSize / 2, iconMode);
          iconPixmapCache_[cacheKey] = pixmap;
      }

      QPoint emblemPos = iconPos;
      emblemPos.setX(emblemPos.x() + opt.decorationSize.width()/4);
      emblemPos.setY(emblemPos.y() + opt.decorationSize.height()/2.6);
      if(gitHasChanges) {
           painter->setOpacity(0.5);
      } else {
          painter->setOpacity(0.1);
      }
      painter->drawPixmap(emblemPos, pixmap);
      painter->setOpacity(1);
  }

  // ======================================= End ugly code duplication

  // probono: Draw label emblems
  bool ok = false;
  QString emblem;
  auto emblemCacheIt = emblemCache_.constFind(path);
  if (emblemCacheIt != emblemCache_.constEnd() && (now - emblemCacheIt.value().second) < kCacheMs) {
      emblem = emblemCacheIt.value().first;
      ok = !emblem.isEmpty();
  } else {
      emblem = Fm::getAttributeValueQString(path, "EMBLEM", ok);
      emblemCache_[path] = qMakePair(ok ? emblem : QString(), now);
  }

  if (ok) {
      QString cacheKey = "emblem_" + emblem + "_" + QString::number(iconMode) + "_" + QString::number(opt.decorationSize.width() / 2);
      QPixmap pixmap;
      auto it = iconPixmapCache_.constFind(cacheKey);
      if (it != iconPixmapCache_.constEnd()) pixmap = it.value();
      else {
          QIcon emblemIcon = QIcon::fromTheme(emblem);
          pixmap = emblemIcon.pixmap(opt.decorationSize / 2, iconMode);
          iconPixmapCache_[cacheKey] = pixmap;
      }
      QPoint emblemIconPos(opt.decorationSize.width() * 0.75 + opt.rect.x() + (opt.rect.width() - opt.decorationSize.width()) / 2, opt.rect.y());
      painter->drawPixmap(emblemIconPos, pixmap);
  }

  // Finder-style tag swatch: a small colored dot at the icon's bottom-left.
  bool tagOk = false;
  QString tagName;
  auto tagCacheIt = tagCache_.constFind(path);
  if (tagCacheIt != tagCache_.constEnd() && (now - tagCacheIt.value().second) < kCacheMs) {
      tagName = tagCacheIt.value().first;
      tagOk = !tagName.isEmpty();
  } else {
      tagName = Fm::getFileTag(path, tagOk);
      tagCache_[path] = qMakePair(tagOk ? tagName : QString(), now);
  }

  if (tagOk) {
      QColor tagColor = Fm::tagColorForName(tagName);
      QRectF dot(iconPos.x() + opt.decorationSize.width() * 0.05,
                 iconPos.y() + opt.decorationSize.height() * 0.75,
                 8, 8);
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setBrush(tagColor);
      painter->setPen(Qt::NoPen);
      painter->drawEllipse(dot);
  }

  // While this item is being renamed, the transparent QLineEdit editor
  // (see createEditor()) sits exactly where the label is drawn -- it was
  // made transparent specifically so it reads as "the label itself became
  // editable" rather than a box on top of it. But that only works if we
  // actually stop painting the static label underneath while editing;
  // QStyledItemDelegate keeps calling paint() for the index being edited,
  // so without this check the old label text kept showing through behind
  // (and around) whatever was being typed, looking like double text.
  // (QStyle::State_Editing exists for this but isn't reliably set by
  // QAbstractItemView across Qt versions/styles, so check the view directly;
  // QAbstractItemView::state() is protected, but indexWidget() -- which is
  // also populated for delegate-created inline editors, not just
  // setIndexWidget() -- is public and gives the same answer.)
  if(view_ && view_->indexWidget(index) != nullptr)
    return;

  // Calculate size of textRect
  QRectF textRect;
  if(labelRight_) {
      int textLeft = iconPos.x() + opt.decorationSize.width() + 10;
      textRect = QRectF(textLeft,
                        opt.rect.y(),
                        opt.rect.width() - textLeft,
                        opt.rect.height());
  } else {
      // The label's wrap/elide width is a fixed 100-110px band, independent
      // of Grid Spacing (which is the gap *between* cells, not the text
      // column width within one) -- centered in the cell.
      static const int kTextWidth = 105;
      static const int kIconTextGap = 5; // vertical gap from icon's bottom edge to text top
      int leftAndRightSpace = (int)((opt.rect.width() - kTextWidth) / 2.0);
      textRect = QRectF(opt.rect.x() + leftAndRightSpace,
                        opt.rect.y() + kIconTextGap + opt.decorationSize.height(),
                        kTextWidth,
                        opt.rect.height() - opt.decorationSize.height());
  }

  // Desktop icon labels always use the distro-provided Mutern VF font
  // (pearos-muternvf package), regardless of the user's UI/document font
  // settings -- there is deliberately no user-facing way to change it.
  // Mutern VF ships each weight as its own named face (fc-list shows
  // "Text Regular", "Text Medium", "Text SemiBold", ...) rather than a
  // single face Qt can bold via setBold() -- that just picked whatever
  // face happened to match "bold" and came out heavier than intended.
  // Selecting the Medium face by name gives a slight, deliberate emphasis
  // instead.
  QFont desktopFont(QStringLiteral("Mutern VF"));
  desktopFont.setStyleName(QStringLiteral("Text Medium"));
  desktopFont.setPointSize(textSize_);
  desktopFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.2); // ~0.015em at 12pt
  
  QString displayString = opt.text;
  if(showItemInfo_ && file) {
      if(fm_file_info_is_dir(file)) {
          displayString += "\n" + tr("-- items"); // Placeholder for item count
      } else {
          QString sizeStr = QLocale().formattedDataSize(fm_file_info_get_size(file));
          displayString += "\n" + sizeStr;
      }
  }

  // The cached pixmap's width and the horizontal centering of its text are
  // both baked in at build time from textRect.width() (which depends on
  // gridSpacing_/labelRight_), then later blitted as-is at
  // textRect.topLeft() -- so the key must include whatever inputs affect
  // textRect's width, or a spacing/label-position change reuses a pixmap
  // sized for the old width and the label drifts off-center under the icon.
  QString textCacheKey = path + "_" + displayString + "_" + QString::number(textSize_)
                        + "_" + QString::number((int)textRect.width())
                        + "_" + (labelRight_ ? "r" : "b");
  QPixmap textPix;
  qreal width = 0;
  qreal height = 0;

  auto pixIt = textPixmapCache_.constFind(textCacheKey);
  if (pixIt != textPixmapCache_.constEnd()) {
      QSizeF size = pixIt.value().first;
      width = size.width();
      height = size.height();
      textPix = pixIt.value().second;
  } else {
      QString textForLayout = displayString;
      {
          // The ellipsis must always land right after the first 20
          // characters -- not wherever the word-wrap loop below happens to
          // still be mid-overflow, which shifted around with font
          // size/grid spacing/text content. Truncate by character count up
          // front: keep the first 20 characters fixed, then binary-search
          // the longest tail that still lays out within kMaxTextLines lines
          // using the real wrapping algorithm (a pixel-width estimate here
          // was unreliable since per-line wrapping doesn't pack lines to
          // exactly textRect.width() each).
          const int kElideHeadChars = 20;
          // Mirrors the overflow conditions of the real wrap loop below
          // (both the kMaxTextLines line cap AND the textRect height
          // check) -- checking only the line cap here let candidates
          // through that the real loop still considered too tall, so it
          // fell back to its own single-line elide on top of the "..."
          // already inserted here, producing a doubled-up ellipsis.
          auto fitsInMaxLines = [&](const QString& s) {
              QTextLayout testLayout(s, desktopFont);
              QTextOption to;
              to.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
              testLayout.setTextOption(to);
              testLayout.beginLayout();
              int lineCount = 0;
              qreal h = 0;
              bool ok = true;
              for(;;) {
                  QTextLine l = testLayout.createLine();
                  if(!l.isValid())
                      break;
                  l.setLineWidth(textRect.width());
                  l.setPosition(QPointF(0, h));
                  bool overflowsHeight = (h + kLineHeight + textRect.y()) + 3 > textRect.bottom();
                  if(overflowsHeight || ++lineCount > kMaxTextLines) {
                      ok = false;
                      break;
                  }
                  h += kLineHeight;
              }
              testLayout.endLayout();
              return ok;
          };

          if(textForLayout.length() > kElideHeadChars && !fitsInMaxLines(textForLayout)) {
              QString head = textForLayout.left(kElideHeadChars);
              QString rest = textForLayout.mid(kElideHeadChars);
              int lo = 0, hi = rest.length();
              QString best;
              while(lo <= hi) {
                  int mid = (lo + hi) / 2;
                  QString candidate = head + QStringLiteral("...") + rest.right(mid);
                  if(fitsInMaxLines(candidate)) {
                      best = candidate;
                      lo = mid + 1;
                  } else {
                      hi = mid - 1;
                  }
              }
              textForLayout = best.isEmpty() ? (head + QStringLiteral("...")) : best;
          }
      }

      QTextLayout layout(textForLayout, desktopFont);
      QTextOption textOption;
      textOption.setAlignment(opt.displayAlignment);
      textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
      textOption.setTextDirection(opt.direction);
      layout.setTextOption(textOption);

      int visibleLines = 0;
      layout.beginLayout();
      // Per-line plain-text substrings and baseline y's, drawn below via
      // QPainter::drawText() rather than QTextLine::draw(). QTextLine::draw()
      // positions a wrapped continuation line's glyphs using the ORIGINAL
      // shaped run's cumulative advance (from the start of the whole string),
      // not relative to that line's own box -- so a manually computed center
      // offset lands in the wrong place for any line after the first (verified
      // with a standalone repro: "NotchBox" wrapped to "NotchB"/"ox" rendered
      // the second line ~15-20px right of where the centering math placed it).
      // Re-drawing each line as its own plain string via drawText() reshapes
      // it fresh and sidesteps that quirk entirely -- the same technique the
      // (correctly-centered) elided-last-line case below already used.
      QVector<QString> lineTexts;
      QVector<qreal> lineBaselineY;
      QString elidedText;

      QFontMetrics layoutMetrics(desktopFont);
      for(;;) {
        QTextLine line = layout.createLine();
        if(!line.isValid())
          break;
        line.setLineWidth(textRect.width());
        line.setPosition(QPointF(0, height));
        bool overflowsHeight = (height + kLineHeight + textRect.y()) + 3 > textRect.bottom();
        bool overflowsLineCap = visibleLines >= kMaxTextLines;
        if(overflowsHeight || overflowsLineCap) {
            QTextLine lastLine = visibleLines > 0 ? layout.lineAt(visibleLines - 1) : line;
            elidedText = textForLayout.mid(lastLine.textStart());
            opt.textElideMode = Qt::ElideMiddle;
            elidedText = layoutMetrics.elidedText(elidedText, opt.textElideMode, textRect.width());
            break;
        }
        lineTexts.push_back(textForLayout.mid(line.textStart(), line.textLength()));
        // Center the font's natural line box within the fixed kLineHeight
        // slot so a 16px line height doesn't clip or bunch up the glyphs.
        lineBaselineY.push_back(height + (kLineHeight + layoutMetrics.ascent() - layoutMetrics.descent()) / 2.0);
        height += kLineHeight;
        ++ visibleLines;
        width = qMax(width, line.naturalTextWidth());
      }
      layout.endLayout();

      QFontMetrics fontMetrics(desktopFont);
      if(!elidedText.isEmpty()) {
          // Replace the last (truncated) line with the elided text -- this is
          // what actually gets drawn for that line, so measurements/drawing
          // must use it, not the untruncated substring.
          if(!lineTexts.isEmpty())
              lineTexts.last() = elidedText;
          else {
              lineTexts.push_back(elidedText);
              lineBaselineY.push_back(fontMetrics.ascent());
              height = fontMetrics.height();
          }
          // The elided line's width was never folded into `width` above
          // (it's computed after the loop breaks), so a 2-line item whose
          // second line is wider than its first ended up with a selection-
          // highlight box (and cached size) sized to the narrower first line
          // only -- making the highlight look off-center relative to the
          // actual (wider) label.
          width = qMax(width, (qreal)fontMetrics.horizontalAdvance(elidedText));
      }

      int pixWidth = (int)textRect.width() + 4;
      int pixHeight = (int)height + 8;
      textPix = QPixmap(pixWidth, pixHeight);
      textPix.fill(Qt::transparent);
      QPainter p(&textPix);
      p.setFont(desktopFont);

      for(const ShadowSample& sample : kShadowKernel) {
        QColor shadow = shadowColor_;
        shadow.setAlphaF(sample.alpha);
        p.setPen(QPen(shadow));
        for(int i = 0; i < lineTexts.size(); ++i) {
          qreal x = (pixWidth - fontMetrics.horizontalAdvance(lineTexts.at(i))) / 2.0;
          p.drawText(QPointF(x + sample.offset.x(), lineBaselineY.at(i) + sample.offset.y()), lineTexts.at(i));
        }
      }

      p.setPen(QPen(textColor_));
      for(int i = 0; i < lineTexts.size(); ++i) {
        qreal x = (pixWidth - fontMetrics.horizontalAdvance(lineTexts.at(i))) / 2.0;
        p.drawText(QPointF(x, lineBaselineY.at(i)), lineTexts.at(i));
      }

      textPixmapCache_[textCacheKey] = qMakePair(QSizeF(width, height), textPix);
  }

  // probono: draw background rounded rect for selected item
  QRectF boundRect;
  int additionalSpace = 1;
  boundRect.setWidth(width + 16 * additionalSpace);
  boundRect.setHeight(height + additionalSpace);
  boundRect.moveTo(textRect.x() - 8*additionalSpace + (textRect.width() - width)/2,
                   textRect.y() - additionalSpace);

  if((opt.state & QStyle::State_Selected) && opt.widget) {
    painter->setFont(desktopFont);
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath path = QPainterPath();
    path.addRoundedRect(boundRect, 8, 8);
    painter->fillPath(path, opt.palette.highlight());
  }

    painter->drawPixmap(textRect.topLeft(), textPix);

  if(opt.state & QStyle::State_HasFocus) {
    // FIXME: draw focus rect
  }
  // painter->restore();
}

QSize DesktopItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QVariant value = index.data(Qt::SizeHintRole);
  if(value.isValid())
    return qvariant_cast<QSize>(value);
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  int iconWidth = opt.decorationSize.width();
  int iconHeight = opt.decorationSize.height();

  // Must stay in sync with DesktopWindow::gridCellSize(), which is what
  // actually positions icons -- this is only the view's layout hint.
  // gridSpacing affects width only; vertical padding is fixed (see
  // DesktopWindow::gridCellSize()).
  static const int kVerticalPadding = 10;
  if(labelRight_) {
     return QSize(iconWidth + 120 + gridSpacing_, qMax(iconHeight, 40) + kVerticalPadding);
  } else {
     static const int kIconTextGap = 5;
     static const int kMaxTextHeight = 32;
     int width = iconWidth + gridSpacing_;
     int height = iconHeight + kIconTextGap + kMaxTextHeight + kVerticalPadding;
     return QSize(width, height);
  }
}

QWidget* DesktopItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
  // The default QStyledItemDelegate::createEditor() returns a plain,
  // unstyled QLineEdit sized to the WHOLE icon+label cell -- a stark white
  // rectangle with a thin native border, covering the icon too. And giving
  // it its own visible background/pill (an earlier attempt) still read as
  // "a box placed on top of the label" rather than the label itself becoming
  // editable. So: fully transparent, no border, no padding, no fill --
  // same white bold text as the static label, same position, nothing else
  // painted. The blinking caret is the only visual cue this is now editable.
  QLineEdit* editor = new QLineEdit(parent);
  editor->setFrame(false);
  editor->setAlignment(Qt::AlignHCenter);
  // Match the static label's font exactly (Mutern VF, Text Medium, same
  // letter spacing) so the editor reads as the label itself, not a
  // different-looking box replacing it.
  QFont f(QStringLiteral("Mutern VF"));
  f.setStyleName(QStringLiteral("Text Medium"));
  f.setPointSize(textSize_);
  f.setLetterSpacing(QFont::AbsoluteSpacing, 0.2);
  editor->setFont(f);
  editor->setAttribute(Qt::WA_TranslucentBackground);
  editor->setStyleSheet(
      "QLineEdit {"
      "  background: transparent;"
      "  color: #ffffff;"
      "  border: none;"
      "  padding: 0px;"
      "  selection-background-color: rgba(47, 124, 246, 160);"
      "  selection-color: #ffffff;"
      "}"
  );
  return editor;
}

void DesktopItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
  // Base implementation seeds the editor's text from index.data(Qt::EditRole)
  // (now fixed in FolderModel::data() to actually return the current name
  // instead of an empty QVariant). Select it all afterwards, Windows/Finder-
  // style, so typing immediately overwrites while clicking/arrow keys still
  // let you edit just part of the name.
  QStyledItemDelegate::setEditorData(editor, index);
  if(QLineEdit* le = qobject_cast<QLineEdit*>(editor))
    le->selectAll();
}

void DesktopItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // Mirror paint()'s icon geometry so the editor lands exactly where the
  // label itself is drawn, rather than the default (whole-cell) rect.
  int decorationHeight = opt.decorationSize.height();
  QRect cellRect = opt.rect;
  QRect labelRect;
  if(labelRight_) {
      int iconRight = cellRect.x() + gridSpacing_ / 2 + opt.decorationSize.width();
      labelRect = QRect(iconRight + 10, cellRect.y(), cellRect.width() - (iconRight + 10 - cellRect.x()), cellRect.height());
  } else {
      // Must match paint()'s textRect exactly (fixed 100-110px band,
      // centered, independent of gridSpacing_; kIconTextGap above the icon;
      // room for up to kMaxTextLines lines at kLineHeight each) -- this had
      // fallen out of sync with paint() when the text geometry there was
      // reworked, leaving the editable box far too small (10px wide) to
      // type into.
      const int kTextWidth = 105;
      const int kIconTextGap = 5;
      int leftAndRightSpace = (int)((cellRect.width() - kTextWidth) / 2.0);
      labelRect = QRect(cellRect.x() + leftAndRightSpace,
                         cellRect.y() + kIconTextGap + decorationHeight,
                         kTextWidth,
                         (int)(kMaxTextLines * kLineHeight) + 4);
  }
  editor->setGeometry(labelRect);
}

DesktopItemDelegate::~DesktopItemDelegate() {

}

void DesktopItemDelegate::clearIconCache() {
    iconPixmapCache_.clear();
    if(view_ && view_->viewport())
        view_->viewport()->update();
}
