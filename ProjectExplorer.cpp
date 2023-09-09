#include "ProjectExplorer.hpp"

#include <QTreeView>
#include <QBoxLayout>

#include "ProjectModel.hpp"

ProjectExplorer::ProjectExplorer(ProjectModel *model, QWidget *parent)
    :
      QWidget(parent),
      m_model(model)
{
    QTreeView *view = new QTreeView(this);
    view->setModel(m_model);
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->addWidget(view);
    setLayout(main_layout);
}

ProjectModel *ProjectExplorer::getModel()
{
    return m_model;
}
