
#include "Tool.h"

#include <QTextCodec>


namespace tool {


QString IconvCode(const QByteArray &data)
{
    QTextCodec::ConverterState state;
    QString text = QTextCodec::codecForName("UTF-8")->toUnicode(data.constData(), data.size(), &state);
    if (state.invalidChars > 0)
    {
        text = QTextCodec::codecForName("GBK")->toUnicode(data.constData(), data.size());
    }

    return text;
}



}
