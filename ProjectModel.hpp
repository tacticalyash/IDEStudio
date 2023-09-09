#ifndef PROJECTMODEL_HPP
#define PROJECTMODEL_HPP

#include "ProjectDetails.hpp"

#include <QAbstractItemModel>
#include <QProcess>



enum class ProjectItemType
{
    PROJECT_NAME = 0x0001,
    SOURCE_LABEL = 0x0002,
    HEADER_LABEL = 0x0004,
    OTHER_LABEL = 0x0008,
    SOURCE_FILE = 0x0010,
    HEADER_FILE = 0x0020,
    OTHER_FILE = 0x0040
};

inline constexpr ProjectItemType operator | (ProjectItemType type1, ProjectItemType type2)
{
    return static_cast<ProjectItemType>(static_cast<int>(type1)|static_cast<int>(type2));
}

inline constexpr int operator & (ProjectItemType type1, ProjectItemType type2)
{
    return static_cast<int>(type1) & static_cast<int>(type2);
}


class ProjectItem
{
public:
    ProjectItem *parentItem();
    void addChild(ProjectItem *child);
    const std::vector<ProjectItem *> &childrenList() const;
    ProjectItem *childAt(int row);
    const ProjectItem *childAt(int row) const;
    int childCount() const;
    const QString &data() const;
    ProjectItemType itemType() const;
    int row() const;

    bool isSourceFile() const;
    bool isHeaderFile() const;
    bool isOtherFile() const;

    bool isFile() const;
    ~ProjectItem();
private:
    ProjectItem(const QString &label, ProjectItemType type, ProjectItem *parent = nullptr);
private:
    QString m_label;
    ProjectItemType m_type;
    std::vector<ProjectItem*> m_children;
    ProjectItem *m_parent;
    //the only class to access its constructor.
    friend class ProjectModel;
};


inline ProjectItem *ProjectItem::parentItem()
{
    return m_parent;
}

inline void ProjectItem::addChild(ProjectItem *child)
{
    m_children.push_back(child);
}

inline const std::vector<ProjectItem *> &ProjectItem::childrenList() const
{
    return m_children;
}

inline ProjectItem *ProjectItem::childAt(int row)
{
    if(row < 0 || static_cast<unsigned int>(row) >= m_children.size())
        return nullptr;
    return m_children[row];
}

inline const ProjectItem *ProjectItem::childAt(int row) const
{
    if(row < 0 || static_cast<unsigned int>(row) >= m_children.size())
        return nullptr;
    return m_children[row];
}

inline int ProjectItem::childCount() const
{
    return m_children.size();
}

inline const QString &ProjectItem::data() const
{
    return m_label;
}

inline ProjectItemType ProjectItem::itemType() const
{
    return m_type;
}

inline bool ProjectItem::isSourceFile() const
{
    return itemType() & ProjectItemType::SOURCE_FILE;
}

inline bool ProjectItem::isHeaderFile() const
{
    return itemType() & ProjectItemType::HEADER_FILE;
}

inline bool ProjectItem::isOtherFile() const
{
    return itemType() & ProjectItemType::OTHER_FILE;
}

inline bool ProjectItem::isFile() const
{
    return itemType() & (
                ProjectItemType::SOURCE_FILE
                | ProjectItemType::HEADER_FILE
                | ProjectItemType::OTHER_FILE
                );
}



/**
 * fill file with template text.
 */
enum class FileTemplateHint
{
    NONE = 0x01,
    MAIN = 0x02,
    HEADER = 0x04,
    HEADER_PFX = 0x8,
    LIB_HEADER = 0x10,
    LIB_HEADER_PFX = 0x20,
};

class ProjectModel : public QAbstractItemModel
{
public:
    enum UserRole
    {
        FileRole = Qt::UserRole
    };
    Q_OBJECT
public:
    ProjectModel(const ProjectDetails &project, QObject *parent = nullptr);

    QString getProjectName() const;


    /**
     * returns a pair of file creation status and it's contents
     * returns  0 if everything is fine.
     *          1 if template file could not be opened (safe to ignore, empty file will be added instead)
     *          -1 if file could not be created (hence it won't be added)
     *          -2 if file already exists (obiviously won't be added).
    */
    std::pair<int,QString> addSourceFile(const QString &file_name, FileTemplateHint hint = FileTemplateHint::NONE);
    std::pair<int,QString> addHeaderFile(const QString &file_name, FileTemplateHint hint = FileTemplateHint::NONE, QString prefix = QString());
    std::pair<int,QString> addOtherFile(const QString &file_name);

    /**
     * read contents of file pointed by index
     * returns a pair of file name and contents of the file (if succeeded).
     * returns file name and empty string and ok is set to false if file couldn't be read.
     * returns empty strings if index is invalid or doesn't point to a file.
     */
    std::pair<QString,QString> getFileContents(const QModelIndex &index, bool &ok);

    /**
     * these functions will be used by treeview.
     */
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * process related functions
     */
    QString readStdOutput();
    QString readStdError();
    /**
     * get project details
     */
    const ProjectDetails &getProjectDetails() const;

    /**
     * additional functions
     */
    QModelIndex indexForFile(const QString &file_name);
    ~ProjectModel();

private:
    std::pair<int,QString> addFile(ProjectItem *parent, ProjectItemType file_type, const QString &file_name, int row, FileTemplateHint hint, const QString &prefix = QString());
    void createCMake();
public slots:
    void buildProject();
    void cleanProject();
    void rebuildProject();
    void buildCMake();
signals:
    void readyCMakeStdOut();
    void readyCMakeStdErr();
    void readyMakeStdOut();
    void readyMakeStdErr();
    void finished(int exit_code, QProcess::ExitStatus exit_status, const QString &errMsg);
private:
    ProjectDetails m_project;
    ProjectItem *m_root;
    ProjectItem *m_sources;
    ProjectItem *m_headers;
    ProjectItem *m_others;
    QProcess *m_process;
    QString m_cmake;            //complete cmake
    bool m_process_running;            //project status
    bool m_cmake_build_scheduled;       //if process was busy then schedul it for later
};

inline QString ProjectModel::readStdOutput()
{
    return m_process->readAllStandardOutput();
}

inline QString ProjectModel::readStdError()
{
    return m_process->readAllStandardError();
}

inline const ProjectDetails &ProjectModel::getProjectDetails() const
{
    return m_project;
}

inline void ProjectModel::rebuildProject()
{
    cleanProject();
    buildProject();
}

#endif // PROJECTMODEL_HPP
