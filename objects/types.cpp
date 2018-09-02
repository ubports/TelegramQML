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

void MessageObject::operator= ( const Message & another) {
    _id = another.id();
    Q_EMIT idChanged();
    _sent = true;
    Q_EMIT sentChanged();
    *_toId = another.toId();
    Q_EMIT toIdChanged();
    _unread = (another.flags() & 0x1);
    Q_EMIT unreadChanged();
    *_action = another.action();
    Q_EMIT actionChanged();
    _fromId = another.fromId();
    Q_EMIT fromIdChanged();
    _out = (another.flags() & 0x2);
    Q_EMIT outChanged();
    _date = another.date();
    Q_EMIT dateChanged();
    *_media = another.media();
    Q_EMIT mediaChanged();
    _fwdDate = another.fwdDate();
    Q_EMIT fwdDateChanged();
    *_fwdFromId = another.fwdFromId();
    Q_EMIT fwdFromIdChanged();
    _replyToMsgId = another.replyToMsgId();
    Q_EMIT replyToMsgIdChanged();
    _entities = another.entities();
    _message = another.message();
    Q_EMIT messageChanged();
    _classType = another.classType();
    Q_EMIT classTypeChanged();
    _unifiedId = _id == 0 ? 0 : QmlUtils::getUnifiedMessageKey(_id, _toId->channelId());
    Q_EMIT unifiedIdChanged();
    _views = another.views();
    Q_EMIT viewsChanged();
    _hash = another.getHash();
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
    _entities = another.entities();
    _message = another.message();
    _classType = another.classType();
    _unifiedId = _id == 0 ? 0 : QmlUtils::getUnifiedMessageKey(_id, _toId->channelId());
    _views = another.views();
    _hash = another.getHash();
}

QRegExp MessageObject::rxEntity;

void MessageObject::getEntitiesFromMessage(const QString messageText, QString &plainText, QList<MessageEntity> &entities)
{
    if (rxEntity.isEmpty())
    {
        rxEntity = QRegExp("\\*\\*.+\\*\\*|__.+__|```.+```|`.+\b");
        rxEntity.setMinimal(true);
    }
    int offsetCorrection = 0;

    int pos = 0;
    plainText = "";
    entities = QList<MessageEntity>();
    while ((pos = rxEntity.indexIn(messageText, pos)) != -1)
    {
        auto match = rxEntity.cap(0);
        if(match.startsWith("*"))
        {
            MessageEntity entity(MessageEntity::MessageEntityClassType::typeMessageEntityBold);
            entity.setLength(rxEntity.matchedLength() -4);
            entity.setOffset(pos - offsetCorrection);
            offsetCorrection += 4;
            entities << entity;
        } else if(match.startsWith("_"))
        {
            MessageEntity entity(MessageEntity::MessageEntityClassType::typeMessageEntityItalic);
            entity.setLength(rxEntity.matchedLength() -4);
            entity.setOffset(pos - offsetCorrection);
            offsetCorrection += 4;
            entities << entity;
        } else if(match.startsWith("`"))
        {
            MessageEntity entity(MessageEntity::MessageEntityClassType::typeMessageEntityCode);
            entity.setLength(rxEntity.matchedLength() -2);
            entity.setOffset(pos - offsetCorrection);
            offsetCorrection += 2;
            entities << entity;
        } else if(match.startsWith("```"))
        {
            MessageEntity entity(MessageEntity::MessageEntityClassType::typeMessageEntityPre);
            entity.setLength(rxEntity.matchedLength() -6);
            entity.setOffset(pos - offsetCorrection);
            offsetCorrection += 6;
            entities << entity;
        }
        pos += rxEntity.matchedLength();
    }

    plainText = messageText;
    plainText = plainText.replace("**", "").replace("__", "").replace("```", "");
}

QString MessageObject::getMessageWithEntities(const QString &plainText, const QList<MessageEntity> &entities)
{
    int sourcePosition = 0;

    QString result("");
    Q_FOREACH(const MessageEntityObject &entity, entities)
    {
        if(entity.classType() == 0)
            continue;
        result.append(plainText.mid(sourcePosition, entity.offset() - sourcePosition));
        switch(entity.messageEntityEnum())
        {
            case MessageEntityObject::MessageEntityEnum::Bold:
                result.append("<b>");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("</b>");
            break;
            case MessageEntityObject::MessageEntityEnum::Italic:
                result.append("<i>");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("</i>");
            break;
            case MessageEntityObject::MessageEntityEnum::Code:
                result.append("<code>");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("</code>");
            break;
            case MessageEntityObject::MessageEntityEnum::Pre:
                result.append("<pre>");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("</pre>");
            break;
            case MessageEntityObject::MessageEntityEnum::Hashtag:
            case MessageEntityObject::MessageEntityEnum::Mention:
                result.append("<font color=\"#598fe5\">");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("</font>");
            break;
            case MessageEntityObject::MessageEntityEnum::Email:
                result.append("<a href=\"mailto:");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("\">");
                result.append(plainText.mid(entity.offset(), entity.length()));
                result.append("</a>");
            break;
            case MessageEntityObject::MessageEntityEnum::Url:
            case MessageEntityObject::MessageEntityEnum::TextUrl:
                auto url = plainText.mid(entity.offset(), entity.length());
                result.append("<a href=\"");
                result.append(entity.url().isEmpty()? url : entity.url());
                result.append("\">");
                result.append(url);
                result.append("</a>");
            break;
        }
        sourcePosition = entity.offset() + entity.length();
    }
    result.append(plainText.mid(sourcePosition, plainText.length()));
    return result;
}

