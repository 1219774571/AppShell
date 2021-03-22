#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AppConsole.h"
#include "version.h"

#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include <QMetaEnum>


#define PASSWORD_CLOSE  "background-color: rgb(255, 51, 51);"
#define PASSWORD_OPEN   "background-color: rgb(92, 255, 51);"

#define PROCESS_START   QStringLiteral("启动")
#define PROCESS_STOP    QStringLiteral("停止")


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , app_(new AppConsole(QCoreApplication::applicationDirPath() + "/config.xml", this))
    , systemTray_(new QSystemTrayIcon(QIcon(":/ico.ico"), this))
{
    ui->setupUi(this);
    init();
    createMenu();

    initSystemTray();
    initProcess();
}


MainWindow::~MainWindow()
{
    killApp(true);
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    int index = ui->AppSelect->currentIndex();
    QString path = ui->AppSelect->currentText();
    if (systemTray_->isVisible() && app_->IsRun(path, GetAppSelectNum(path, index))) {
        this->hide();
        event->ignore();
        return ;
    }
    systemTray_->hide();
    return QMainWindow::closeEvent(event);
}


void MainWindow::init()
{
    this->setWindowTitle(QString("AppShell %1").arg(VERSION));

    UpdateSelectApp();
    ui->ArgsPassword->setStyleSheet(PASSWORD_CLOSE);

    connect(app_, &AppConsole::AppStandOut, this, [this](const QString &path, const QString &log, int index){
        LogOut(QString("index[%1] %2: %3").arg(index).arg(path).arg(log));
    });

    connect(app_, &AppConsole::AppErrorOut, this, [this](const QString &path, const QString &log, int index){
        LogOut(QString("index[%1] %2: %3").arg(index).arg(path).arg(log));
    });
    connect(app_, &AppConsole::AppStarted, this, [this](const QString &path, int index){
        LogOut(QString("index[%1], start: %2").arg(index).arg(app_->AppData(path, index)));
        if (ui->AppSelect->currentText() == path) {
            ui->StartUp->setText(PROCESS_STOP);
        }
    });
    connect(app_, &AppConsole::AppExitStatus, this, [this](const QString &path, int code, QProcess::ExitStatus status, int index){
        QMetaEnum meta = QMetaEnum::fromType<QProcess::ExitStatus>();
        LogOut(QString("index[%1] %2: exit. code[%3], status[%4]").arg(index).arg(path).arg(code).arg(meta.valueToKey(status)));
        if (ui->AppSelect->currentText() == path) {
            ui->StartUp->setText(PROCESS_START);
        }
        this->show();
    });
}


void MainWindow::initSystemTray()
{
    connect(systemTray_, &QSystemTrayIcon::activated, this, &MainWindow::activeTray);

    QMenu *menu = new QMenu(this);
    QAction *action = new QAction(QStringLiteral("显示"), menu);
    connect(action, &QAction::triggered, this, [this](){ this->show(); });
    menu->addAction(action);
    action = new QAction(QStringLiteral("退出"), menu);
    connect(action, &QAction::triggered, this, [this](){ this->~MainWindow(); exit(0); });
    menu->addAction(action);
    systemTray_->setContextMenu(menu);

    systemTray_->show();
}


void MainWindow::initProcess()
{
    for (int i = 0; i < ui->AppSelect->count(); ++i) {
        QString path = ui->AppSelect->itemText(i);
        QFileInfo fileInfo(path);
        if (QFile::exists(fileInfo.absoluteFilePath())) {
            QScopedPointer<QProcess> process(new QProcess);
#ifdef Q_OS_WIN
            process->start("tasklist", QStringList() << "/FI imagename eq " + fileInfo.fileName());
#else
            process->start("/bin/bash", QStringList() << "-c" << QString("ps -ef | grep '%1' | grep -v grep").arg(fileInfo.fileName()));
#endif
            process->waitForFinished();
            QString outputStr = QString::fromLocal8Bit(process->readAllStandardOutput());
            if(outputStr.contains(fileInfo.fileName()) == false){
                app_->Start(path, GetAppSelectNum(path, i));
            }
        }
    }

    if (app_->IsAllRun() && app_->Size() != 0) {
        QTimer::singleShot(0, this, &MainWindow::hide);
    }
}


