#ifndef COUNTERMODE_H
#define COUNTERMODE_H

#include "baseblockcipher.h"

class CounterMode
{
public:
    explicit CounterMode(BaseBlockCipher *cipher);
    bool setNonce(const QByteArray &nonce);
    void startTask(qint64 startOffset = 0);
    QByteArray updateTask(const QByteArray &data);

private:
    BaseBlockCipher *mCipher;
    QByteArray mNonce;
    QByteArray mCB;
    qint64 mOffset;

    void setupCB(const QByteArray &baseValue, quint32 counter);
    void increaseCB(quint32 increment = 1);
};

#endif // COUNTERMODE_H
