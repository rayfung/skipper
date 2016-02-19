#ifndef TEA_H
#define TEA_H

#include <QByteArray>
#include "baseblockcipher.h"

class TEA : public BaseBlockCipher
{
public:
    explicit TEA();
    ~TEA();

    int keySize() const;
    int blockSize() const;
    bool setKey(const QByteArray &key);
    QByteArray encryptBlock(const QByteArray &in);
    QByteArray decryptBlock(const QByteArray &in);

private:
    static quint32 littleEndian(const QByteArray &number);
    static void serializeLittleEndian(QByteArray *out, int start, quint32 number);

private:
    QByteArray mKey;
};

#endif // TEA_H
