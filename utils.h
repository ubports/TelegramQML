#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QMutex>
#include <QtGlobal>

class QmlUtils
{

public:
    static qint64 getUnifiedMessageKey(qint32 messageId, qint32 peerId);
    static qint32 getSeparateMessageId(qint64 unifiedKey);
    static qint32 getSeparatePeerId(qint64 unifiedKey);

};

#endif // QMLUTILS_H
