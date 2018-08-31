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

MessageObject::MessageObject(const Message & another, QObject *parent) : TqObject(parent){
    (void)another;
    _id = another.id();
    _sent = true;
    _encrypted = false;
    _upload = new UploadObject(this);
    _toId = new PeerObject(another.toId(), this);
    _unread = (another.flags() & 0x1);
    _action = new MessageActionObject(another.action(), this);
    _fromId = another.fromId();
    _out = (another.flags() & 0x2);
    _date = another.date();
    _media = new MessageMediaObject(another.media(), this);
    _fwdDate = another.fwdDate();
    _fwdFromId = new PeerObject(another.fwdFromId(), this);
    _replyToMsgId = another.replyToMsgId();
    _message = messageWithEntities(another.message(), another.entities());
    _classType = another.classType();
    _unifiedId = _id == 0 ? 0 : QmlUtils::getUnifiedMessageKey(_id, _toId->channelId());
    _views = another.views();
    _hash = another.getHash();
}

QString MessageObject::messageWithEntities(QString message, QList<MessageEntity> entities) const
{
    int sourcePosition = 0;
    QString result("");
    Q_FOREACH(const MessageEntityObject &entity, entities)
    {
    	if(entity.classType() == 0)
    		continue;
        result.append(message.mid(sourcePosition, entity.offset()));
        switch(entity.messageEntityEnum())
        {
            case MessageEntityObject::MessageEntityEnum::Bold:
                result.append("<b>");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</b>");
            break;
            case MessageEntityObject::MessageEntityEnum::Italic:
                result.append("<i>");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</i>");
            break;
            case MessageEntityObject::MessageEntityEnum::Code:
                result.append("<code>");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</code>");
            break;
            case MessageEntityObject::MessageEntityEnum::Pre:
                result.append("<pre>");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</pre>");
            break;
            case MessageEntityObject::MessageEntityEnum::Hashtag:
            case MessageEntityObject::MessageEntityEnum::Mention:
                result.append("<font color="blue">");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</font>");
            break;

        }
        sourcePosition = entity.offset() + entity.length();
    }
    result.append(message.mid(sourcePosition, message.length()));
    return result;
}