void MainWindow::createMenu()
{
    QMenu *menu = new QMenu(QStringLiteral("控制"),this);
    ui->Menu->addMenu(menu);

    QAction *action = new QAction(getProcessAutoRun(QCoreApplication::applicationFilePath()).isEmpty() ? QStringLiteral("开机启动") : QStringLiteral("关闭开机启动"), this);
    actionMap_["AutoRunBoot"] = action;
    connect(action, &QAction::triggered, this, [this](){
        QString path(QCoreApplication::applicationFilePath());
        if (getProcessAutoRun(path).isEmpty() == false) {
            if (delProcessAutoRun(path)) {
                actionMap_["AutoRunBoot"]->setText(QStringLiteral("开机启动"));
            }
        } else {
            if (setProcessAutoRun(path)) {
                actionMap_["AutoRunBoot"]->setText(QStringLiteral("关闭开机启动"));
            }
        }
    });
    menu->addAction(action);

    action = new QAction(QStringLiteral("强制关闭运行程序"), this);
    actionMap_["forceKillAllApp"] = action;
    connect(action, &QAction::triggered, this, [this](){ killApp(false, true); });
    menu->addAction(action);

    action = new QAction(QStringLiteral("全部关闭"), this);
    actionMap_["AllKillAllApp"] = action;
    connect(action, &QAction::triggered, this, [this](){ killApp(); });
    menu->addAction(action);
}


void MainWindow::killApp(bool all, bool sys)
{
    QString path = ui->AppSelect->currentText();
    int index = ui->AppSelect->currentIndex();

    // 杀掉自身运行中的
    if (all) {
        app_->StopAll();
    } else {
        int num = GetAppSelectNum(path, index);
        app_->Stop(path, num);
    }

    // 杀掉外部运行的
    if (sys) {
        QFileInfo fileInfo(path);
        if (QFile::exists(fileInfo.absoluteFilePath())) {
#ifdef Q_OS_WIN
            system(QString("taskkill /F /IM " + fileInfo.fileName() + " /T").toUtf8().constData());
#else
            system(QString("ps -ef | grep \"%1\" |grep -v \"grep\" | awk \'{print $2}\' | xargs | kill -9 ").arg(fileInfo.fileName()).toUtf8().constData());
#endif
        }
    }
}


bool MainWindow::isOpenPassword() const
{
    return ui->ArgsPassword->styleSheet().contains(PASSWORD_OPEN);
}


void MainWindow::LogOut(const QString &log)
{
    QString time = QTime::currentTime().toString("hh:mm:ss.zzz") + ": ";
    ui->LogOut->append(time + log);
}


bool MainWindow::setProcessAutoRun(const QString &path) const
{
#if WIN32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::Registry64Format);
#else
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
#endif
    QFileInfo info(path);
    QString name = info.baseName();
    QString setPath = reg.value(name).toString();
    QString newPath = QDir::toNativeSeparators(path);
    if (setPath != newPath) {
        reg.setValue(name, newPath);
    }
    return true;
#else
    Q_UNUSED(path);
    return false;
#endif
}


QString MainWindow::getProcessAutoRun(const QString &path) const
{
#if WIN32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::Registry64Format);
#else
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
#endif
#else
    QSettings reg;
#endif
    QFileInfo info(path);
    return reg.value(info.baseName()).toString();
}


bool MainWindow::delProcessAutoRun(const QString &path) const
{
#if WIN32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::Registry64Format);
#else
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
#endif
    QFileInfo info(path);
    QString name = info.baseName();
    reg.setValue(name, "");
    reg.remove(name);
    return true;
#else
    Q_UNUSED(path);
    return false;
#endif
}


int MainWindow::GetAppSelectNum(const QString &path, int index)
{
    int num = 0;
    if (index > ui->AppSelect->count()) {
        index = ui->AppSelect->count();
    }

    for (int i = 0; i < index; ++i) {
        if (ui->AppSelect->itemText(i) != path) {
            continue;
        }
        ++num;
    }

    return num;
}

void MainWindow::UpdateSelectApp()
{
    int index = ui->AppSelect->currentIndex();
    ui->AppSelect->clear();
    for (int i = 0; i < app_->Size(); ++i) {
        ui->AppSelect->addItem(app_->GetArgsPath(i));
    }
    if (index == -1 || index >= ui->AppSelect->count()) {
        return ;
    }
    ui->AppSelect->setCurrentIndex(index);
}


void MainWindow::activeTray(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick: {
        this->show();
    }break;
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::Context:
    case QSystemTrayIcon::Unknown:
        break;
    }
}


void MainWindow::on_ArgsPassword_clicked()
{
    LogOut(isOpenPassword() ? QStringLiteral("恢复普通参数状态") : QStringLiteral("进入加密参数状态"));
    QString color(isOpenPassword() ? PASSWORD_CLOSE : PASSWORD_OPEN);
    ui->ArgsPassword->setStyleSheet(color);
    QLineEdit::EchoMode mode = QLineEdit::Normal;
    if (isOpenPassword()) {
        mode = QLineEdit::Password;
    }
    ui->ArgsValue->setEchoMode(mode);
}


