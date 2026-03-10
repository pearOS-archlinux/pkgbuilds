#include "todo_widget.h"

#include <QApplication>
#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>

class NotesEditor : public QTextEdit {
public:
    explicit NotesEditor(QWidget* parent = nullptr)
        : QTextEdit(parent) {
    }

protected:
    void keyPressEvent(QKeyEvent* e) override {
        // Custom, mică indentare pentru Markdown: Tab = 2 spații, Shift+Tab = scoate 2 spații
        if (e->key() == Qt::Key_Tab && !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
            QTextCursor c = textCursor();
            if (e->modifiers() & Qt::ShiftModifier) {
                // Unindent: scoate până la 2 spații la începutul liniei curente
                c.beginEditBlock();
                c.movePosition(QTextCursor::StartOfBlock);
                c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
                const QString sel = c.selectedText();
                int toRemove = 0;
                if (sel.startsWith(QLatin1Char(' ')))
                    ++toRemove;
                if (sel.size() > 1 && sel.at(1) == QLatin1Char(' '))
                    ++toRemove;
                if (toRemove > 0) {
                    c.clearSelection();
                    c.movePosition(QTextCursor::StartOfBlock);
                    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, toRemove);
                    c.removeSelectedText();
                }
                c.endEditBlock();
            } else {
                // Indent: inserează 2 spații la poziția curentă
                c.insertText(QStringLiteral("  "));
            }
            return;
        }

        if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
            !(e->modifiers() & Qt::ShiftModifier)) {
            const QTextCursor cursor = textCursor();
            QTextBlock block = cursor.block();
            const QString blockText = block.text();

            // Leading whitespace (spaces/tabs) to preserve indentation
            QString leading;
            for (QChar ch : blockText) {
                if (ch == QLatin1Char(' ') || ch == QLatin1Char('\t')) {
                    leading.append(ch);
                } else {
                    break;
                }
            }

            // Trimmed line to see if it's an "empty" list marker (end list)
            QString trimmed = blockText.trimmed();

            QRegularExpression bulletRe(QStringLiteral("^(\\s*)([-*+])\\s+"));
            QRegularExpression orderedRe(QStringLiteral("^(\\s*)(\\d+)[\\.)]\\s+"));

            auto bulletMatch = bulletRe.match(blockText);
            auto orderedMatch = orderedRe.match(blockText);

            const bool isBullet = bulletMatch.hasMatch();
            const bool isOrdered = orderedMatch.hasMatch();

            if (isBullet || isOrdered) {
                // End of list if line has only the marker and nothing else
                if ((isBullet && (trimmed == QStringLiteral("-") ||
                                  trimmed == QStringLiteral("*") ||
                                  trimmed == QStringLiteral("+"))) ||
                    (isOrdered && QString(trimmed).remove(QRegularExpression(QStringLiteral("[0-9\\.)]"))).trimmed().isEmpty())) {
                    QTextEdit::keyPressEvent(e);
                    return;
                }

                QString indent;
                QString nextMarker;

                if (isBullet) {
                    indent = bulletMatch.captured(1);
                    const QString symbol = bulletMatch.captured(2);
                    nextMarker = indent + symbol + QLatin1Char(' ');
                } else {
                    indent = orderedMatch.captured(1);
                    bool ok = false;
                    int number = orderedMatch.captured(2).toInt(&ok);
                    if (!ok) number = 1;
                    nextMarker = indent + QString::number(number + 1) + QStringLiteral(". ");
                }

                QTextEdit::keyPressEvent(e);
                QTextCursor c = textCursor();
                c.insertText(nextMarker);
                return;
            }

            // Non-list line: păstrează indentarea de la începutul liniei
            if (!leading.isEmpty()) {
                QTextEdit::keyPressEvent(e);
                QTextCursor c = textCursor();
                c.insertText(leading);
                return;
            }
        }

        // Default behavior (including Shift+Enter or non-list lines)
        QTextEdit::keyPressEvent(e);
    }
};

