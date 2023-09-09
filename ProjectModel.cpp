#include <fstream>

#include "ProjectModel.hpp"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QProcess>

#include "KMX/kmx.hpp"

//addFile function will use this function
static std::pair<int,QString> createFileFromTemplate(const QString &file_name, const QString &path, FileTemplateHint hint, const QString &prefix);

constexpr inline FileTemplateHint operator | (FileTemplateHint a, FileTemplateHint b)
{
    return static_cast<FileTemplateHint>(static_cast<int>(a)|static_cast<int>(b));
}
constexpr inline int operator & (FileTemplateHint a, FileTemplateHint b)
{
    return static_cast<int>(a) & static_cast<int>(b);
}

inline ProjectItem::ProjectItem(const QString &label, ProjectItemType type, ProjectItem *parent)
    :
      m_label(label),
      m_type(type),
      m_parent(parent)
{
    //
}


int ProjectItem::row() const
{
    if(m_parent)
    {
        for(int i_ = 0; i_ < m_parent->childCount(); ++i_)
            if(m_parent->childAt(i_) == this)
                return i_;
    }
    return 0;
}

ProjectItem::~ProjectItem()
{
    for(ProjectItem *item : m_children)
    {
        delete item;
    }
}



ProjectModel::ProjectModel(const ProjectDetails &project, QObject *parent)
    :
      QAbstractItemModel(parent),
      m_project(project),
      m_root(new ProjectItem(project.name,ProjectItemType::PROJECT_NAME)),
      m_sources(new ProjectItem("Sources",ProjectItemType::SOURCE_LABEL,m_root)),
      m_headers(new ProjectItem("Headers",ProjectItemType::HEADER_LABEL,m_root)),
      m_others(new ProjectItem("Others",ProjectItemType::OTHER_LABEL,m_root)),
      m_process(new QProcess()),
      m_process_running(false),
      m_cmake_build_scheduled(false)
{
    m_root->addChild(m_sources);
    m_root->addChild(m_headers);
    m_root->addChild(m_others);
    m_process->setParent(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exit_code, QProcess::ExitStatus exit_status){
        m_process_running = false;
        emit finished(exit_code,exit_status,m_process->errorString());
        if(m_cmake_build_scheduled){
            m_cmake_build_scheduled = false;
            buildCMake();
        }
    });
    createCMake();
}



std::pair<int,QString> ProjectModel::addSourceFile(const QString &file_name, FileTemplateHint hint)
{
    //0 for sources
    if(!(hint & FileTemplateHint::MAIN))
        hint = FileTemplateHint::NONE;
    return addFile(m_sources,ProjectItemType::SOURCE_FILE,file_name,0,hint);
}

std::pair<int,QString> ProjectModel::addHeaderFile(const QString &file_name, FileTemplateHint hint, QString prefix)
{
    using FTH = FileTemplateHint;
    if(hint & (FTH::HEADER_PFX | FTH::LIB_HEADER_PFX) && prefix.isEmpty())
        prefix = m_project.name;
    if(!(hint & (FTH::HEADER | FTH::HEADER_PFX | FTH::LIB_HEADER | FTH::LIB_HEADER_PFX)))
        hint = FTH::NONE;
    return addFile(m_headers,ProjectItemType::HEADER_FILE,file_name,1,hint,prefix);
}

std::pair<int,QString> ProjectModel::addOtherFile(const QString &file_name)
{
    //2 for others
    return addFile(m_others,ProjectItemType::OTHER_FILE,file_name,2,FileTemplateHint::NONE);
}

std::pair<QString,QString> ProjectModel::getFileContents(const QModelIndex &index, bool &ok)
{
    QVariant var = data(index,FileRole);
    if(!var.isValid()){
        ok = false;
        return {};
    }
    QString file_name = var.toString();
    QFile file(file_name);
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        ok = false;
        return {file_name,QString()};
    }
    QString contents = file.readAll();
    file.close();
    ok = true;
    return {file_name,contents};
}


