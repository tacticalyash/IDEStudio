#include "SyntaxHighlighter.hpp"

KSyntaxHighlighter::KSyntaxHighlighter(QTextDocument *doc) : QSyntaxHighlighter(doc)
{
    //
    m_function_rule.rexpr = QRegularExpression("\\b[A-Za-z0-9_]+\\s*\\(");
    m_function_rule.format.setForeground(Qt::GlobalColor::darkCyan);
    m_function_rule.format.setFontItalic(true);

    //
    QStringList keywords = {
        "alignas", "alignof", "and", "and_eq", "asm",
        "atomic_cancel", "atomic_commit", "atomic_noexcept", "bitand",
        "bitor", "break", "case", "catch", "class",
        "co_await", "co_return", "co_yield", "compl", "concept",
        "const", "const_cast", "consteval", "constexpr", "constinit",
        "continue", "decltype", "default", "delete", "double",
        "dynamic_cast", "else", "enum", "explicit", "export",
        "extern", "for", "friend", "goto", "if", "inline",
        "mutable", "namespace", "new", "noexcept", "not",
        "not_eq", "operator", "or", "or_eq", "private", "protected",
        "public", "reflexpr", "register", "reinterpret_cast", "requires",
        "return", "sizeof", "static", "static_assert", "static_cast",
        "struct", "switch", "synchronized", "template", "this",
        "thread_local", "throw", "try", "typedef", "typeid",
        "typename", "union", "using", "virtual", "volatile",
        "while", "xor", "xor_eq"
    };

    m_keyword_rule.format.setForeground(Qt::GlobalColor::magenta);
    //m_keyword_rule.format.setFontWeight(QFont::Bold);
    m_keyword_rule.format.setFontItalic(true);
    m_keyword_rule.searcher.setList(keywords);

    QStringList built_in_types = {
        "auto","bool", "char", "char16_t", "char32_t",
        "char8_t", "float", "int", "int16_t", "int32_t",
        "int64_t", "int8_t", "long", "short", "signed",
        "uint16_t", "uint32_t", "uint64_t", "uint8_t", "unsigned",
        "void", "wchar_t"
    };

    m_builtin_types_rule.format.setForeground(Qt::GlobalColor::darkGreen);
    m_builtin_types_rule.format.setFontWeight(QFont::Bold);
    m_builtin_types_rule.searcher.setList(built_in_types);
    //
    m_parentheses_rule.rexpr = QRegularExpression("[\\(\\)\\{\\}\\[\\]]");
    m_parentheses_rule.format.setFontWeight(QFont::Bold);
    //
    m_preprocessor_rule.rexpr = QRegularExpression("^\\s*#.+$");
    m_preprocessor_rule.format.setForeground(Qt::GlobalColor::darkMagenta);
    m_preprocessor_rule.format.setFontItalic(true);

    //
    m_single_line_comment_rule.rexpr = QRegularExpression("//[^\n]*");
    m_single_line_comment_rule.format.setForeground(Qt::GlobalColor::darkCyan);

    //
    QStringList literals = {
        "\\b(0|([1-9]\\d*))(u|U|l|L|ll|LL|((u|U)(l|L))|((l|L)(u|U))|((u|U)(ll|LL))|((ll|LL)(u|U)))?\\b",  //integers (decimals)
        "\\b0x[\\dA-Fa-f]+\\b",             //integers (hexadecimals)
        "\\b0[01234567]*\\b",               //integers (octal)
        "\\b0b[01]+\\b",                    //integers (binary)
        "'\\\\?.'",                              //characters
        "L'..'",                            //wide characters
        "\"([^\"\\\\]|\\\\.)*\"",
        "\\b\\d+\\.\\d*(f)?\\b",            //real numbers (decimals)
        "true",
        "false",
        "nullptr"
    };
    RE_Rules literal_rule;
    literal_rule.format.setForeground(Qt::GlobalColor::red);
    literal_rule.format.setFontItalic(true);
    for(const QString &lit : literals)
    {
        literal_rule.rexpr = QRegularExpression(lit);
        m_literal_rules.append(literal_rule);
    }
    //multi line comment
    /*
    */
    m_multi_line_comment_start_exp = QRegularExpression("/\\*");
    m_multi_line_comment_end_exp = QRegularExpression("\\*/");
    m_multi_line_comment_format.setForeground(Qt::GlobalColor::darkRed);
}

