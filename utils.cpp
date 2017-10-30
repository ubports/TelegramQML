#include "utils.h"

qint64 QmlUtils::getUnifiedMessageKey(qint32 messageId, qint32 peerId)
{
    return (static_cast<qint64>(peerId) << 32) + messageId;
}

qint32 QmlUtils::getSeparatePeerId(qint64 unifiedKey)
{
    return static_cast<qint32>(unifiedKey >> 32);
}

qint32 QmlUtils::getSeparateMessageId(qint64 unifiedKey)
{
    return static_cast<qint32>((unifiedKey << 32) >> 32);
}
