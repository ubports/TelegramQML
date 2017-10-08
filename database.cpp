#define FIRST_CHECK if(!this->core) return;

#include "database.h"
#include "databasecore.h"
#include "telegramqml_macros.h"

#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QDir>
#include <QDebug>

Database::Database(QObject *parent) :
    QObject(parent)
{
    this->thread = 0;
    this->core = 0;
    this->internal_encrypter = 0;
}

void Database::setPhoneNumber(const QString &phoneNumber)
{
    if(this->internal_phoneNumber == phoneNumber)
        return;

    this->internal_phoneNumber = phoneNumber;
    refresh();

    Q_EMIT phoneNumberChanged();
}

QString Database::phoneNumber() const
{
    return this->internal_phoneNumber;
}

void Database::setConfigPath(const QString &path)
{
    if(this->internal_configPath == path)
        return;

    this->internal_configPath = path;
    refresh();

    Q_EMIT configPathChanged();
}

QString Database::configPath() const
{
    return this->internal_configPath;
}

void Database::setEncrypter(DatabaseAbstractEncryptor *encrypter)
{
    this->internal_encrypter = encrypter;
    if(this->core)
        QMetaObject::invokeMethod(this->core, "setEncrypter", Qt::QueuedConnection, Q_ARG(DatabaseAbstractEncryptor*, encrypter));
}

DatabaseAbstractEncryptor *Database::encrypter() const
{
    return this->internal_encrypter;
}

void Database::insertUser(const User &user)
{
    FIRST_CHECK;
    DbUser duser;
    duser.user = user;

    QMetaObject::invokeMethod(this->core, "insertUser", Qt::QueuedConnection, Q_ARG(DbUser,duser));
}

void Database::insertChat(const Chat &chat)
{
    FIRST_CHECK;
    DbChat dchat;
    dchat.chat = chat;

    QMetaObject::invokeMethod(this->core, "insertChat", Qt::QueuedConnection, Q_ARG(DbChat,dchat));
}

void Database::insertDialog(const Dialog &dialog, bool encrypted)
{
    FIRST_CHECK;
    DbDialog ddlg;
    ddlg.dialog = dialog;

    QMetaObject::invokeMethod(this->core, "insertDialog", Qt::QueuedConnection, Q_ARG(DbDialog,ddlg), Q_ARG(bool,encrypted));
}

void Database::insertContact(const Contact &contact)
{
    FIRST_CHECK;
    DbContact dcnt;
    dcnt.contact = contact;

    QMetaObject::invokeMethod(this->core, "insertContact", Qt::QueuedConnection, Q_ARG(DbContact,dcnt));
}

void Database::insertMessage(const Message &message, bool encrypted)
{
    FIRST_CHECK;
    DbMessage dmsg;
    dmsg.message = message;

    QMetaObject::invokeMethod(this->core, "insertMessage", Qt::QueuedConnection, Q_ARG(DbMessage,dmsg), Q_ARG(bool,encrypted));
}

void Database::insertMediaEncryptedKeys(qint64 mediaId, const QByteArray &key, const QByteArray &iv)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "insertMediaEncryptedKeys", Qt::QueuedConnection, Q_ARG(qint64,mediaId), Q_ARG(QByteArray,key), Q_ARG(QByteArray,iv));
}

void Database::updateUnreadCount(qint64 chatId, int unreadCount)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "updateUnreadCount", Qt::QueuedConnection, Q_ARG(qint64,chatId), Q_ARG(int,unreadCount));
}

void Database::readFullDialogs()
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "readFullDialogs", Qt::QueuedConnection);
}

void Database::markMessagesAsReadFromMaxDate(qint32 chatId, qint32 maxDate)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "markMessagesAsReadFromMaxDate", Qt::QueuedConnection, Q_ARG(qint32, chatId), Q_ARG(qint32, maxDate));
}

void Database::markMessagesAsRead(const QList<qint32> &messages)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "markMessagesAsRead", Qt::QueuedConnection, Q_ARG(QList<qint32>, messages));
}