NotesWidget::NotesWidget(QWidget* parent)
    : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setText(tr("Notes"));
    mainLayout->addWidget(m_titleLabel);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet("color: #a1a1aa; font-size: 13px;");
    mainLayout->addWidget(m_summaryLabel);

    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins(0, 8, 0, 4);
    toolbarLayout->setSpacing(6);
    setupToolbar(toolbarLayout);
    mainLayout->addLayout(toolbarLayout);

    m_editor = new NotesEditor(this);
    m_editor->setAcceptRichText(true);
    m_editor->setPlaceholderText(tr("Write your note here..."));
    m_editor->setFrameStyle(QFrame::NoFrame);
    mainLayout->addWidget(m_editor, 1);

    connect(m_editor, &QTextEdit::textChanged, this, [this]() {
        if (m_blockContentSignal || !m_editor) return;
        emit contentChanged(m_editor->toHtml());
        updateSummary();
    });

    updateSummary();
}

void NotesWidget::setList(const NoteDocument& note) {
    m_titleLabel->setText(note.name);
    setContent(note.content);
}

void NotesWidget::setContent(const QString& content) {
    if (!m_editor) return;
    m_blockContentSignal = true;
    m_editor->setHtml(content.isEmpty() ? QString() : content);
    m_blockContentSignal = false;
    updateSummary();
}

void NotesWidget::updateSummary() {
    if (!m_editor) {
        m_summaryLabel->clear();
        return;
    }
    const QString text = m_editor->toPlainText().trimmed();
    if (text.isEmpty()) {
        m_summaryLabel->setText(tr("No notes yet"));
        return;
    }
    const int chars = text.size();
    const int lines = text.count(QLatin1Char('\n')) + 1;
    m_summaryLabel->setText(tr("%1 lines, %2 characters").arg(lines).arg(chars));
}

void NotesWidget::setupToolbar(QHBoxLayout* toolbarLayout) {
    auto makeToolButton = [this, toolbarLayout](const QString& text, const QString& tooltip) -> QToolButton* {
        auto* btn = new QToolButton(this);
        btn->setText(text);
        btn->setToolTip(tooltip);
        btn->setCheckable(true);
        toolbarLayout->addWidget(btn);
        return btn;
    };

    m_boldButton = makeToolButton(QStringLiteral("B"), tr("Bold"));
    QFont bf = m_boldButton->font();
    bf.setBold(true);
    m_boldButton->setFont(bf);

    m_underlineButton = makeToolButton(QStringLiteral("U"), tr("Underline"));
    QFont uf = m_underlineButton->font();
    uf.setUnderline(true);
    m_underlineButton->setFont(uf);

    m_strikeButton = makeToolButton(QStringLiteral("S"), tr("Strikethrough"));

    toolbarLayout->addSpacing(8);

    m_fontSizeBox = new QComboBox(this);
    m_fontSizeBox->setEditable(false);
    const QList<int> sizes{10, 12, 14, 16, 18, 20};
    for (int s : sizes) {
        m_fontSizeBox->addItem(QString::number(s), s);
    }
    m_fontSizeBox->setCurrentIndex(2);
    toolbarLayout->addWidget(m_fontSizeBox);
    toolbarLayout->addStretch(1);

    connect(m_boldButton, &QToolButton::toggled, this, [this](bool on) {
        applyFormat([on](QTextCharFormat& fmt) {
            fmt.setFontWeight(on ? QFont::Bold : QFont::Normal);
        });
    });

    connect(m_underlineButton, &QToolButton::toggled, this, [this](bool on) {
        applyFormat([on](QTextCharFormat& fmt) {
            fmt.setFontUnderline(on);
        });
    });

    connect(m_strikeButton, &QToolButton::toggled, this, [this](bool on) {
        applyFormat([on](QTextCharFormat& fmt) {
            fmt.setFontStrikeOut(on);
        });
    });

    connect(m_fontSizeBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx < 0 || !m_fontSizeBox) return;
        const int size = m_fontSizeBox->itemData(idx).toInt();
        applyFormat([size](QTextCharFormat& fmt) {
            fmt.setFontPointSize(size);
        });
    });
}

void NotesWidget::applyFormat(std::function<void(QTextCharFormat&)> formatter) {
    if (!m_editor) return;
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection()) {
        QTextCharFormat fmt = cursor.charFormat();
        formatter(fmt);
        cursor.mergeCharFormat(fmt);
        m_editor->mergeCurrentCharFormat(fmt);
    } else {
        QTextCharFormat fmt;
        formatter(fmt);
        cursor.mergeCharFormat(fmt);
        m_editor->mergeCurrentCharFormat(fmt);
    }
}

