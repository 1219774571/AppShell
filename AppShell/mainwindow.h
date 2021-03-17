

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QMap>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class AppConsole;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void init();
    void initSystemTray();
    void initProcess();

    void createMenu();

    void killApp(bool all = false);

    /** 加密参数值是否开启 */
    bool isOpenPassword() const;
    void logOut(const QString &log);

    /** 获取, 设置, 删除 程序开机启动值 */
    QString getProcessAutoRun(const QString &path) const;
    bool setProcessAutoRun(const QString &path) const;
    bool delProcessAutoRun(const QString &path) const;

private slots:
    void activeTray(QSystemTrayIcon::ActivationReason reason);

    void processFinished(int exitCode, QProcess::ExitStatus status);

    void on_SelectApp_clicked();
    void on_ArgsPassword_clicked();
    void on_ArgsAdd_clicked();
    void on_ReArgs_clicked();
    void on_Save_clicked();
    void on_StartUp_clicked();

private:
    Ui::MainWindow *ui;
    AppConsole *app_;
    QSystemTrayIcon *systemTray_;
    QProcess *process_;
    QMap<QString, QAction *> actionMap_;
};


#endif // MAINWINDOW_H
