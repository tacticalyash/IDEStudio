#include "TextEditor.hpp"

#include <QtGui>
#include <QAbstractItemView>
#include <QScrollBar>

#include "CppCompletionModel.hpp"
#include "SyntaxHighlighter.hpp"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor), textEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(textEditor->lnaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        textEditor->lnaPaintEvent(event);
    }

private:
    TextEditor *textEditor;
};





TextEditor::TextEditor(QWidget *parent, bool syntax)
    :
      QPlainTextEdit(parent),
      m_completer(nullptr),
      m_saved(true)
{
    m_lna = new LineNumberArea(this);

    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLnaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLna);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);

    updateLnaWidth(0);
    highlightCurrentLine();

    setLineWrapMode(QPlainTextEdit::NoWrap);

    setCompleter(new QCompleter(this));
    QFont lFont = font();
    lFont.setPixelSize(14);
    setTabStopDistance(4*QFontMetrics(lFont).horizontalAdvance(' '));
    setFont(lFont);

    if(syntax)
    {
        new KSyntaxHighlighter(this->document());
    }

}


int TextEditor::lnaWidth()
{
    int digits = 1;
    int max = blockCount();
    while (max) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void TextEditor::setCompleter(QCompleter *cmpltr)
{
    if (m_completer)
        m_completer->disconnect(this);

    m_completer = cmpltr;

    if (!m_completer)
        return;

    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated),
                     this, &TextEditor::insertCompletion);
}

QCompleter *TextEditor::completer() const
{
    return m_completer;
}

void TextEditor::setModel(CppCompletionModel *model)
{
    m_completer->setModel(model);
    m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    m_completer->setWrapAround(false);
}

void TextEditor::updateLnaWidth(int /* newBlockCount */)
{
    setViewportMargins(lnaWidth(), 0, 0, 0);
}

void TextEditor::updateLna(const QRect &rect, int dy)
{
    if (dy)
        m_lna->scroll(0, dy);
    else
        m_lna->update(0, rect.y(), m_lna->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLnaWidth(0);
}

void TextEditor::insertCompletion(const QString &completion)
{
    if (m_completer->widget() != this)
        return;
    QTextCursor text_cursor = textCursor();
    text_cursor.movePosition(QTextCursor::Left);
    text_cursor.movePosition(QTextCursor::EndOfWord);
    text_cursor.select(QTextCursor::SelectionType::WordUnderCursor);
    text_cursor.removeSelectedText();
    text_cursor.insertText(completion);
    setTextCursor(text_cursor);
}

//in case if a eow (end of word) found then to get the word before eow call it with moveCursor = true.
QString TextEditor::textUnderCursor(bool moveCursor) const
{
    QTextCursor tc = textCursor();
    if(moveCursor)
        tc.movePosition(QTextCursor::Left);
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TextEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lna->setGeometry(QRect(cr.left(), cr.top(), lnaWidth(), cr.height()));
}

void TextEditor::keyPressEvent(QKeyEvent *e)
{
    if (m_completer && m_completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    const bool is_ctrl = e->modifiers().testFlag(Qt::ControlModifier);
    const bool is_shortcut = ( is_ctrl && e->key() == Qt::Key_Tab); // CTRL+E
    if (!m_completer || !is_shortcut) // do not process the shortcut when we have a completer
    {
        if(e->key() != Qt::Key_Save)
            m_saved = false;
        QPlainTextEdit::keyPressEvent(e);
    }
    const bool ctrl_or_shift = is_ctrl || e->modifiers().testFlag(Qt::ShiftModifier);
    if (!m_completer || (ctrl_or_shift && e->text().isEmpty()))
        return;

    static const QString end_of_word("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-= "); // end of word

    bool is_endof_word = end_of_word.contains(e->text().right(1));

    if((is_endof_word || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return))       //end of word then add it to the list
    {
        QString user_completion = textUnderCursor(true);
        if(user_completion.length() > 2)
        {
            CppCompletionModel *cpp_model = dynamic_cast<CppCompletionModel*>(m_completer->model());
            cpp_model->addUserCompletion(user_completion);
        }
    }

    const bool has_modifier = (e->modifiers() != Qt::NoModifier) && !ctrl_or_shift;
    QString current_text = textUnderCursor();
    if (!is_shortcut && (has_modifier || e->text().isEmpty()|| current_text.length() < 3
                      || is_endof_word)) {
        m_completer->popup()->hide();
        return;
    }

    if (current_text != m_completer->completionPrefix()) {
        m_completer->setCompletionPrefix(current_text);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }

    QRect cr = cursorRect();
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                + m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(cr); // popup it up!
}

void TextEditor::focusInEvent(QFocusEvent *e)
{
    if (m_completer)
        m_completer->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void TextEditor::lnaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lna);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lna->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

