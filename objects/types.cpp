#include "objects/types.h"

#include <QPointer>
#include <QScreen>
#include <QtPrintSupport/QPrinter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QTextCharFormat>
#include <QtQml/QQmlEngine>

bool MessageObject::operator==(const MessageObject &that)
{
    auto result = this->_hash.length() == that._hash.length() &&
            this->_hash == (QString)that._hash;
    return result;
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
    _editDate = another.editDate();
    Q_EMIT editDateChanged();
    *_media = another.media();
    Q_EMIT mediaChanged();
    _fwdDate = another.fwdFrom().date();
    Q_EMIT fwdDateChanged();
    _fwdFromId = another.fwdFrom().fromId();
    Q_EMIT fwdFromIdChanged();
    _replyToMsgId = another.replyToMsgId();
    Q_EMIT replyToMsgIdChanged();
    _entities = another.entities();
    _message = another.message();
    messageDocument(_msgDocument);
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
    _action = new MessageActionObject(another.action(), this);
    _fromId = another.fromId();
    _out = another.out();
    _date = another.date();
    _editDate = another.editDate();
    _media = new MessageMediaObject(another.media(), this);
    _fwdDate = another.fwdFrom().date();
    _fwdFromId = another.fwdFrom().fromId();
    _replyToMsgId = another.replyToMsgId();
    _entities = another.entities();
    _message = another.message().isEmpty()? _media->caption() : another.message();
    _msgDocument = new QTextDocument();
    //QPrinter printer(QPrinter::HighResolution);
    //printer.setResolution(QScreen::physicalDotsPerInch());
    //_msgDocument->documentLayout()->setPaintDevice(printer);
    messageDocument(_msgDocument);
    _classType = another.classType();
    _unifiedId = _id == 0 ? 0 : QmlUtils::getUnifiedMessageKey(_id, _toId->channelId());
    _views = another.views();
    _hash = another.getHash();
}

QRegExp MessageObject::rxEntity;
QRegExp MessageObject::rxLinebreaks;
QColor MessageObject::linkColor;
QColor MessageObject::codeColor;

void MessageObject::getEntitiesFromMessage(const QString messageText, QString &plainText, QList<MessageEntity> &entities)
{
    if (rxEntity.isEmpty())
    {
        rxEntity = QRegExp("\\*\\*.+\\*\\*|__.+__|```.+```|`.+`");
        rxEntity.setMinimal(true);
        rxLinebreaks = QRegExp("\\n|\\r");

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
        } else if(match.startsWith("`") && !match.startsWith("``"))
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
    plainText = plainText.replace("**", "").replace("__", "").replace("`", "");
}

void MessageObject::messageDocument(QTextDocument *result)
{

    if(!codeColor.isValid())
    {
        linkColor.setNamedColor("#207982");
        codeColor.setNamedColor("#997327");
    }
    result->clear();
    if(_message.length() > 0)
    {
        result->setPlainText(_message);
        Q_FOREACH(const MessageEntity &entity, _entities)
        {
            QTextCursor cursor{result};
            cursor.setPosition(entity.offset(), QTextCursor::MoveAnchor);
            cursor.setPosition(entity.offset() + entity.length(), QTextCursor::KeepAnchor);
            const QString subText = _message.mid(entity.offset(), entity.length());
            switch(entity.classType()) {
            case MessageEntity::MessageEntityClassType::typeMessageEntityBold:
            {
                QTextCharFormat format;
                format.setFontWeight(60);
                cursor.mergeCharFormat(format);
                break;
            }
            case MessageEntity::MessageEntityClassType::typeMessageEntityBotCommand:
                break;
            case MessageEntity::MessageEntityClassType::typeMessageEntityCode:
            case MessageEntity::MessageEntityClassType::typeMessageEntityPre:
            {
                QTextCharFormat format;
                format.setFontFamily("Ubuntu Mono");
                format.setFontFixedPitch(true);
                if (codeColor.isValid()) {
                    format.setForeground(codeColor);
                }
                cursor.mergeCharFormat(format);
                break;
            }
            case MessageEntity::MessageEntityClassType::typeMessageEntityEmail:
            {
                QTextCharFormat format;
                format.setAnchor(true);
                format.setAnchorHref("mailto:" % subText);
                if (linkColor.isValid()) {
                    format.setForeground(linkColor);
                }
                cursor.mergeCharFormat(format);
                break;
            }
            case MessageEntity::MessageEntityClassType::typeMessageEntityItalic:
            {
                QTextCharFormat format;
                format.setFontItalic(true);
                cursor.mergeCharFormat(format);
                break;
            }
            case MessageEntity::MessageEntityClassType::typeMessageEntityHashtag:
            case MessageEntity::MessageEntityClassType::typeMessageEntityMention:
            case MessageEntity::MessageEntityClassType::typeMessageEntityMentionName:
            {
                QTextCharFormat format;
                format.setAnchor(true);
                format.setAnchorHref(subText);
                if (linkColor.isValid()) {
                    format.setForeground(linkColor);
                }
                cursor.mergeCharFormat(format);
                break;
            }
            case MessageEntity::MessageEntityClassType::typeMessageEntityTextUrl:
            case MessageEntity::MessageEntityClassType::typeMessageEntityUrl:
            {
                QTextCharFormat format;
                format.setAnchor(true);
                format.setAnchorHref(entity.classType() == MessageEntity::MessageEntityClassType::typeMessageEntityTextUrl ? entity.url() : subText);
                format.setFontUnderline(true);
                if (linkColor.isValid()) {
                    format.setForeground(linkColor);
                }
                cursor.mergeCharFormat(format);
                break;
            }
            default:
                break;
            }
        }
    }
    result->adjustSize();
}

