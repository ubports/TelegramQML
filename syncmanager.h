#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QWaitCondition>
#include <QLoggingCategory>
#include "telegramqml.h"
#include "telegram.h"


Q_DECLARE_LOGGING_CATEGORY(TG_QML_SYNCMGR)

class SyncManagerSyncTask: public QThread
{
    TelegramQml *tQ;
    QMutex *serverSyncMutex;
    QWaitCondition *serverSyncRequested;

public:
    SyncManagerSyncTask(QMutex *serverSyncMutex, QWaitCondition *serverSyncRequested, TelegramQml *tQ, QObject *parent = NULL);
    void run();
};

class TELEGRAMQMLSHARED_EXPORT SyncManager : public QObject
{
    TelegramQml *tQ;
    UpdatesState _globalState;
    QHash<qint32, UpdatesState> _channelStates;
    QMutex updateMutex;
    QMutex serverSyncMutex;
    QWaitCondition serverSyncRequested;

    SyncManagerSyncTask *syncTask;

public:
    SyncManager(TelegramQml *tQ);
    bool isSynced(const UpdatesState &state, qint32 channelId = 0);
    void setState(const UpdatesState &state, qint32 channelId = 0);
    UpdatesState getState(qint32 channelId = 0);
    void requestSync();

};



#endif // SYNCMANAGER_H
