#include "MainWindow.hpp"

#include <QMenuBar>
#include <QAction>
#include <QStatusBar>
#include <QFile>
#include <QDockWidget>
#include <QMessageBox>

#include "TextEditor.hpp"
#include "CppCompletionModel.hpp"
#include "ProjectModel.hpp"
#include "NewProjectDialog.hpp"
#include "NewFileDialog.hpp"

static void logFileCreationMessage(QWidget *parent, int code, const QString &file_name);

MainWindow::MainWindow(QWidget *parent)
    :
      QMainWindow(parent),
      m_project(nullptr),
      m_editor_tab(new QTabWidget(this)),
      m_completion_model(new CppCompletionModel(this))
{
    m_editor_tab->setMovable(true);
    m_editor_tab->setTabsClosable(true);
    m_editor_tab->setTabShape(QTabWidget::Triangular);
    connect(m_editor_tab,&QTabWidget::tabCloseRequested,this,&MainWindow::closeEditor);
    setCentralWidget(m_editor_tab);
    
    m_project_tab = new ProjectTab(this);
    m_dock = new QDockWidget(this);
    m_dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    m_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dock->setWidget(m_project_tab);
    addDockWidget(Qt::LeftDockWidgetArea,m_dock);
    //

    setMenu();
}

/**
 * Some actions will be disabled that depends on opened files and project.
 * For that they will be inserted into m_menus and m_actions.
 */
void MainWindow::setMenu()
{
    menuBar()->setNativeMenuBar(true);

    //file menu
    QMenu *menu_file = menuBar()->addMenu("&File");
    m_menus.insert({"File",menu_file});

    //new project
    QAction *action_new_project = menu_file->addAction("&New Project");
    connect(action_new_project,&QAction::triggered, this, &MainWindow::OnCreateNewProject);
    m_actions.insert({"New Project",action_new_project});

    //new project
    QMenu *submenu_new_file = menu_file->addMenu("&New File");
    submenu_new_file->setEnabled(false);
    m_menus.insert({"New File",submenu_new_file});

    //
    QAction *action_new_file_cpp = submenu_new_file->addAction("Source File");
    connect(action_new_file_cpp,&QAction::triggered,this,&MainWindow::OnCreateNewFile);
    QAction *action_new_file_hpp = submenu_new_file->addAction("Header File");
    connect(action_new_file_hpp,&QAction::triggered,this,&MainWindow::OnCreateNewFile);
    QAction *action_new_file_other = submenu_new_file->addAction("Other File");
    connect(action_new_file_other,&QAction::triggered,this,&MainWindow::OnCreateNewFile);

    QAction *action_save = menu_file->addAction("&Save &File");
    action_save->setShortcut(QKeySequence::Save);
    connect(action_save,&QAction::triggered,this,&MainWindow::OnSaveCurrentFile);
    action_save->setEnabled(false);
    m_actions.insert({"Save File",action_save});

    QAction *action_save_all = menu_file->addAction("&Save &All &Files");
    connect(action_save_all,&QAction::triggered,this,&MainWindow::OnSaveAllFiles);
    action_save_all->setEnabled(false);
    m_actions.insert({"Save All Files",action_save_all});

    QAction *action_close = menu_file->addAction("&Close &File");
    action_close->setShortcut(QKeySequence::Close);
    connect(action_close,&QAction::triggered,this,&MainWindow::OnCloseCurrentFile);
    action_close->setEnabled(false);
    m_actions.insert({"Close File",action_close});

    QAction *action_close_all = menu_file->addAction("&Close &All &Files");
    action_close_all->setEnabled(false);
    connect(action_close_all,&QAction::triggered,this,&MainWindow::OnCloseAllFiles);
    m_actions.insert({"Close All Files",action_close_all});

    QMenu *menu_edit = menuBar()->addMenu("&Edit");
    menu_edit->setEnabled(false);
    m_menus.insert({"Edit",menu_edit});

    QAction *action_copy = menu_edit->addAction("&Copy");
    action_copy->setShortcut(QKeySequence::Copy);
    m_actions.insert({"Copy",action_copy});

    QAction *action_cut = menu_edit->addAction("&Cut");
    action_cut->setShortcut(QKeySequence::Cut);
    m_actions.insert({"Cut",action_cut});

    QAction *action_paste = menu_edit->addAction("&Paste");
    action_paste->setShortcut(QKeySequence::Paste);
    m_actions.insert({"Paste",action_paste});

    QAction *action_zoom_in = menu_edit->addAction("&Zoom In");
    action_zoom_in->setShortcut(QKeySequence::ZoomIn);
    m_actions.insert({"Zoom In",action_zoom_in});

    QAction *action_zoom_out = menu_edit->addAction("&Zoom Out");
    action_zoom_out->setShortcut(QKeySequence::ZoomOut);
    m_actions.insert({"Zoom Out",action_zoom_out});

    QMenu *menu_project = menuBar()->addMenu("&Project");
    menu_project->setEnabled(false);
    m_menus.insert({"Project",menu_project});

    QAction *action_build = menu_project->addAction("&Build");
    m_actions.insert({"Build",action_build});

    QAction *action_clean = menu_project->addAction("&Clean");
    m_actions.insert({"Clean",action_clean});

    QAction *action_rebuild = menu_project->addAction("&Rebuild");
    m_actions.insert({"Rebuild",action_rebuild});

    //
}

