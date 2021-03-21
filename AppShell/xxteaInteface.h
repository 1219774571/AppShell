

#ifndef XXTEAINTEFACE_H
#define XXTEAINTEFACE_H


#include <QObject>


class xxteaInteface
{
public:
    xxteaInteface();

    /**
     * @brief encrypt 加密data数据
     * @param data 需要加密的数据
     * @param key 密钥
     * @return 返回加密的数据
     */
    static QByteArray encrypt(const QByteArray &data, const QByteArray &key);

    /**
     * @brief decrypt 解密data数据
     * @param data 需要解密的数据
     * @param key 密钥
     * @return 返回解密的密钥
     */
    static QByteArray decrypt(const QByteArray &data, const QByteArray &key);
};

#endif // XXTEAINTEFACE_H