ProjectModel::~ProjectModel()
{
    std::string file_name = QString("%1/%2.pro").arg(m_project.path, m_project.name).toStdString();
    std::ofstream file(file_name);
    if(file.is_open())
    {
        KMXTag project_root("project");

        KMXTag *source = new KMXTag("source");
        KMXTag *header = new KMXTag("header");
        KMXTag *other = new KMXTag("other");

        for(ProjectItem *item : m_sources->childrenList())
        {
            KMXTag *tag = new KMXTag("file");
            tag->setTagType(KMXTag::Leaf);
            tag->insertProperty({"name",item->data().toStdString()});
            source->addChildTag(tag);
        }
        for(ProjectItem *item : m_headers->childrenList())
        {
            KMXTag *tag = new KMXTag("file");
            tag->setTagType(KMXTag::Leaf);
            tag->insertProperty({"name",item->data().toStdString()});
            header->addChildTag(tag);
        }
        for(ProjectItem *item : m_others->childrenList())
        {
            KMXTag *tag = new KMXTag("file");
            tag->setTagType(KMXTag::Leaf);
            tag->insertProperty({"name",item->data().toStdString()});
            other->addChildTag(tag);
        }
        project_root.insertProperty({"name",m_project.name.toStdString()});
        project_root.insertProperty({"language",m_project.language.toStdString()});
        project_root.insertProperty({"type",m_project.typeAsString().toStdString()});

        project_root.addChildTag(source);
        project_root.addChildTag(header);
        project_root.addChildTag(other);

        project_root.writeTo(file,0,2);
        file.close();
    }
    else
    {
        QMessageBox::critical(nullptr,"Error","Can't save project file!");
    }
    delete m_root;
}

std::pair<int,QString> ProjectModel::addFile(ProjectItem *parent, ProjectItemType file_type, const QString &file_name, int row, FileTemplateHint hint, const QString &prefix)
{
    const std::vector<ProjectItem*> &file_list = parent->childrenList();

    auto it = file_list.cbegin();
    while(it != file_list.cend() && (*it)->data() < file_name)
    {
        ++it;
    }
    if(it == file_list.cend() || (*it)->data() != file_name)
    {
        auto file_created = createFileFromTemplate(file_name,m_project.path,hint,prefix);
        if(file_created.first != -1)
        {
            int file_index = std::distance(file_list.cbegin(),it);
            beginInsertRows(index(row,0,QModelIndex()),file_index,file_index);
            parent->addChild(new ProjectItem(file_name,file_type,parent));
            endInsertRows();
            if(m_process_running)
                m_cmake_build_scheduled = true;
            else
                buildCMake();
        }
        return file_created;
    }

    return {-2,QString()};
}


QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row,column,parent))
        return QModelIndex();
    ProjectItem *parent_item;
    if(!parent.isValid())
        parent_item = m_root;
    else
        parent_item = static_cast<ProjectItem*>(parent.internalPointer());
    ProjectItem *child_item = parent_item->childAt(row);
    if (child_item)
        return createIndex(row, column, child_item);
    return QModelIndex();
}

QModelIndex ProjectModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    ProjectItem *child_item = static_cast<ProjectItem*>(child.internalPointer());
    ProjectItem *parent_item = child_item->parentItem();

    if (parent_item == m_root)
        return QModelIndex();

    return createIndex(parent_item->row(), 0, parent_item);
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
    ProjectItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<ProjectItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int ProjectModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QVariant();
    ProjectItem *item = static_cast<ProjectItem*>(index.internalPointer());
    if ( role == Qt::DisplayRole)
        return item->data();
    else if( role == FileRole && (item->isFile()))
        return QString("%1/%2").arg(m_project.path,item->data());
    return QVariant();
}

QVariant ProjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_root->data();
    return QVariant();
}

QModelIndex ProjectModel::indexForFile(const QString &file_name)
{
    ProjectItem *items[3] = {m_sources,m_headers,m_others};
    for(unsigned i = 0; i < 3; ++i)
    {
        unsigned size = items[i]->childCount();
        for(unsigned j = 0; j < size; ++j)
        {
            if(items[i]->childAt(j)->data() == file_name)
            {
                return createIndex(j,0,items[i]->childAt(j));
            }
        }
    }
    return QModelIndex();
}


