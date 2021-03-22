

#include "ArgsFormatData.h"
#include "xxteaInteface.h"


void ArgsFormatBase::UpdateMethod(const QByteArray &key)
{
    switch (method) {
    case kPassword: {
        this->value = QByteArray(xxteaInteface::encrypt(this->value.toUtf8().constData(), key)).toBase64();
    }break;
    case kNone:
    default:
        break;
    }
}


void ArgsFormatBase::Conver(const nArgsFormatBase &args)
{
    this->option = args.option;
    this->value = args.value;

#define IF_METHOD(s) if (args.method == #s) { this->method = s; }
#define ELSE_IF_METHOD(s) else if (args.method == #s) { this->method = s; }

    IF_METHOD(kNone)
    ELSE_IF_METHOD(kPassword)
    else {
        this->method = kNone;
    }

#undef IF_METHOD
#undef ELSE_IF_METHOD
}


void nArgsFormatBase::Conver(const ArgsFormatBase &args)
{
    this->option = args.option;
    this->value = args.value;

#define IF_METHOD(s) if (args.method == ArgsFormatBase::s) { this->method = #s; }
#define ELSE_IF_METHOD(s) else if (args.method == ArgsFormatBase::s) { this->method = #s; }

    IF_METHOD(kNone)
    ELSE_IF_METHOD(kPassword)
    else {
        this->method = "kNone";
    }

#undef IF_METHOD
#undef ELSE_IF_METHOD
}


QStringList ArgsFormatBase::Args(const QByteArray &key) const
{
    QStringList argsList;
    if (option.isEmpty() == false) {
        argsList.append(option);
    }

    switch (method) {
    case kPassword :{
        if (value.isEmpty() == false) {
            argsList.append(xxteaInteface::decrypt(QByteArray::fromBase64(value.toUtf8()), key));
        }
    }break;
    case kNone:
    default:
        if (value.isEmpty() == false) {
            argsList.append(value.toUtf8());
        }
    }

    return argsList;
}


QStringList ArgsFormatBase::Args() const
{
    QStringList argsList;
    if (option.isEmpty() == false) {
        argsList.append(option);
    }

    if (value.isEmpty() == false) {
        argsList.append(value);
    }

    return argsList;
}


QStringList ArgsFormat::Args(const QByteArray &key) const
{
    QStringList argsList;
    for (auto &i : args) {
        QStringList appArgs = i.Args(key);
        if (appArgs.isEmpty()) {
            continue;
        }
        argsList.append(appArgs);
    }
    return argsList;
}


QStringList ArgsFormat::Args() const
{
    QStringList argsList;
    for (auto &i : args) {
        argsList.append(i.Args());
    }
    return argsList;
}


void ArgsFormat::Conver(const nArgsFormat &args)
{
    this->enable = args.enable == "true";
    this->path = args.path;
    this->args = args.args;
}


void nArgsFormat::ParseXml(const QDomElement &element)
{
    if (element.isNull()) {
        return ;
    }

    this->path = element.attribute("path");

    args.clear();
    QDomElement node = element.firstChildElement("Args");
    while (node.isNull() == false) {
        nArgsFormatBase arg;
        arg.option = node.attribute("option");
        arg.value = node.attribute("value");
        arg.method = node.attribute("method");

        ArgsFormatBase argBase(arg);
        this->args.append(argBase);
        node = node.nextSiblingElement();
    }

    node = element.firstChildElement("enable");
    this->enable = node.attribute("value");
}


void nArgsFormat::CreateXml(QDomDocument &doc, QDomElement &root) const
{
    QDomElement app = doc.createElement("App");
    app.setAttribute("path", this->path);
    root.appendChild(app);

    QDomElement node = doc.createElement("enable");
    node.setAttribute("value", this->enable);
    app.appendChild(node);

    for (auto &i : args) {
        nArgsFormatBase base(i);
        node = doc.createElement("Args");
        node.setAttribute("option", base.option);
        node.setAttribute("value", base.value);
        node.setAttribute("method", base.method);
        app.appendChild(node);
    }
}


void nArgsFormat::Conver(const ArgsFormat &args)
{
    this->enable = args.enable ? "true" : "false";
    this->path = args.path;
    this->args = args.args;
}
