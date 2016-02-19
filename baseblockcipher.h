#ifndef BASEBLOCKCIPHER_H
#define BASEBLOCKCIPHER_H

#include <QByteArray>

class BaseBlockCipher
{
public:
    BaseBlockCipher();
    virtual ~BaseBlockCipher() = 0;
    virtual int keySize() const = 0;
    virtual int blockSize() const = 0;
    virtual bool setKey(const QByteArray &key) = 0;
    virtual QByteArray encryptBlock(const QByteArray &in) = 0;
    virtual QByteArray decryptBlock(const QByteArray &in) = 0;
};

#endif // BASEBLOCKCIPHER_H
