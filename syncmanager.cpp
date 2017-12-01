#include "syncmanager.h"

SyncManager::SyncManager(TelegramQml *tQ)
{
    this->tQ = tQ;
    this->_globalState = UpdatesState();
    this->_channelStates = QHash<qint32, UpdatesState>();
    this->syncTask = new SyncManagerSyncTask(&serverSyncMutex, &serverSyncRequested, tQ);
    syncTask->start();
}

UpdatesState SyncManager::getState(qint32 channelId)
{
    UpdatesState result = UpdatesState();
    updateMutex.lock();
    if (channelId == 0)
        result = this->_globalState;
    else if (this->_channelStates.contains(channelId))
    {
        result = this->_channelStates.value(channelId);
    }
    updateMutex.unlock();
    return result;
}

void SyncManager::requestSync()
{
    serverSyncRequested.wakeAll();

}

void SyncManager::setState(const UpdatesState &state, qint32 channelId)
{
    updateMutex.lock();
    if (channelId == 0)
    {
        qWarning() << "Setting global state, seq: " << state.seq();
        this->_globalState = state;
    }
    else
        this->_channelStates[channelId] = UpdatesState(state);
    updateMutex.unlock();
}

bool SyncManager::isSynced(const UpdatesState &state, qint32 channelId)
{
    bool result = false;
    updateMutex.lock();
    if (channelId == 0)
    {
        result = (this->_globalState.seq() == state.seq() );
        qWarning() << "Global state " << (result? "is unchanged" : "has changed");
    }
    else
    {
        if (this->_channelStates.contains(channelId))
        {
            UpdatesState currentState = this->_channelStates[channelId];
            qWarning() << "Channel " << channelId << " state old seq: " << currentState.seq() << ", new seq: " << state.seq();
            result = (currentState.seq() == state.seq() - 1);
        }
    }
    updateMutex.unlock();
    return result;
}

SyncManagerSyncTask::SyncManagerSyncTask(QMutex *serverSyncMutex, QWaitCondition *serverSyncRequested, TelegramQml *tQ, QObject *parent): QThread(parent)
{
    this->tQ = tQ;
    this->serverSyncMutex = serverSyncMutex;
    this->serverSyncRequested = serverSyncRequested;
}

void SyncManagerSyncTask::run()
{
    serverSyncMutex->lock();
    while(true)
    {
        serverSyncRequested->wait(this->serverSyncMutex);
        QThread::msleep(1500);
        qWarning() << "Deferred execution of server synchronization requested";
        QMetaObject::invokeMethod(tQ, "updatesGetDifference", Qt::QueuedConnection);

    }
}
