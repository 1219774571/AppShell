#include "AppConsole.h"
#include "Tool.h"

#include <QFile>
#include <QDomDocument>


AppConsole::AppConsole(const QString &path, QObject *parent)
    : QObject(parent)
    , key_("password.741/+8520*-963")
    , path_(path)
{
    Restore();
}


void AppConsole::AddApp(const QString &appPath, const QString &option, const QString &value, const ArgsFormatBase::mMethod method, bool enable, int index)
{
    ArgsFormatBase arg(option, value, method);
    arg.UpdateMethod(key_);

    if (index == -1) {
        AppArgsStruct args(ArgsFormat(appPath, arg, enable));
        args_.append(args);
        return ;
    }

    int pos = GetSameArgsIndex(appPath, index);

    args_[pos].args.enable = enable;
    int argPos = args_.at(pos).args.args.indexOf(arg);
    if (argPos == -1) {
        args_[pos].args.args.append(arg);
    } else {
        args_[pos].args.args.replace(argPos, arg);
    }
}


bool AppConsole::DelApp(const QString &appPath, int index)
{
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return false;
    }

    args_.remove(pos);
    return true;
}


bool AppConsole::ModifyAppPath(const QString &oldPath, const QString &newPath, int index)
{
    int pos = GetSameArgsIndex(oldPath, index);
    if (pos == -1) {
        return false;
    }

    args_[pos].args.path = newPath;
    return true;
}


bool AppConsole::CleanArgs(const QString &appPath, int index)
{
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return false;
    }

    args_[pos].args.args.clear();
    return true;
}


bool AppConsole::DelArgs(const QString &appPath, const QString &option, int index)
{
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return false;
    }

    ArgsFormatBase base;
    base.option = option;
    int argsPos = args_.at(pos).args.args.indexOf(base);

    if (argsPos == -1) {
        return false;
    }

    args_[pos].args.args.remove(argsPos);
    return true;
}


QStringList AppConsole::AppArgs(const QString &appPath, int index) const
{
    QStringList args;
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return args;
    }

    return args_.at(pos).args.Args(key_);
}


QString AppConsole::AppData(const QString &appPath, int index) const
{
    QString appArgs;
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return appArgs;
    }

    QStringList args = args_.at(pos).args.Args();
    appArgs = appPath;
    for (auto &i : args) {
        appArgs += " " + i;
    }

    return appArgs;
}


void AppConsole::Save() const
{
    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    QDomElement root = doc.createElement("AppConsole");
    root.setAttribute("method", "startApp");
    doc.appendChild(root);

    for (auto &i : args_) {
        nArgsFormat nArgs(i.args);
        nArgs.CreateXml(doc, root);
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


void AppConsole::Restore()
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

    args_.clear();
    element = element.firstChildElement("App");
    while (element.isNull() == false) {
        nArgsFormat nArgs;
        nArgs.ParseXml(element);
        ArgsFormat args(nArgs);
        AppArgsStruct appArgs(args);
        args_.append(appArgs);
        element = element.nextSiblingElement();
    }
}


QString AppConsole::GetArgsPath(int i)
{
    QString path;
    if (i < 0 || i >= args_.size()) {
        return path;
    }

    return args_.at(i).args.path;
}


void AppConsole::Start(const QString &appPath, int index)
{
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return ;
    }

    if (args_.at(pos).process->isOpen()) {
        return ;
    }

    disconnect(args_.at(pos).process.data(), &QProcess::readyReadStandardOutput, this, &AppConsole::ParseStandOut);
    disconnect(args_.at(pos).process.data(), &QProcess::readyReadStandardError, this, &AppConsole::ParseErrorOut);
    disconnect(args_.at(pos).process.data(), &QProcess::started, this, &AppConsole::ParseStarted);
    disconnect(args_.at(pos).process.data(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(ParseExited(int, QProcess::ExitStatus)));

    connect(args_.at(pos).process.data(), &QProcess::readyReadStandardOutput, this, &AppConsole::ParseStandOut);
    connect(args_.at(pos).process.data(), &QProcess::readyReadStandardError, this, &AppConsole::ParseErrorOut);
    connect(args_.at(pos).process.data(), &QProcess::started, this, &AppConsole::ParseStarted);
    connect(args_.at(pos).process.data(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(ParseExited(int, QProcess::ExitStatus)));

    args_.at(pos).process->start(appPath, this->AppArgs(appPath, pos));
}


void AppConsole::Stop(const QString &appPath, int index)
{
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return ;
    }

    args_.at(pos).process->kill();
    args_.at(pos).process->close();
}


void AppConsole::StartAll()
{
    for (int i = 0; i < args_.size(); ++i) {
        Start(args_.at(i).args.path, i);
    }
}


void AppConsole::StopAll()
{
    for (int i = 0; i < args_.size(); ++i) {
        Stop(args_.at(i).args.path, i);
    }
}


bool AppConsole::IsRun(const QString &appPath, int index) const
{
    int pos = GetSameArgsIndex(appPath, index);

    if (pos == -1) {
        return false;
    }

    return args_.at(pos).process->isOpen();
}


bool AppConsole::IsAllRun() const
{
    for (auto &i : args_) {
        if (i.process->isOpen() == false) {
            return false;
        }
    }

    return true;
}


int AppConsole::GetArgsIndex(const QString &path, int pos) const
{
    for (int i = pos; i < args_.size(); ++i) {
        if (args_.at(i).args.path != path) {
            continue;
        }
        return i;
    }
    return -1;
}


int AppConsole::GetArgsIndex(const QObject *obj) const
{
    for (int i = 0; i < args_.size(); ++i) {
        if (args_.at(i).process.data() != obj) {
            continue;
        }
        return i;
    }
    return -1;
}


int AppConsole::GetSameArgsIndex(const QString &appPath, int num) const
{
    int pos = GetArgsIndex(appPath);

    if (pos == -1) {
        return pos;
    }

    for (int i = 0; i < num; ++i, --num) {
        int position = GetArgsIndex(appPath, pos + 1);
        if (position == -1) {
            break;
        }
        pos = position;
    }

    return pos;
}


void AppConsole::ParseStandOut()
{
    int i = GetArgsIndex(sender());
    QString output = tool::IconvCode(args_.at(i).process->readAllStandardOutput());
    emit AppStandOut(args_.at(i).args.path, output, i);
}


void AppConsole::ParseErrorOut()
{
    int i = GetArgsIndex(sender());
    if (i == -1) {
        return ;
    }
    QString output = tool::IconvCode(args_.at(i).process->readAllStandardError());
    emit AppErrorOut(args_.at(i).args.path, output, i);
}


void AppConsole::ParseStarted()
{
    int i = GetArgsIndex(sender());
    if (i == -1) {
        return ;
    }
    emit AppStarted(args_.at(i).args.path, i);
}


void AppConsole::ParseExited(int code, QProcess::ExitStatus status)
{
    int index = GetArgsIndex(sender());
    if (index == -1) {
        return ;
    }

    args_.at(index).process->close();
    emit AppExitStatus(args_.at(index).args.path, code, status, index);
}