void KSyntaxHighlighter::highlightBlock(const QString &text)
{
    QRegularExpressionMatchIterator match_it;
    QRegularExpressionMatch match;

    match_it = m_function_rule.rexpr.globalMatch(text);
    while(match_it.hasNext())
    {
        match = match_it.next();
        setFormat(match.capturedStart(),match.capturedLength(),m_function_rule.format);
    }

    match_it = m_parentheses_rule.rexpr.globalMatch(text);
    while(match_it.hasNext())
    {
        match = match_it.next();
        setFormat(match.capturedStart(),match.capturedLength(),m_parentheses_rule.format);
    }

    m_keyword_rule.searcher.setText(text);
    int keyword_index = m_keyword_rule.searcher.next();
    while(keyword_index != -1)
    {
        int capturedLength = m_keyword_rule.searcher.capturedLength();
        setFormat(keyword_index, capturedLength, m_keyword_rule.format);
        keyword_index = m_keyword_rule.searcher.next(keyword_index+capturedLength);
    }
    m_builtin_types_rule.searcher.setText(text);
    int bi_types_index = m_builtin_types_rule.searcher.next();
    while(bi_types_index != -1)
    {
        int capturedLength = m_builtin_types_rule.searcher.capturedLength();
        setFormat(bi_types_index, capturedLength, m_builtin_types_rule.format);
        bi_types_index = m_builtin_types_rule.searcher.next(bi_types_index+capturedLength);
    }

    for(const auto &rule : m_literal_rules)
    {
        match_it = rule.rexpr.globalMatch(text);
        while(match_it.hasNext())
        {
            match = match_it.next();
            setFormat(match.capturedStart(),match.capturedLength(),rule.format);
        }
    }


    match = m_preprocessor_rule.rexpr.match(text);
    if(match.hasMatch())
    {
        setFormat(match.capturedStart(),match.capturedLength(),m_preprocessor_rule.format);
    }

    match_it = m_single_line_comment_rule.rexpr.globalMatch(text);
    while(match_it.hasNext())
    {
        match = match_it.next();
        if(format(match.capturedStart()) != m_literal_rules[0].format)
        {
            setFormat(match.capturedStart(),match.capturedLength(),m_single_line_comment_rule.format);
            break;  //match only first comment sign
        }
    }

    setCurrentBlockState(0);
    int mlc_start_index = 0;
    //if previous line was not part of the multi line comment, find the start of the multi line comment
    if(previousBlockState() != BlockState::MultiLineComment)
    {
        mlc_start_index = text.indexOf(m_multi_line_comment_start_exp);
    }
    //if a multi level index found
    while(mlc_start_index >= 0)
    {
        QRegularExpressionMatch end_match;
            int endIndex = text.indexOf(m_multi_line_comment_end_exp, mlc_start_index, &end_match);
            int comment_length;
            if (endIndex == -1) {
                setCurrentBlockState(BlockState::MultiLineComment);
                comment_length = text.length() - mlc_start_index;
            } else {
                comment_length = endIndex - mlc_start_index
                                + end_match.capturedLength();
            }
            setFormat(mlc_start_index, comment_length, m_multi_line_comment_format);
            mlc_start_index = text.indexOf(m_multi_line_comment_start_exp,
                                      mlc_start_index + comment_length);
    }
}

KSyntaxHighlighter::~KSyntaxHighlighter()
{
    //qDebug(__FUNCTION__);
}

/*
 *
 */

int PlainTextSearch::next(int index)
{
    if(index < 0)
        return -1;
    const int msize = m_string.size();
    int start_index = index;
    if(start_index >= m_string.size())
        return -1;
    while(start_index < msize && !(m_string[start_index].isLetterOrNumber() || m_string[start_index].toLatin1() == '_'))
    {
        ++start_index;
    }

    if(start_index == m_string.size())
        return -1;

    int end_index = start_index + 1;
    while(end_index < msize && (m_string[end_index].isLetterOrNumber() || m_string[end_index].toLatin1() == '_'))
    {
        ++end_index;
    }
    QStringView view(m_string.cbegin()+start_index, m_string.cbegin()+end_index);
    if(std::binary_search(m_list.cbegin(),m_list.cend(),view))
    {
        m_length = end_index - start_index;
        return start_index;
    }
    else
    {
        return next(end_index);
    }
}
