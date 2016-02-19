#include "tea.h"

TEA::TEA()
{
}

TEA::~TEA()
{
}

int TEA::keySize() const
{
    return 16;
}

int TEA::blockSize() const
{
    return 8;
}

bool TEA::setKey(const QByteArray &key)
{
    if (key.size() != keySize())
        return false;
    this->mKey = key;
    return true;
}

quint32 TEA::littleEndian(const QByteArray &number)
{
    quint32 b0 = (quint32)(unsigned char)number.at(0);
    quint32 b1 = (quint32)(unsigned char)number.at(1);
    quint32 b2 = (quint32)(unsigned char)number.at(2);
    quint32 b3 = (quint32)(unsigned char)number.at(3);

    return (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

void TEA::serializeLittleEndian(QByteArray *out, int start, quint32 number)
{
    quint32 mask = 0xffu;

    (*out)[start] = number & mask;
    (*out)[start + 1] = (number >> 8) & mask;
    (*out)[start + 2] = (number >> 16) & mask;
    (*out)[start + 3] = (number >> 24) & mask;
}

QByteArray TEA::encryptBlock(const QByteArray &in)
{
    if (in.size() != blockSize())
        return QByteArray();

    QByteArray out;
    out.resize(blockSize());

    quint32 v0 = littleEndian(in.mid(0, 4));
    quint32 v1 = littleEndian(in.mid(4, 4));
    quint32 sum = 0, i;
    quint32 delta = 0x9e3779b9;
    quint32 k0 = littleEndian(this->mKey.mid(0, 4));
    quint32 k1 = littleEndian(this->mKey.mid(4, 4));
    quint32 k2 = littleEndian(this->mKey.mid(8, 4));
    quint32 k3 = littleEndian(this->mKey.mid(12, 4));

    for (i=0; i < 32; i++)
    {
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }

    serializeLittleEndian(&out, 0, v0);
    serializeLittleEndian(&out, 4, v1);
    return out;
}

QByteArray TEA::decryptBlock(const QByteArray &in)
{
    if (in.size() != blockSize())
        return QByteArray();

    QByteArray out;
    out.resize(blockSize());

    quint32 v0 = littleEndian(in.mid(0, 4));
    quint32 v1 = littleEndian(in.mid(4, 4));
    quint32 sum = 0xC6EF3720, i;
    quint32 delta = 0x9e3779b9;
    quint32 k0 = littleEndian(this->mKey.mid(0, 4));
    quint32 k1 = littleEndian(this->mKey.mid(4, 4));
    quint32 k2 = littleEndian(this->mKey.mid(8, 4));
    quint32 k3 = littleEndian(this->mKey.mid(12, 4));

    for (i=0; i<32; i++)
    {
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;
    }

    serializeLittleEndian(&out, 0, v0);
    serializeLittleEndian(&out, 4, v1);
    return out;
}
