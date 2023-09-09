#ifndef KM_MAINWINDOW_HPP
#define KM_MAINWINDOW_HPP

#include <QMainWindow>
#include <set>

#include "ProjectTab.hpp"

class CppCompletionModel;
class QRegularExpressionValidator;
class ProjectModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setMenu();

private slots:
    void OnCreateNewProject();
    void OnCreateNewFile();
    void OnOpenProject();
    void OnSaveCurrentFile();
    bool SaveCurrentFile(int index);
    void OnSaveAllFiles();
    void OnCloseCurrentFile();
    void OnCloseAllFiles();
    void OpenFile(const QModelIndex &index);

    void closeEditor(int index);
private:
    void OpenEditor(const QString &file_name, const QString &contents, bool enable_syntax = true);

    //
private:
    QDockWidget *m_dock;            //dock containing project tab
    ProjectModel *m_project;        //model of the project
    ProjectTab *m_project_tab;      //tab consisting of project explorer, project file system
    QTabWidget *m_editor_tab;
    std::map<QString,QAction*> m_actions;
    std::map<QString,QMenu*> m_menus;
    CppCompletionModel *m_completion_model;
    QStringList m_source_ext;
    QStringList m_header_ext;
};

#endif // KM_MAINWINDOW_HPP
