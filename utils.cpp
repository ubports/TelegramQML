#include "utils.h"
#include <QDebug>

qint64 QmlUtils::getUnifiedMessageKey(qint32 messageId, qint32 peerId)
{
    qint64 result;
    if (messageId==0)
        result = 0;
    else
        result = (static_cast<qint64>(peerId) << 32) + messageId;
    //qWarning() << "getUnifiedMessageKey: mId=" << messageId << ", peer=" << peerId << ", result=" << result;
    return result;
}

qint32 QmlUtils::getSeparatePeerId(qint64 unifiedKey)
{
    qint64 result;

    result = static_cast<qint32>(unifiedKey >> 32);
    //qWarning() << "getSeparatePeerId: unifiedId=" << unifiedKey << ", peer=" << result;
    return result;
}

qint32 QmlUtils::getSeparateMessageId(qint64 unifiedKey)
{
    qint64 result;

    result = static_cast<qint32>((unifiedKey << 32) >> 32);
    //qWarning() << "getSeparateMessageId: unifiedId=" << unifiedKey << ", msgid=" << result;
    return result;
}
