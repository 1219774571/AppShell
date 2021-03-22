

#ifndef TOOL_H
#define TOOL_H


#include <QObject>


namespace tool
{


/**
 * @brief IconvCode 转换为Unicode编码
 * @param data 原数据
 * @return 转换数据
 */
QString IconvCode(const QByteArray &data);


}



#endif // TOOL_H
