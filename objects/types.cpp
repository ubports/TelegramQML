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
    _message = MessageObject::messageWithEntities(another.message(), another.entities());
    _classType = another.classType();
    _unifiedId = _id == 0 ? 0 : QmlUtils::getUnifiedMessageKey(_id, _toId->channelId());
    _views = another.views();
    _hash = another.getHash();
}

//void MessageObject::entitiesFromMessage(QString message, QList<MessageEntity> &entities, QString &plainText)
//{
//    if (rxBold.isEmpty())
//    {
//        rxBold = QRegExp("\*\*(.+?)\*\*");
//        rxItalic = QRegExp("__(.+?)__");
//        rxCode = QRegExp("```(.+?)```");
//    }

//    int pos = 0;
//    auto result = QList<MessageEntity>();
//    while ((pos = rxBold.indexIn(message, pos)) != -1) {
//        MessageEntity entity(MessageEntity::MessageEntityClassType::typeMessageEntityBold);
//        entity.setLength(rxBold.matchedLength() - 4);
//        entity.setOffset(pos);
//        result << entity;
//        plainText += message.mid(pos, entity.offset());
//        plainText += rxBold.cap(1);
//        pos += rxBold.matchedLength();
//    }
//    plainText += message.mid(pos);
//}

QString MessageObject::messageWithEntities(QString message, QList<MessageEntity> entities)
{
    int sourcePosition = 0;
    QString result("");
    qWarning() << "Plaintext:" << message;
    Q_FOREACH(const MessageEntityObject &entity, entities)
    {
    	if(entity.classType() == 0)
    		continue;
        result.append(message.mid(sourcePosition, entity.offset() - sourcePosition));
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
                result.append("<font color=\"#598fe5\">");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</font>");
            break;
            case MessageEntityObject::MessageEntityEnum::Email:
                result.append("<a href=\"mailto:");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("\">");
                result.append(message.mid(entity.offset(), entity.length()));
                result.append("</a>");
            break;
            case MessageEntityObject::MessageEntityEnum::Url:
            case MessageEntityObject::MessageEntityEnum::TextUrl:
                auto url = message.mid(entity.offset(), entity.length());
                qWarning() << "entity.url:" << (entity.url().isEmpty()? "???" : entity.url());
                result.append("<a href=\"");
                result.append(entity.url().isEmpty()? url : entity.url());
                result.append("\">");
                result.append(url);
                result.append("</a>");
            break;


        }
        sourcePosition = entity.offset() + entity.length();
        qWarning() << "Message parsing:" << sourcePosition << ":" << entity.offset() << ":" << entity.length() << ":" << result;
    }
    result.append(message.mid(sourcePosition, message.length()));
    qWarning() << "Message parsing:" << sourcePosition << ":" << message.length() << ":" << result;
    return result;
}
