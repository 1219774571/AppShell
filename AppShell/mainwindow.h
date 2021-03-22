

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

    /**
     * @brief killApp 两者为false时, 杀死当前选择的程序
     * @param all 杀死程序内的全部
     * @param sys 杀死系统内相关的全部
     */
    void killApp(bool all = true, bool sys = false);

    /** 加密参数值是否开启 */
    bool isOpenPassword() const;
    void LogOut(const QString &log);

    /** 获取, 设置, 删除 程序开机启动值 */
    QString getProcessAutoRun(const QString &path) const;
    bool setProcessAutoRun(const QString &path) const;
    bool delProcessAutoRun(const QString &path) const;

    /**
     * @brief GetAppSelectNum 获取路径前面有多少个相同的
     * @param path 路径
     * @param index 位置
     * @return 找不到返回0
     */
    int GetAppSelectNum(const QString &path, int index);

    void UpdateSelectApp();

private slots:
    void activeTray(QSystemTrayIcon::ActivationReason reason);

    void on_ArgsPassword_clicked();
    void on_ReArgs_clicked();
    void on_Save_clicked();
    void on_StartUp_clicked();
    void on_AddApp_clicked();
    void on_DelApp_clicked();
    void on_AddArgs_clicked();
    void on_DelArgs_clicked();
    void on_AllStartUp_clicked();
    void on_AppSelect_currentIndexChanged(const QString &arg1);
    void on_ModifyApp_clicked();

private:
    Ui::MainWindow *ui;
    AppConsole *app_;
    QSystemTrayIcon *systemTray_;
    QMap<QString, QAction *> actionMap_;
};


#endif // MAINWINDOW_H
