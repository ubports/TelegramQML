#include "utils.h"

qint64 QmlUtils::getUnifiedMessageKey(qint32 messageId, qint32 peerId)
{
    qint64 result;
    if (messageId==0)
        result = 0;
    else
        result = (static_cast<qint64>(peerId) << 32) + messageId;
    return result;
}

qint32 QmlUtils::getSeparatePeerId(qint64 unifiedKey)
{
    qint64 result;

    result = static_cast<qint32>(unifiedKey >> 32);
    return result;
}

qint32 QmlUtils::getSeparateMessageId(qint64 unifiedKey)
{
    qint64 result;

    result = static_cast<qint32>((unifiedKey << 32) >> 32);
    return result;
}


