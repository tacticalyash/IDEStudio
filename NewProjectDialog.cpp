#include "NewProjectDialog.hpp"
#include <QHBoxLayout>
#include <QLabel>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QStackedWidget>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

enum DefaultFilesFlag{
    LANGUAGE_CPP = 0x01,
    LANGUAGE_C = 0x02
};

NewProjectDialog::NewProjectDialog(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *main_layout = new QVBoxLayout();
    setWindowTitle("Create New Project");

    QFormLayout *form_layout = new QFormLayout();
    main_layout->addLayout(form_layout);

    m_project_name = new QLineEdit(this);
    m_project_name->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z][\\w]+"),this));
    form_layout->addRow("Project Name",m_project_name);

    QHBoxLayout *project_path_layout = new QHBoxLayout();
    m_project_path = new QLineEdit(this);
    m_project_path->setText(QDir::homePath());
    QPushButton *brows_btn = new QPushButton("Brows",this);
    connect(brows_btn,&QAbstractButton::clicked,this,&NewProjectDialog::browsPath);
    project_path_layout->addWidget(m_project_path);
    project_path_layout->addWidget(brows_btn);
    form_layout->addRow("Path", project_path_layout);


    QVBoxLayout *language_layout = new QVBoxLayout();
    m_language_c = new QRadioButton("C",this);
    m_language_cpp = new QRadioButton("C++",this);
    language_layout->addWidget(m_language_c);
    language_layout->addWidget(m_language_cpp);
    m_language_cpp->click();        //default
    connect(m_language_cpp,&QRadioButton::toggled,this,&NewProjectDialog::onLanguageChanged);
    form_layout->addRow("Language",language_layout);

    m_project_types = new QComboBox(this);
    m_project_types->insertItems(0,{
                                "Console Project",
                                "Shared Library",
                                "Static Library"
                               });
    connect(m_project_types, &QComboBox::currentTextChanged, this, &NewProjectDialog::onProjectTypeChanged);
    form_layout->addRow("Project Type", m_project_types);

    m_main_file = new QCheckBox("main.cpp",this);
    m_main_file->setToolTip("Add main.cpp to the project!");
    form_layout->addWidget(m_main_file);

    m_message = new QLabel(this);
    m_message->setStyleSheet("color:red;");
    main_layout->addWidget(m_message);

    QPushButton *btn_create = new QPushButton("Create",this);
    connect(btn_create,&QPushButton::clicked,this,&NewProjectDialog::onOkButtonClicked);
    QPushButton *btn_cancle = new QPushButton("Cancel",this);
    connect(btn_cancle,&QPushButton::clicked,this,&NewProjectDialog::reject);
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addWidget(btn_create);
    button_layout->addWidget(btn_cancle);
    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}





NewProjectDialog::~NewProjectDialog()
{
    //
}

void NewProjectDialog::browsPath()
{
    QString path = QFileDialog::getExistingDirectory(this,"Choose project directory",QDir::homePath());
    if(!path.isEmpty())
        m_project_path->setText(path);
}

void NewProjectDialog::onOkButtonClicked()
{
    struct {
        void operator()()
        {
            label->setText(QString());
        }
        QLabel *label;
    } _reset_message = {m_message};
    if(m_project_name->text().isEmpty())
    {
        m_message->setText(QStringLiteral("Project name can't be empty!"));
        QTimer::singleShot(3000,this,_reset_message);
        return;
    }
    else if(m_project_path->text().isEmpty() || !QDir(m_project_path->text()).exists())
    {
        m_message->setText(QStringLiteral("Given path is not valid!"));
        QTimer::singleShot(3000,this,_reset_message);
        return;
    }

    /**
     *
     * Create project directory.
     * so if project_path is /home/username/projects
     * and project_name is sample then final path will
     * be /home/username/projects/sample
     * (note : this full path will be supplied to ProjectDetailsPath
     * so project files will look like
     * /home/username/projects/sample/sample.pro
     */
    if(!QDir(m_project_path->text()).mkdir(m_project_name->text()))
    {
        QMessageBox::critical(this,"Project Error",QString("Could not make directory %1!").arg(m_details.path));
        return;
    }

    /*[1]*/m_details.name = m_project_name->text();
    /*[2]*/m_details.path = m_project_path->text() + '/';  //path + '/' + projectName
        m_details.path += m_details.name;

    /*[3]*/m_details.language = (m_language_c->isChecked())?"C":"C++";
    /*[4]*/m_details.type = ProjectDetails::Type(m_project_types->currentIndex());

    if(m_main_file->isEnabled() && m_main_file->isChecked())
    {
        //will add some more templates later so used it as a vector.
        m_default_files.push_back(m_main_file->text());
    }
    accept();
}

void NewProjectDialog::onLanguageChanged()
{
    if(m_project_types->currentIndex()) //if current project type is not console
        return;
    if(m_language_cpp->isChecked())
        m_main_file->setText("main.cpp");
    else
        m_main_file->setText("main.c");
    m_main_file->setToolTip(QString("Add %1 to the project!").arg(m_main_file->text()));
}

void NewProjectDialog::onProjectTypeChanged(const QString &text)
{
    m_main_file->setEnabled(text == "Console Project");
}
