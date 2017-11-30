#include "objects/types.h"

#include <QPointer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringList>

bool MessageObject::operator==(const MessageObject *that)
{
    return this->_id == that->_id &&
        this->_sent == that->_sent &&
        this->_encrypted == that->_encrypted &&
        this->_upload == that->_upload &&
        this->_toId == that->_toId &&
        this->_unread == that->_unread &&
        this->_action == that->_action &&
        this->_fromId == that->_fromId &&
        this->_out == that->_out &&
        this->_date == that->_date&&
        this->_media == that->_media &&
        this->_fwdDate == that->_fwdDate &&
        this->_fwdFromId == that->_fwdFromId &&
        this->_replyToMsgId == that->_replyToMsgId &&
        this->_message == that->_message &&
        this->_classType == that->_classType &&
        this->_views == that->_views;
}
