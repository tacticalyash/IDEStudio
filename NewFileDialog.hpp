#ifndef NEWFILEDIALOG_HPP
#define NEWFILEDIALOG_HPP

#include <QDialog>

class QRegularExpressionValidator;
class QComboBox;
class QLineEdit;
class NewFileDialog : public QDialog
{
public:
    NewFileDialog(QWidget *parent, const QString &title, QStringList extensions);
    QString getFileName() const;
    static QString getText(QWidget *parent, const QString &title, QStringList extensions = {}, QRegularExpressionValidator *validator = nullptr);
    ~NewFileDialog() = default;
private:
    QLineEdit *m_file_name;
    QComboBox *m_file_ext;
};

#endif // NEWFILEDIALOG_HPP
