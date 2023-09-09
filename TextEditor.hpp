#ifndef KM_TEXTEDITOR_HPP
#define KM_TEXTEDITOR_HPP

#include <QPlainTextEdit>
#include <QCompleter>

class CppCompletionModel;
class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr, bool syntax = true);
    QCompleter *completer() const;
    void setModel(CppCompletionModel *model);
    void lnaPaintEvent(QPaintEvent *event);
    int lnaWidth();

    void setFileName(const QString &fileName);
    const QString &getFileName() const;

    void markSaved();
    bool isSaved();
protected:
    void resizeEvent(QResizeEvent *event) override;

    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    //
private slots:
    void updateLnaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLna(const QRect &rect, int dy);

    void insertCompletion(const QString &completion);
private:
    QString textUnderCursor(bool moveCursor = false) const;
    void setCompleter(QCompleter *cmpltr);

private:
    QWidget *m_lna; //line number area
    QCompleter *m_completer;
    QString m_full_file_path;
    bool m_saved;
};

inline void TextEditor::setFileName(const QString &fileName)
{
    m_full_file_path = fileName;
}

inline const QString &TextEditor::getFileName() const
{
    return m_full_file_path;
}

inline void TextEditor::markSaved()
{
    m_saved = true;
}

inline bool TextEditor::isSaved()
{
    return m_saved;
}




#endif // KM_TEXTEDITOR_HPP
