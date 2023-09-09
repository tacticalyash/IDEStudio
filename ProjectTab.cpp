#include "ProjectTab.hpp"

#include <QTabWidget>
#include <QHBoxLayout>

#include "ProjectExplorer.hpp"

ProjectTab::ProjectTab(QWidget *parent) : QWidget(parent), m_project_explorer(nullptr)
{

    m_tab_widget = new QTabWidget(this);
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->addWidget(m_tab_widget);
    setLayout(main_layout);
}

void ProjectTab::createProjectView(ProjectModel *model)
{
    m_project_explorer = new ProjectExplorer(model,this);
    m_tab_widget->addTab(m_project_explorer,"Project Explorer");
}