void ProjectModel::createCMake()
{
    QFile file(m_project.path + "/CMakeLists.txt");
    if(!file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        QMessageBox::critical(nullptr,"Can't create CMakeLists.txt",QString("error %1").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);

    stream << "project( " << m_project.name << " LANGUAGES " << ((m_project.language == "C++")?("CXX"):("C")) << ")\n";
    stream << "add_compile_options(-Wall -Wextra -Wpedantic)\n";

    stream << "set(\n\tKM_SOURCES #source files";

    const std::vector<ProjectItem*> &sources = m_sources->childrenList();
    for(ProjectItem *item : sources)
    {
        stream << "\n\t"<< item->data() ;
    }
    stream << "\n)\nset(\n\tKM_HEADERS #header files";
    const std::vector<ProjectItem*> &headers = m_headers->childrenList();
    for(ProjectItem *item : headers)
    {
        stream << "\n\t" << item->data();
    }
    stream << "\n)\nset(\n\tKM_OTHERS #other files";
    const std::vector<ProjectItem*> &others = m_others->childrenList();
    for(ProjectItem *item : others)
    {
        stream << "\n\t" << item->data();
    }

    switch(m_project.type)
    {
        case ProjectDetails::CONSOLE:
            stream  << "\n)\nadd_executable( ${PROJECT_NAME}  #binary\n";
            break;
        case ProjectDetails::STATIC_LIB:
            stream << "\n)\nadd_library( ${PROJECT_NAME} STATIC #static library\n";
            break;
        case ProjectDetails::SHARED_LIB:
            stream << "\n)\nadd_library( ${PROJECT_NAME} SHARED #shared library\n";
            break;
    }
    stream << "\t${KM_SOURCES} #include source files\n";
    stream << "\t${KM_HEADERS} #include header files\n";
    stream << "\n)";
    stream.flush();
    file.close();
}

/************************Build Related Functions*********************************/

void ProjectModel::buildCMake()
{
    createCMake();
    if(m_process->program() != "cmake")
    {
        disconnect(m_process,&QProcess::readyReadStandardOutput,nullptr,nullptr);
        disconnect(m_process,&QProcess::readyReadStandardError,nullptr,nullptr);
        connect(m_process,&QProcess::readyReadStandardOutput, this, &ProjectModel::readyCMakeStdOut);
        connect(m_process,&QProcess::readyReadStandardError, this, &ProjectModel::readyCMakeStdErr);
        m_process->setProgram("cmake");
        m_process->setWorkingDirectory(m_project.path);
    }
    m_process->setArguments({"-S",".","-B","build"});
    m_process->start();
    m_process_running = true;
    m_cmake_build_scheduled = false;    //if cmake build was scheduled then reset it.
}

void ProjectModel::buildProject()
{
    if(m_process_running)
        return;
    if(m_process->program() != "make")
    {
        disconnect(m_process,&QProcess::readyReadStandardOutput,nullptr,nullptr);
        disconnect(m_process,&QProcess::readyReadStandardError,nullptr,nullptr);
        connect(m_process,&QProcess::readyReadStandardOutput, this, &ProjectModel::readyMakeStdOut);
        connect(m_process,&QProcess::readyReadStandardError, this, &ProjectModel::readyMakeStdErr);
        m_process->setProgram("make");
        m_process->setWorkingDirectory(m_project.path + "/build");
    }
    m_process->start();
    m_process_running = true;
}

void ProjectModel::cleanProject()
{
    if(m_process_running)
        return;
    if(m_process->program() != "make")
    {
        disconnect(m_process,&QProcess::readyReadStandardOutput,nullptr,nullptr);
        disconnect(m_process,&QProcess::readyReadStandardError,nullptr,nullptr);
        connect(m_process,&QProcess::readyReadStandardOutput, this, &ProjectModel::readyMakeStdOut);
        connect(m_process,&QProcess::readyReadStandardError, this, &ProjectModel::readyMakeStdErr);
        m_process->setProgram("make");
        m_process->setWorkingDirectory(m_project.path + "/build");
    }
    m_process->setArguments({"clean"});
    m_process->start();
    m_process_running = true;
}

/**********************Non member but helper functions***************************/
static inline QString prepareHeaderName(QString header_file_name)
{
    header_file_name.replace('.','_');
    return header_file_name.toUpper();
}

static bool loadTemplate(QString &contents,QString file_name, FileTemplateHint hint)
{
    QFile file;
    using FTH = FileTemplateHint;
    switch(hint)
    {
        case FTH::MAIN:
            if(file_name.endsWith(".c"))
                file.setFileName("templates/main.c.txt");
            else
                file.setFileName("templates/main.cpp.txt");
            break;
        case FTH::HEADER:
        case FTH::HEADER_PFX:
            file.setFileName("templates/header.h.txt");
            break;
        case FTH::LIB_HEADER:
        case FTH::LIB_HEADER_PFX:
            file.setFileName("templates/global_settings.h.txt");
            break;
        default:
            contents.clear();
            return true;
    }
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly))
        return false;
    contents = file.readAll();
    file.close();
    return true;
}

static std::pair<int,QString> createFileFromTemplate(const QString &file_name, const QString &path, FileTemplateHint hint, const QString &prefix)
{
    QString full_path = path + '/';
    full_path += file_name;

    QFile output_file(full_path);

    if(!output_file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        return {-1,QString()};
    }

    QString contents;
    if(!loadTemplate(contents,file_name,hint))
    {
        output_file.close();
        return {1,QString()};
    }
    using FTH = FileTemplateHint;
    QString header_macro;      //strings like #ifndef MY_HEADER_HPP
    switch(hint)
    {
        case FTH::HEADER_PFX:
            header_macro = prefix + '_'; //add prefix
        case FTH::HEADER:
            header_macro += file_name; //add file name
            header_macro = prepareHeaderName(header_macro);
            contents.replace("${HEADER_H}",header_macro);
            break;
        default:
            break;
    }
    QTextStream stream(&output_file);
    stream << contents;
    stream.flush();
    output_file.close();
    return {0,contents};
}
