#include "Api.h"

#include "Es2XmlReader.h"


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
{
    Es2XmlReader::read(m_platforms); // TODO: check result
}

QQmlListProperty<Model::Platform> ApiObject::getPlatformsProp()
{
    return QQmlListProperty<Model::Platform>(this, m_platforms);
}
