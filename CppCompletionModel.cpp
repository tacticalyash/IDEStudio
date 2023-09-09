#include "CppCompletionModel.hpp"

#include <QFontMetrics>

CppCompletionModel::CppCompletionModel(QObject *parent) : QAbstractListModel(parent)
{
    m_completions = {
        "algorithm",
        "alignas",
        "alignof",
        "and",
        "and_eq",
        "asm",
        "atomic_cancel",
        "atomic_commit",
        "atomic_noexcept",
        "auto",
        "begin",
        "bitand",
        "bitor",
        "bool",
        "break",
        "case",
        "catch",
        "cerr",
        "char",
        "char16_t",
        "char32_t",
        "char8_t",
        "cin",
        "class",
        "clog",
        "co_await",
        "co_return",
        "co_yield",
        "compl",
        "concept",
        "const",
        "const_cast",
        "const_iterator",
        "consteval",
        "constexpr",
        "constinit",
        "continue",
        "cout",
        "decltype",
        "default",
        "define",
        "delete",
        "double",
        "dynamic_cast",
        "else",
        "enable_if",
        "end",
        "endif",
        "endl",
        "enum",
        "error",
        "explicit",
        "export",
        "extern",
        "false",
        "float",
        "for",
        "foreach",
        "forward_list",
        "friend",
        "goto",
        "ifdef",
        "ifndef",
        "include",
        "include<algorithm>",
        "include<array>",
        "include<atomic>",
        "include<bitset>",
        "include<cassert>",
        "include<ccomplex>",
        "include<cctype>",
        "include<cerrno>",
        "include<cfenv>",
        "include<cfloat>",
        "include<chrono>",
        "include<cinttypes>",
        "include<ciso646>",
        "include<climits>",
        "include<clocale>",
        "include<cmath>",
        "include<complex>",
        "include<condition_variable>",
        "include<csetjmp>",
        "include<csignal>",
        "include<cstdalign>",
        "include<cstdarg>",
        "include<cstdbool>",
        "include<cstddef>",
        "include<cstdint>",
        "include<cstdio>",
        "include<cstdlib>",
        "include<cstring>",
        "include<ctgmath>",
        "include<ctime>",
        "include<cwchar>",
        "include<cwctype>",
        "include<deque>",
        "include<exception>",
        "include<forward_list>",
        "include<fstream>",
        "include<functional>",
        "include<future>",
        "include<initializer_list>",
        "include<iomanip>",
        "include<ios>",
        "include<iosfwd>",
        "include<iostream>",
        "include<istream>",
        "include<iterator>",
        "include<limits>",
        "include<list>",
        "include<locale>",
        "include<map>",
        "include<memory>",
        "include<mutex>",
        "include<new>",
        "include<numeric>",
        "include<ostream>",
        "include<queue>",
        "include<random>",
        "include<ratio>",
        "include<regex>",
        "include<scoped_allocator>",
        "include<set>",
        "include<sstream>",
        "include<stack>",
        "include<stdexcept>",
        "include<streambuf>",
        "include<string>",
        "include<system_error>",
        "include<thread>",
        "include<tuple>",
        "include<type_traits>",
        "include<typeindex>",
        "include<typeinfo>",
        "include<unordered_map>",
        "include<unordered_set>",
        "include<utility>",
        "include<valarray>",
        "include<vector>",
        "inline",
        "int",
        "int16_t",
        "int32_t",
        "int64_t",
        "int8_t",
        "istream",
        "iterator",
        "long",
        "mutable",
        "namespace",
        "new",
        "noexcept",
        "not",
        "not_eq",
        "nullptr",
        "operator",
        "or_eq",
        "ostream",
        "pragma",
        "printf",
        "private",
        "protected",
        "public",
        "read",
        "reflexpr",
        "register",
        "reinterpret_cast",
        "requires",
        "return",
        "scanf",
        "set",
        "short",
        "signed",
        "sizeof",
        "sort(_FItr, _FItr)",
        "static",
        "static_assert",
        "static_cast",
        "std",
        "string",
        "struct",
        "switch",
        "synchronized",
        "template",
        "this",
        "thread_local",
        "throw",
        "to_string(param)",
        "true",
        "try",
        "typedef",
        "typeid",
        "typename",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "uint8_t",
        "union",
        "unsigned",
        "using",
        "vector",
        "vector<type,allocator>",
        "vector<type>",
        "virtual",
        "void",
        "volatile",
        "wcerr",
        "wchar_t",
        "wcin",
        "wclog",
        "wcout",
        "while",
        "write",
        "xor",
        "xor_eq"
    };
    unsigned int size_ = m_completions.size();
    m_indices.reserve(size_);
    for(unsigned int i_ = 0; i_ < size_; ++i_)
    {
        m_indices.push_back(i_);
    }
}

void CppCompletionModel::addUserCompletion(const QString &completion)
{
    auto it = std::lower_bound(m_indices.begin(),m_indices.end(),0,[this,completion](unsigned int v1, unsigned int v2){
        static_cast<void>(v2);
        return m_completions [v1] < completion;
    });
    if(it == m_indices.end() || m_completions[*it] != completion)
    {
        unsigned int index = std::distance(m_indices.begin(),it);
        beginInsertRows(QModelIndex(),index,index);
        m_completions.push_back(completion);
        m_indices.insert(it,m_completions.size() - 1);
        endInsertRows();
        //qDebug("added %s",completion.toStdString().c_str());
    }
}

void CppCompletionModel::setFont(const QFont &font)
{
    m_font = font;
}

int CppCompletionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_indices.size();
}

QVariant CppCompletionModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || (index.column() > 0 || index.row() >= rowCount()))
        return QVariant();
    if(role == Qt::EditRole || role == Qt::DisplayRole)
        return m_completions[m_indices[index.row()]];
    else if(role == Qt::SizeHintRole)
        return QFontMetrics(m_font).size(Qt::TextSingleLine,m_completions[m_indices[index.row()]]);
    else
        return QVariant();
}


Qt::ItemFlags CppCompletionModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}
