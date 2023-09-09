#include "NewFileDialog.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

NewFileDialog::NewFileDialog(QWidget *parent, const QString &title, QStringList extensions)
    :
      QDialog(parent)
{
    setWindowTitle(title);
    QVBoxLayout *layout_main = new QVBoxLayout();

    QHBoxLayout *layout_field = new QHBoxLayout();
    m_file_name = new QLineEdit(this);
    m_file_name->setPlaceholderText("FileName");
    m_file_name->setValidator(new QRegularExpressionValidator(QRegularExpression("\\w+"),m_file_name));
    layout_field->addWidget(m_file_name);
    if(!extensions.isEmpty())
    {
        m_file_ext = new QComboBox(this);
        m_file_ext->addItems(extensions);
        layout_field->addWidget(m_file_ext);
    }
    else
    {
        m_file_ext = nullptr;
    }
    QHBoxLayout *layout_btn = new QHBoxLayout();
    QPushButton *btn_cancel = new QPushButton("Cancel",this);
    connect(btn_cancel, &QPushButton::clicked, this, &NewFileDialog::reject);
    layout_btn->addWidget(btn_cancel);
    QPushButton *btn_ok = new QPushButton("Create",this);
    connect(btn_ok, &QPushButton::clicked, this, &NewFileDialog::accept);
    layout_btn->addWidget(btn_ok);

    layout_main->addStretch();
    layout_main->addLayout(layout_field);
    layout_main->addLayout(layout_btn);
    setLayout(layout_main);
}

QString NewFileDialog::getFileName() const
{
    if(result() == QDialog::Accepted && !m_file_name->text().isEmpty())
    {
        QString final_file_name = m_file_name->text();
        if(m_file_ext)
            final_file_name.append("." + m_file_ext->currentText());
        return final_file_name;
    }
    return QString();
}

QString NewFileDialog::getText(QWidget *parent, const QString &title, QStringList extensions, QRegularExpressionValidator *validator)
{
    NewFileDialog dialog(parent,title,extensions);
    if(validator)
    {
        dialog.m_file_name->setValidator(nullptr);
        dialog.m_file_name->setValidator(validator);
    }
    dialog.setModal(true);
    dialog.exec();
    return dialog.getFileName();
}