void MainWindow::OnSaveCurrentFile()
{
    if(m_editor_tab->count())
    {
        SaveCurrentFile(m_editor_tab->currentIndex());
    }
}

bool MainWindow::SaveCurrentFile(int index)
{
    TextEditor *editor = qobject_cast<TextEditor*>(m_editor_tab->widget(index));
    if(!editor->isSaved()){
        editor->markSaved();
        QFile file(editor->getFileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this,"Error",QString("Error while saving file %1").arg(editor->getFileName()));
            return false;
        }
        QTextStream stream(&file);
        stream << editor->toPlainText() << Qt::endl;
        file.close();
    }
    return true;
}

void MainWindow::OnSaveAllFiles()
{
    for(int i = 0; i < m_editor_tab->count(); ++i)
        SaveCurrentFile(i);
}

void MainWindow::OnCloseCurrentFile()
{
    if(m_editor_tab->count())
    {
        closeEditor(m_editor_tab->currentIndex());
    }
}

void MainWindow::OnCloseAllFiles()
{
    while(m_editor_tab->count() > 0)
        OnCloseCurrentFile();
}

void MainWindow::OpenFile(const QModelIndex &index)
{
    auto var = m_project->data(index,ProjectModel::FileRole);
    if(!var.isValid())
        return;
    QString file_name = var.toString();
    for(int i = 0, count = m_editor_tab->count(); i < count; ++i)
    {
        if(qobject_cast<TextEditor*>(m_editor_tab->widget(i))->getFileName() == file_name)
        {
            m_editor_tab->setCurrentIndex(i);
            return;
        }
    }
    bool ok;
    std::pair<QString,QString> file = m_project->getFileContents(index,ok);
    if(!ok)                //if it could not read the file (but file exists in project system)
    {
        QMessageBox::critical(this,"Error",QString("Could not open file %1, may be it is deleted.").arg(file.first));
        return;
    }
    OpenEditor(file.first,file.second, !static_cast<ProjectItem*>(index.internalPointer())->isOtherFile());
}

void MainWindow::closeEditor(int index)
{
    if(SaveCurrentFile(index))
    {
        m_editor_tab->removeTab(index);
    }
    if(!m_editor_tab->count())
    {
        m_menus.at("Edit")->setEnabled(false);
        m_actions.at("Save File")->setEnabled(false);
        m_actions.at("Save All Files")->setEnabled(false);
        m_actions.at("Close File")->setEnabled(false);
        m_actions.at("Close All Files")->setEnabled(false);
    }
}



void MainWindow::OnOpenProject()
{
    //
}

