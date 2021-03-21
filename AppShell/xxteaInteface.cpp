

#include "xxteaInteface.h"
#include "xxtea/xxtea.h"


xxteaInteface::xxteaInteface()
{

}


QByteArray xxteaInteface::encrypt(const QByteArray &data, const QByteArray &key)
{
    std::string stdData = data.toStdString();
    std::size_t len = 0;
    char *encrypt = static_cast<char *>(xxtea_encrypt(stdData.c_str(), stdData.size(), key.toStdString().c_str(), &len));
    QByteArray encryptData(encrypt, static_cast<int>(len));
    free(encrypt);
    return encryptData;
}


QByteArray xxteaInteface::decrypt(const QByteArray &data, const QByteArray &key)
{
    std::string stdData = data.toStdString();
    std::size_t len = 0;
    char *encrypt = static_cast<char *>(xxtea_decrypt(stdData.c_str(), stdData.size(), key.toStdString().c_str(), &len));
    QByteArray decryptData(encrypt, static_cast<int>(len));
    free(encrypt);
    return decryptData;
}
