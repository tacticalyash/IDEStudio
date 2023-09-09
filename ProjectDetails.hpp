#ifndef PROJECTDETAILS_HPP
#define PROJECTDETAILS_HPP

#include <QString>

struct ProjectDetails{
    enum Type{
        CONSOLE,
        STATIC_LIB,
        SHARED_LIB
    }type;
    QString typeAsString() const{
        QString strings[] = {"console","static library","dynamic library"};
        return strings[type];
    }
    QString name;
    QString path;
    QString language;
};



#endif // PROJECTDETAILS_HPP
