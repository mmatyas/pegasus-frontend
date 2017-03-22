#include "Api.h"

#include "Es2XmlReader.h"


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
{
    Es2XmlReader::read(m_platforms); // TODO: check result
}
