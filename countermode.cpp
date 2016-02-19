#include "countermode.h"

///
/// \brief CounterMode::CounterMode
/// \param cipher blockSize() 必须大于 4
///
CounterMode::CounterMode(BaseBlockCipher *cipher)
{
    this->mCipher = cipher;
    this->mOffset = -1;
}

bool CounterMode::setNonce(const QByteArray &nonce)
{
    if (nonce.size() != this->mCipher->blockSize())
        return false;
    this->mNonce = nonce;
    return true;
}

void CounterMode::startTask(qint64 startOffset)
{
    if (startOffset < 0 || this->mNonce.isEmpty())
    {
        this->mOffset = -1;
        return;
    }

    this->mOffset = startOffset;
    setupCB(this->mNonce, startOffset / this->mCipher->blockSize());
}

QByteArray CounterMode::updateTask(const QByteArray &data)
{
    if (mOffset < 0)
        return QByteArray();

    QByteArray out;
    QByteArray xorKey = this->mCipher->encryptBlock(this->mCB);
    int blockSize = this->mCipher->blockSize();
    int s = this->mOffset % (qint64)blockSize;

    out.resize(data.size());

    for (int i = 0; i < data.size(); ++i)
    {
        out[i] = data.at(i) ^ xorKey.at(s);
        ++s;
        if (s == blockSize)
        {
            s = 0;
            increaseCB(1);
            xorKey = this->mCipher->encryptBlock(this->mCB);
        }
    }
    this->mOffset += data.size();
    return out;
}

void CounterMode::setupCB(const QByteArray &baseValue, quint32 counter)
{
    this->mCB = baseValue;
    increaseCB(counter);
}

void CounterMode::increaseCB(quint32 increment)
{
    int size = this->mCB.size();
    quint32 c1 = (quint32)(unsigned char)this->mCB.at(size - 1);
    quint32 c2 = (quint32)(unsigned char)this->mCB.at(size - 2);
    quint32 c3 = (quint32)(unsigned char)this->mCB.at(size - 3);
    quint32 c4 = (quint32)(unsigned char)this->mCB.at(size - 4);

    quint32 lsb = c1 | (c2 << 8) | (c3 << 16) | (c4 << 24);

    lsb += increment;

    quint32 mask = 0xffu;
    this->mCB[size - 1] = lsb & mask;
    this->mCB[size - 2] = (lsb >> 8) & mask;
    this->mCB[size - 3] = (lsb >> 16) & mask;
    this->mCB[size - 4] = (lsb >> 24) & mask;
}
