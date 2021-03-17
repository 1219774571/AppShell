﻿#include "mainwindow.h"
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



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , app_(new AppConsole(QCoreApplication::applicationDirPath() + "/config.xml", this))
    , systemTray_(new QSystemTrayIcon(QIcon(":/ico.ico"), this))
    , process_(new QProcess(this))
{
    ui->setupUi(this);
    this->setWindowTitle(QString("AppShell %1").arg(VERSION));
    init();
    createMenu();

    initSystemTray();
    initProcess();
}


MainWindow::~MainWindow()
{
    killApp();
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (systemTray_->isVisible() && process_->isOpen()) {
        this->hide();
        event->ignore();
        return ;
    }
    systemTray_->hide();
    return QMainWindow::closeEvent(event);
}


void MainWindow::init()
{
    this->setMaximumSize(this->size());
    this->setMinimumSize(this->size());
    ui->AppText->setText(app_->appPath());

    ui->ArgsPassword->setStyleSheet(PASSWORD_CLOSE);

    if (app_->appData().isEmpty() == false) {
        logOut(app_->appData());
    }
}


void MainWindow::initSystemTray()
{
    connect(systemTray_, &QSystemTrayIcon::activated, this, &MainWindow::activeTray);

    QMenu *menu = new QMenu(this);
    QAction *action = new QAction("显示", menu);
    connect(action, &QAction::triggered, this, [this](){ this->show(); });
    menu->addAction(action);
    action = new QAction("退出", menu);
    connect(action, &QAction::triggered, this, [this](){ this->~MainWindow(); exit(0); });
    menu->addAction(action);
    systemTray_->setContextMenu(menu);

    systemTray_->show();
}


void MainWindow::initProcess()
{
    connect(process_, &QProcess::started, this, [this](){ ui->StartUp->setText(QStringLiteral("停止")); });
    connect(process_, &QProcess::readyReadStandardOutput, this, [this](){
        QString outputStr = QString::fromLocal8Bit(process_->readAllStandardOutput());
        logOut(outputStr);
    });
    connect(process_, &QProcess::readyReadStandardError, this, [this](){
        QString outputStr = QString::fromLocal8Bit(process_->readAllStandardError());
        logOut(outputStr);
    });
    connect(process_, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));

    QFileInfo fileInfo(app_->appPath());
    if (QFile::exists(fileInfo.absoluteFilePath())) {
        QScopedPointer<QProcess> process(new QProcess);
#ifdef Q_OS_WIN
        process->start("tasklist", QStringList() << "/FI imagename eq " + fileInfo.fileName());
#else
        process->start("sh", QStringList() << "-c" << QString("ps | grep '%1'").arg(fileInfo.fileName()));
#endif
        process->waitForFinished();
        QString outputStr = QString::fromLocal8Bit(process->readAllStandardOutput());
        if(outputStr.contains(fileInfo.fileName()) == false){
            on_StartUp_clicked();
            QTimer::singleShot(0, this, &MainWindow::hide);
        }
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
    connect(action, &QAction::triggered, this, [this](){ killApp(true); });
    menu->addAction(action);
}


void MainWindow::killApp(bool all)
{
    if (process_->isOpen()) {
        process_->close();
        process_->terminate();
    }

    if (all) {
        QFileInfo fileInfo(app_->appPath());
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


void MainWindow::logOut(const QString &log)
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


void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    QMetaEnum meta = QMetaEnum::fromType<QProcess::ExitStatus>();
    logOut(QString("exit code[%1], status[%2]").arg(exitCode).arg(meta.valueToKey(status)));
    ui->StartUp->setText(QStringLiteral("启动"));
}


void MainWindow::on_SelectApp_clicked()
{
    QString path =  QFileDialog::getOpenFileName(this, QStringLiteral("选择启动程序"));
    if (path.isEmpty()) {
        return ;
    }
    ui->AppText->setText(path);
    app_->setApp(path);
    logOut(app_->appData());
}


void MainWindow::on_ArgsPassword_clicked()
{
    logOut(isOpenPassword() ? QStringLiteral("恢复普通参数状态") : QStringLiteral("进入加密参数状态"));
    QString color(isOpenPassword() ? PASSWORD_CLOSE : PASSWORD_OPEN);
    ui->ArgsPassword->setStyleSheet(color);
}


void MainWindow::on_ArgsAdd_clicked()
{
    QString method("password");
    if (isOpenPassword() == false) {
        method.clear();
    }
    QString option = ui->ArgsOption->text();
    QString value = ui->ArgsValue->text();
    app_->addArgs(option, value, method);
    logOut(app_->appData());
}


void MainWindow::on_ReArgs_clicked()
{
    app_->cleanArgs();
    logOut(app_->appData());
}


void MainWindow::on_Save_clicked()
{
    if (app_->appPath().isEmpty()) {
        logOut(QStringLiteral("错误: 应用程序路径未设置"));
        return ;
    }
    app_->save();
    logOut(app_->appData());
    logOut(QStringLiteral("数据保存到: ") + app_->getPath());
}


void MainWindow::on_StartUp_clicked()
{
    if (ui->StartUp->text() == QStringLiteral("启动")) {
        QString path = app_->appPath();
        if (path.isEmpty()) {
            return ;
        }
        process_->start(path, app_->appArgs());
        ui->StartUp->setText(QStringLiteral("停止"));
    } else {
        process_->kill();
        process_->close();
        ui->StartUp->setText(QStringLiteral("启动"));
    }
}
