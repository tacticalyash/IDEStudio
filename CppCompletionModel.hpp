#ifndef CPPCOMPLETIONMODEL_H
#define CPPCOMPLETIONMODEL_H

#include <QAbstractListModel>
#include <QFont>

/**
 * @brief The CppCompletion class
 * when adding new word to completion model, new word will be just appended
 * to the m_completions, and its index will be inserted in m_indices in its
 * right place.
 * if m_completions is ["int", "char", "double"]
 * m_indices will be [1 2 0].
 */
class CppCompletionModel final : public QAbstractListModel
{
public:
    explicit CppCompletionModel(QObject *parent);
    /* will add its implementation lator*/
    void addUserCompletion(const QString &completion);
    void setFont(const QFont& font);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
private:
    std::vector<unsigned int> m_indices;
    std::vector<QString> m_completions;
    QFont m_font;

    //
};

#endif // CPPCOMPLETIONMODEL_H
