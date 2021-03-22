#ifndef APPCONSOLE_H
#define APPCONSOLE_H

#include <QObject>
#include <QVector>

#include "ArgsFormatData.h"


class AppConsole : public QObject
{
    Q_OBJECT
public:

    explicit AppConsole(const QString &path, QObject *parent = nullptr);

    /**
     * @brief AddApp 添加应用程序
     * @param app 应用路径
     * @param option 选项, 如果相同选项则替换
     * @param value 参数值
     * @param method 方法
     * @param enable 启用
     * @param index 修改第几个相同的[0, max), 超出则修改最后一个找到的, 未找到或-1则进行添加
     */
    void AddApp(const QString &appPath, const QString &option = QString(), const QString &value = QString(), const ArgsFormatBase::mMethod method = ArgsFormatBase::kNone, bool enable = true, int index = -1);

    /**
     * @brief DelApp 删除应用程序
     * @param appPath 应用路径
     * @param index 删除第几个相同的[0, max), 超出则删除最后一个找到的, 0则删除第一个
     * @return 存在删除成功返回true, 不存在返回false
     */
    bool DelApp(const QString &appPath, int index = 0);

    /**
     * @brief ModifyAppPath 修改应用程序路径
     * @param oldPath 原路径
     * @param newPath 更新路径
     * @param index 更新第几个相同的[0, max), 超出则选择最后一个找到的, 0则第一个
     * @return 存在更新成功返回true, 不存在返回false
     */
    bool ModifyAppPath(const QString &oldPath, const QString &newPath, int index = 0);

    /**
     * @brief CleanArgs 清空应用程序的参数
     * @param appPath 应用路径
     * @param index 清空第几个相同的[0, max), 超出则清空最后一个找到的, 0则清空第一个
     * @return 存在清空成功返回true, 不存在返回false
     */
    bool CleanArgs(const QString &appPath, int index = 0);

    /**
     * @brief DelArgs 删除指定参数
     * @param appPath 应用参数
     * @param option 选项
     * @param index 删除第几个相同的[0, max), 超出则删除最后一个找到的, 0则删除第一个
     * @return 存在删除成功返回true, 不存在返回false
     */
    bool DelArgs(const QString &appPath, const QString &option, int index = 0);

    /**
     * @brief appArgs 获取解封后的参数列表
     * @param index 第几个相同的[0, max), 超出则返回最后一个找到的, 0则第一个
     * @return 解封装后的参数
     */
    QStringList AppArgs(const QString &appPath, int index = 0) const;

    /**
     * @brief appData 获取应用路径和参数组合的数据
     * @param index 第几个相同的[0, max), 超出则返回最后一个找到的, 0则第一个
     * @return 返回封装后的所有数据
     */
    QString AppData(const QString &appPath, int index = 0) const;

    /**
     * @brief GetPath 获取xml配置文件保存路径
     * @return 路径
     */
    QString GetPath() const {return path_; }

    /**
     * @brief SetPath 设置xml配置文件保存路径
     * @param path 路径
     */
    void SetPath(const QString &path) { path_ = path; }

    /**
     * @brief Save 保存数据到配置文件
     */
    void Save() const;

    /**
     * @brief Restore 配置文件还原数据
     */
    void Restore();

    /**
     * @brief Size 返回数据数量
     * @return 数量
     */
    int Size() const { return args_.size(); }

    /**
     * @brief GetArgsPath 获取参数的应用路径
     * @param i 索引
     * @return 不存在返回空, 存在返回应用程序路径
     */
    QString GetArgsPath(int i);

    void Start(const QString &appPath, int index = 0);
    void Stop(const QString &appPath, int index = 0);

    void StartAll();
    void StopAll();

    bool IsRun(const QString &appPath, int index = 0) const;
    bool IsAllRun() const;

private:

    /**
     * @brief GetArgsIndex 搜索args_是否存在该路径
     * @param path 应用路径
     * @param pos 起始位置
     * @return 存在返回当前索引, 不存在返回-1
     */
    int GetArgsIndex(const QString &path, int pos = 0) const;
    int GetArgsIndex(const QObject *obj) const;

    /**
     * @brief GetSameArgsIndex 获取相同路径的最后索引
     * @param path 路径
     * @param num 相同数量
     * @return 不存在返回-1, 数量不够则返回最后正确的位置
     */
    int GetSameArgsIndex(const QString &appPath, int num) const;

signals:
    void AppStandOut(const QString &path, const QString &log, int index);
    void AppErrorOut(const QString &path, const QString &log, int index);
    void AppStarted(const QString &path, int index);
    void AppExitStatus(const QString &path, int code, QProcess::ExitStatus status, int index);

private slots:
    void ParseStandOut();
    void ParseErrorOut();
    void ParseStarted();
    void ParseExited(int code, QProcess::ExitStatus status);

private:
    QByteArray key_;    ///< 加密密钥
    QString path_;      ///< 文件保存路径
    QVector<AppArgsStruct> args_;
};

#endif // APPCONSOLE_H
