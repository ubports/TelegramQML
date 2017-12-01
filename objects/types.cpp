#include "objects/types.h"

#include <QPointer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringList>

bool MessageObject::operator==(const MessageObject *that)
{
    return this->_hash == that->_hash;
}