void MainWindow::on_ReArgs_clicked()
{
    int index = ui->AppSelect->currentIndex();
    if (index == -1) {
        return ;
    }

    QString path = ui->AppSelect->itemText(index);
    app_->CleanArgs(path);
    LogOut(app_->AppData(path));
}


void MainWindow::on_Save_clicked()
{
    if (app_->GetPath().isEmpty()) {
        LogOut(QStringLiteral("错误: 配置文件保存路径未设置"));
        return ;
    }
    app_->Save();

    ui->LogOut->clear();

    int max = ui->AppSelect->count();
    for (int i = 0; i < max; ++i) {
        QString path = ui->AppSelect->itemText(i);
        int num = GetAppSelectNum(path, i);
        LogOut(app_->AppData(path, num));
    }

    LogOut(QStringLiteral("数据保存到: ") + app_->GetPath());
}


void MainWindow::on_StartUp_clicked()
{
    int index = ui->AppSelect->currentIndex();
    if (index == -1) {
        return ;
    }
    QString path = ui->AppSelect->itemText(index);
    int num = GetAppSelectNum(path, index);
    if (app_->IsRun(path, num)) {
        app_->Stop(path, num);
    } else {
        app_->Start(path, num);
    }
}

void MainWindow::on_AddApp_clicked()
{
    QString path =  QFileDialog::getOpenFileName(this, QStringLiteral("选择启动程序"), ui->AppSelect->currentText());
    if (path.isEmpty()) {
        return ;
    }

    ui->AppSelect->addItem(path);
    app_->AddApp(path);
    ui->AppSelect->setCurrentIndex(ui->AppSelect->count() - 1);
}


void MainWindow::on_DelApp_clicked()
{
    int index = ui->AppSelect->currentIndex();
    if (index == -1) {
        return ;
    }

    QString path = ui->AppSelect->itemText(index);

    int num = GetAppSelectNum(path, index);

    if (app_->DelApp(path, num) == false) {
        LogOut("Application data does not exist");
    }

    ui->AppSelect->removeItem(index);
}


void MainWindow::on_AddArgs_clicked()
{
    int index = ui->AppSelect->currentIndex();
    if (index == -1) {
        LogOut("Please select an application");
        return ;
    }

    QString path = ui->AppSelect->itemText(index);
    QString option = ui->ArgsOption->text();
    QString value = ui->ArgsValue->text();

    int num = GetAppSelectNum(path, index);

    ArgsFormatBase::mMethod method = ArgsFormatBase::kNone;
    if (isOpenPassword()) {
        method = ArgsFormatBase::kPassword;
        ui->ArgsValue->clear();
    }

    app_->AddApp(path, option, value, method, true, num);
    LogOut(app_->AppData(path, num));
}


void MainWindow::on_DelArgs_clicked()
{
    int index = ui->AppSelect->currentIndex();
    if (index == -1) {
        LogOut("Please select an application");
        return ;
    }

    QString path = ui->AppSelect->itemText(index);

    int num = GetAppSelectNum(path, index);

    QString option = ui->ArgsOption->text();
    if (app_->DelArgs(path, option, num) == false) {
        LogOut("Application data does not exist");
    }
    LogOut(app_->AppData(path, num));
}


void MainWindow::on_AllStartUp_clicked()
{
    app_->StartAll();
}


void MainWindow::on_AppSelect_currentIndexChanged(const QString &arg1)
{
    int index = ui->AppSelect->currentIndex();
    int num = GetAppSelectNum(arg1, index);

    LogOut(app_->AppData(arg1, num));
    if (app_->IsRun(arg1, num)) {
        ui->StartUp->setText(PROCESS_STOP);
    } else {
        ui->StartUp->setText(PROCESS_START);
    }
}

void MainWindow::on_ModifyApp_clicked()
{
    QString path =  QFileDialog::getOpenFileName(this, QStringLiteral("选择启动程序"), ui->AppSelect->currentText());
    if (path.isEmpty()) {
        return ;
    }

    int index = ui->AppSelect->currentIndex();
    if (index == -1) {
        return ;
    }

    QString oldPath = ui->AppSelect->itemText(index);

    if (oldPath == path) {
        return ;
    }

    int num = GetAppSelectNum(oldPath, index);

    if (app_->ModifyAppPath(oldPath, path, num) == false) {
        LogOut("Application data does not exist");
    }
    UpdateSelectApp();
}
