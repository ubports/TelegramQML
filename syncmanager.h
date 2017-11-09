#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include <QList>
#include <QLoggingCategory>
#include "telegramqml.h"
#include "objects/types.h"

Q_DECLARE_LOGGING_CATEGORY(TG_QML_SYNCMGR)

class TELEGRAMQMLSHARED_EXPORT SyncManager : public QObject
{
    TelegramQml *tQ;
    UpdatesStateObject _globalState;
    QHash<qint32, UpdatesStateObject> _channelStates;

public:
    SyncManager(TelegramQml *tQ);
    bool isSynced(const UpdatesState &state, qint32 channelId = 0);
    void setState(const UpdatesState &state, qint32 channelId = 0);
    UpdatesStateObject getState(qint32 channelId = 0);

};

#endif // SYNCMANAGER_H