void MainWindow::OnCreateNewProject()
{
    NewProjectDialog dialog(this);
    dialog.setModal(true);
    if(dialog.exec()){
        const ProjectDetails &p_details = dialog.getProjectDetails();
        m_project = new ProjectModel(p_details,this);
        m_project_tab->createProjectView(m_project);

        if(!dialog.getDefaultFiles().empty())
        {
            auto result = m_project->addSourceFile(dialog.getDefaultFiles().at(0),FileTemplateHint::MAIN);
            if(result.first != 0)
            {
                logFileCreationMessage(this,result.first,dialog.getDefaultFiles().at(0));
            }
            if(result.first >= 0)
                OpenEditor(QString("%1/%2").arg(p_details.path,dialog.getDefaultFiles().front()),result.second);
        }

        m_actions.at("New Project")->setEnabled(false);
        m_menus.at("New File")->setEnabled(true);
        m_menus.at("Project")->setEnabled(true);

        QAction *build = m_actions.at("Build");
        build->setText(QString("&Build &%1").arg(p_details.name));
        connect(build, &QAction::triggered, m_project, &ProjectModel::buildProject);

        QAction *clean = m_actions.at("Clean");
        clean->setText(QString("&Clean &%1").arg(p_details.name));
        connect(clean, &QAction::triggered, m_project, &ProjectModel::cleanProject);

        QAction *rebuild = m_actions.at("Rebuild");
        rebuild->setText(QString("&Rebuild &%1").arg(p_details.name));
        connect(rebuild, &QAction::triggered, m_project, &ProjectModel::rebuildProject);

        if(p_details.language == "C++")
        {
            m_source_ext = { "cpp", "CPP", "c++", "cxx", "CXX", "C"};
            m_header_ext = { "h", "hpp", "H", "hxx", "h++"};
        }
        else
        {
            m_source_ext = {"c"};
            m_header_ext = {"h"};
        }
    }
    else{
        qDebug("cancel was called!");
    }
}


void MainWindow::OpenEditor(const QString &file_name, const QString &contents, bool enable_syntax)
{
    if(!m_editor_tab->count())
    {
        //m_menus.at("Edit")->setEnabled(true);
        m_actions.at("Save File")->setEnabled(true);
        m_actions.at("Save All Files")->setEnabled(true);
        m_actions.at("Close File")->setEnabled(true);
        m_actions.at("Close All Files")->setEnabled(true);
        m_menus.at("Edit")->setEnabled(true);
    }
    TextEditor *editor = new TextEditor(m_editor_tab,enable_syntax);
    editor->setFileName(file_name);
    editor->setModel(m_completion_model);
    m_editor_tab->addTab(editor,file_name.split('/').last());
    editor->insertPlainText(contents);
}

void MainWindow::OnCreateNewFile()
{
    QAction *action = qobject_cast<QAction*>(sender());
    const ProjectDetails &p_details = m_project->getProjectDetails();
    QString file_name;
    std::pair<int,QString> result;
    bool enable_syntax = true;
    if(action->text() == "Source File")
    {
        file_name = NewFileDialog::getText(this,QString("Create New %1 Source File").arg(p_details.language),m_source_ext);
        if(file_name.isEmpty())
            return;
        result = m_project->addSourceFile(file_name);
    }
    else if(action->text() == "Header File")
    {
        file_name = NewFileDialog::getText(this,QString("Create New %1 Header File").arg(p_details.language),m_header_ext);
        if(file_name.isEmpty())
            return;
        result = m_project->addHeaderFile(file_name,FileTemplateHint::HEADER);
    }
    else if(action->text() == "Other File")
    {
        file_name = NewFileDialog::getText(this,QString("Create New File"));
        if(file_name.isEmpty())
            return;
        result = m_project->addOtherFile(file_name);
        enable_syntax = false;
    }
    else
    {
        return;
    }
    QString full_name = m_project->getProjectDetails().path + '/';
    full_name += file_name;
    if(result.first != 0)
        logFileCreationMessage(this,result.first,full_name);

    OpenEditor(full_name,result.second,enable_syntax);
}


/**
 * just to make other functions shorter.
 */
static void logFileCreationMessage(QWidget *parent, int code, const QString &file_name)
{
    switch(code)
    {
        case -1:
            QMessageBox::critical(parent,QString("Error"),QString("Errr while creating %1.").arg(file_name));
            return;
        case -2:
            QMessageBox::critical(parent,QString("Error"),QString("File %1 already exists.").arg(file_name));
            return;
        case 1:
            QMessageBox::information(parent,QString("Warning"),QString("Could not load sample contents."));
    }
}
