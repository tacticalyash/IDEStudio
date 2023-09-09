#ifndef KM_NEWPROJECT_HPP
#define KM_NEWPROJECT_HPP

#include <QDialog>
#include <QRadioButton>
#include "ProjectDetails.hpp"

class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;


class NewProjectDialog : public QDialog
{
    Q_OBJECT
public:
    NewProjectDialog(QWidget *parent = nullptr);
    const std::vector<QString> &getDefaultFiles() const;
    const ProjectDetails &getProjectDetails() const;
    ~NewProjectDialog();
private slots:
    void browsPath();
    void onOkButtonClicked();
    void onLanguageChanged();
    void onProjectTypeChanged(const QString &text);
private:
    QLineEdit *m_project_name;
    QLineEdit *m_project_path;
    QComboBox *m_project_types;
    QRadioButton *m_language_c;
    QRadioButton *m_language_cpp;
    QCheckBox *m_main_file;
    QLabel *m_message;

    ProjectDetails m_details;
    std::vector<QString> m_default_files;
};

inline const std::vector<QString> &NewProjectDialog::getDefaultFiles() const
{
    return m_default_files;
}

inline const ProjectDetails &NewProjectDialog::getProjectDetails() const
{
    return m_details;
}



#endif // KM_NEWPROJECT_HPP