void Database::readMessages(const Peer &peer, int offset, int limit)
{
    FIRST_CHECK;
    DbPeer dpeer;
    dpeer.peer = peer;

    QMetaObject::invokeMethod(this->core, "readMessages", Qt::QueuedConnection, Q_ARG(DbPeer,dpeer), Q_ARG(int,offset), Q_ARG(int,limit) );
}

void Database::deleteMessage(qint64 msgId)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "deleteMessage", Qt::QueuedConnection, Q_ARG(qint64,msgId));
}

void Database::deleteDialog(qint64 dlgId)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "deleteDialog", Qt::QueuedConnection, Q_ARG(qint64,dlgId));
}

void Database::deleteHistory(qint64 dlgId)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "deleteHistory", Qt::QueuedConnection, Q_ARG(qint64,dlgId));
}

void Database::blockUser(qint64 userId)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "blockUser", Qt::QueuedConnection, Q_ARG(qint64, userId));
}

void Database::unblockUser(qint64 userId)
{
    FIRST_CHECK;
    QMetaObject::invokeMethod(this->core, "unblockUser", Qt::QueuedConnection, Q_ARG(qint64, userId));
}

void Database::userFounded_slt(const DbUser &user)
{
    Q_EMIT userFounded(user.user);
}

void Database::chatFounded_slt(const DbChat &chat)
{
    Q_EMIT chatFounded(chat.chat);
}

void Database::dialogFounded_slt(const DbDialog &dialog, bool encrypted)
{
    Q_EMIT dialogFounded(dialog.dialog, encrypted);
}

void Database::messageFounded_slt(const DbMessage &message)
{
    Q_EMIT messageFounded(message.message);
}

void Database::contactFounded_slt(const DbContact &contact)
{
    Q_EMIT contactFounded(contact.contact);
}

void Database::refresh()
{
    if(this->core && this->thread)
    {
        this->thread->quit();
        this->thread->wait();
        this->thread->deleteLater();
        this->core->deleteLater();
        this->thread = 0;
        this->core = 0;
    }

    if(this->internal_phoneNumber.isEmpty() || this->internal_configPath.isEmpty())
        return;

    const QString &dirPath = this->internal_configPath + "/" + this->internal_phoneNumber;
    QDir().mkpath(dirPath);
    this->path = dirPath + "/database.db";

    if( !QFileInfo::exists(this->path) )
        QFile::copy(DATABASE_DB_PATH,this->path);

    QFile(this->path).setPermissions(QFileDevice::WriteOwner|QFileDevice::WriteGroup|QFileDevice::ReadUser|QFileDevice::ReadGroup);

    this->core = new DatabaseCore(this->path, this->internal_configPath, this->internal_phoneNumber);
    this->core->setEncrypter(this->internal_encrypter);

    this->thread = new QThread(this);
    this->thread->start();

    this->core->moveToThread(this->thread);

    connect(this->core, SIGNAL(chatFounded(DbChat))         , SLOT(chatFounded_slt(DbChat))         , Qt::QueuedConnection );
    connect(this->core, SIGNAL(userFounded(DbUser))         , SLOT(userFounded_slt(DbUser))         , Qt::QueuedConnection );
    connect(this->core, SIGNAL(dialogFounded(DbDialog,bool)), SLOT(dialogFounded_slt(DbDialog,bool)), Qt::QueuedConnection );
    connect(this->core, SIGNAL(messageFounded(DbMessage))   , SLOT(messageFounded_slt(DbMessage))   , Qt::QueuedConnection );
    connect(this->core, SIGNAL(contactFounded(DbContact))   , SLOT(contactFounded_slt(DbContact))   , Qt::QueuedConnection );
    connect(this->core, SIGNAL(mediaKeyFounded(qint64,QByteArray,QByteArray)),
            SIGNAL(mediaKeyFounded(qint64,QByteArray,QByteArray)), Qt::QueuedConnection );
}

Database::~Database()
{
    if(this->core && this->thread)
    {
        this->thread->quit();
        this->thread->wait();
        this->thread->deleteLater();
        this->core->deleteLater();
        this->thread = 0;
        this->core = 0;
    }
}
