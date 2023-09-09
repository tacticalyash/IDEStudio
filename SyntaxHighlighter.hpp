#ifndef KM_SYNTAXHIGHLIGHTER_HPP
#define KM_SYNTAXHIGHLIGHTER_HPP

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <vector>
#include <QStringList>
#include <QStringView>

/**
 * @brief The PlainTextSearch class
 * Helper class to replace regular expressions to some extents.
 * if passed list is => ["alpha", "beta", "gamma"]
 * then `next()` will search for words (series containing letters,
 * digits and underscore `_`) and will search it on the list (binary search).
 * If it is found in list, starting index of the word is returned and length
 * can be retrieved using `capturedLength()`. If not then -1 is returned.
*/

class PlainTextSearch
{
public:
    PlainTextSearch() : m_length(0) {}
    PlainTextSearch(const QStringList &list) : m_list(list), m_length(0) {}
    PlainTextSearch(QStringList &&list) : m_list(list), m_length(0) {}
    void setList(const QStringList &list) { m_list = list;}
    void setText(const QString &line) {m_string = line;}
    int capturedLength() const {return m_length;};
    int next(int index = 0);
private:
    QStringList m_list;
    QString m_string;
    int m_length;
};

class KSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    KSyntaxHighlighter(QTextDocument *doc);
    void highlightBlock(const QString &text) override;
    ~KSyntaxHighlighter();
private:
    struct RE_Rules{
        QTextCharFormat format;
        QRegularExpression rexpr;
    };
    QVector<RE_Rules> m_literal_rules;
    RE_Rules m_parentheses_rule;
    RE_Rules m_preprocessor_rule;
    RE_Rules m_single_line_comment_rule;
    RE_Rules m_function_rule;
    //
    struct PTS_Rules{
        QTextCharFormat format;
        PlainTextSearch searcher;
    };
    PTS_Rules m_keyword_rule;
    PTS_Rules m_builtin_types_rule;

    QRegularExpression m_multi_line_comment_start_exp;
    QRegularExpression m_multi_line_comment_end_exp;
    QTextCharFormat m_multi_line_comment_format;
    //
    enum BlockState{
        MultiLineComment = 1,
    };
};

#endif // KM_SYNTAXHIGHLIGHTER_HPP
