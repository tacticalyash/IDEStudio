#ifndef PROJECT_TAB_HPP
#define PROJECT_TAB_HPP

#include "ProjectDetails.hpp"

#include <QWidget>

class ProjectModel;
class QTabWidget;
class ProjectExplorer;

class ProjectTab : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectTab(QWidget *parent = nullptr);
    void createProjectView(ProjectModel *model);
    ProjectModel *getProjectModel();
private:
    QTabWidget *m_tab_widget;
    ProjectExplorer *m_project_explorer;
};

#endif // PROJECTDOCK_HPP
