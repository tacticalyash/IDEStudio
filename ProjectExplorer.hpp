#ifndef PROJECT_EXPLORER_HPP
#define PROJECT_EXPLORER_HPP

#include <QWidget>

class ProjectModel;
class ProjectExplorer : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectExplorer(ProjectModel *model,QWidget *parent = nullptr);
    ProjectModel *getModel();
signals:
private:
    const QString m_project_name;
    ProjectModel *m_model;
};

#endif // PROJECTVIEW_HPP
