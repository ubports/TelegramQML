#include "syncmanager.h"

SyncManager::SyncManager(TelegramQml *tQ)
{
    this->tQ = tQ;
    this->_globalState = UpdatesStateObject();
    this->_channelStates = QHash<qint32, UpdatesStateObject>();

}

UpdatesStateObject SyncManager::getState(qint32 channelId)
{
    if (channelId == 0)
        return this->_globalState;
    else
        if (!this->channelStates.contains(channelId))
        {
            this->_channelStates[channelId] = UpdatesStateObject();
        }
    return this->_channelStates.value(channelId);
}

void SyncManager::setState(const UpdatesState &state, qint32 channelId)
{
    if (channelId == 0)
    {
        _globalState = state;
    }
    else
        this->_channelStates[channelId] = UpdatesStateObject(state);
}

bool SyncManager::isSynced(const UpdatesState &state, qint32 channelId)
{
    if (channelId == 0)
    {
        return (_globalState.seq() == state.seq()- 1 );
    }
    else
    {
        UpdatesStateObject* _channelState = this->_channelStates[channelId];
        if (!_channelState)
            return false;
        else
            return (_channelState.seq() == state.seq() - 1);
    }

}
