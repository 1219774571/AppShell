#include "AppConsole.h"

#include <QFile>
#include <QDomDocument>

AppConsole::AppConsole(const QString &path, QObject *parent)
    : QObject(parent)
    , key_("password.741/+8520*-963")
    , path_(path)
{
    restore();
}

bool AppConsole::setApp(const QString &app)
{
    if (app.isEmpty()) {
        return false;
    }
    if (QFile::exists(app) == false) {
        return false;
    }
    app_ = app;
    return true;
}

void AppConsole::addArgs(const QString &option, const QString &arg, const QString &method)
{
    argsFormat data(option, arg, method);
    packageArgsFormat(data);

    int index = args_.indexOf(data);
    if (index != -1) {
        args_.replace(index, data);
    } else {
        args_.append(data);
    }
}

QStringList AppConsole::appArgs() const
{
    QStringList args;
    for (auto i : args_) {
        unpackageArgsFormat(i);
        if (i.option.isEmpty() == false) {
            args.append(i.option);
        }
        if (i.value.isEmpty() == false) {
            args.append(i.value);
        }
    }

    return args;
}

QString AppConsole::appData() const
{
    QString data = app_;
    for (auto &i : args_) {
        data += QStringLiteral(" ") + i.option + QStringLiteral(" ") + i.value;
    }
    return data;
}

void AppConsole::save() const
{
    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    QDomElement root = doc.createElement("AppConsole");
    root.setAttribute("method", "startApp");
    doc.appendChild(root);
    QDomElement app = doc.createElement("App");
    app.setAttribute("path", app_);
    root.appendChild(app);
    for (auto &i : args_) {
        QDomElement element = doc.createElement("Args");
        element.setAttribute("option", i.option);
        element.setAttribute("value", i.value);
        element.setAttribute("method", i.method);
        app.appendChild(element);
    }
    QByteArray data = doc.toByteArray();
    QFile file(path_);
    if (file.open(QFile::WriteOnly) == false) {
        return ;
    }
    file.write(data);
    file.flush();
    file.close();
}

void AppConsole::restore()
{
    QFile file(path_);
    if (file.open(QFile::ReadOnly) == false) {
        return ;
    }
    QByteArray data = file.readAll();
    file.close();

    QDomDocument doc;
    doc.setContent(data);
    QDomElement element = doc.firstChildElement("AppConsole");
    if (element.isNull()) {
        return ;
    }
    if (element.attribute("method") != "startApp") {
        return ;
    }
    element = element.firstChildElement("App");
    if (element.isNull()) {
        return ;
    }
    app_ = element.attribute("path");

    args_.clear();
    element = element.firstChildElement("Args");
    while (element.isNull() == false) {
        argsFormat arg;
        arg.option = element.attribute("option");
        arg.value = element.attribute("value");
        arg.method = element.attribute("method");
        args_.append(arg);
        element = element.nextSiblingElement();
    }
}

void AppConsole::packageArgsFormat(AppConsole::argsFormat &arg) const
{
    if (arg.method.isEmpty()) {
        return ;
    } else if (arg.method == "password") {
        arg.value = encrypt(arg.value.toUtf8(), key_);
    }
}

void AppConsole::unpackageArgsFormat(AppConsole::argsFormat &arg) const
{
    if (arg.method.isEmpty()) {
        return ;
    } else if (arg.method == "password") {
        arg.value = decrypt(arg.value.toUtf8(), key_);
    }
}

QByteArray AppConsole::encrypt(QByteArray data, const QByteArray &key) const
{
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data.at(i) ^ key.at(i % key.size());
    }
    return data.toBase64();
}

QByteArray AppConsole::decrypt(QByteArray data, const QByteArray &key) const
{
    data = QByteArray::fromBase64(data);
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data.at(i) ^ key.at(i % key.size());
    }
    return data;
}
