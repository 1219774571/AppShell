#ifndef APPCONSOLE_H
#define APPCONSOLE_H

#include <QObject>
#include <QVector>

class AppConsole : public QObject
{
    Q_OBJECT
public:
    struct argsFormat{
        QString option;
        QString value;
        QString method;
        argsFormat(){}
        argsFormat(const QString &o, const QString &v, const QString &m) :option(o), value(v), method(m) {}
        bool operator ==(const argsFormat &f) { return option == f.option; }
    };

    explicit AppConsole(const QString &path, QObject *parent = nullptr);

    /**
     * @brief setApp 设置应用程序路径
     * @param app 应用路径
     * @return 成功返回true, 反之返回false
     */
    bool setApp(const QString &app);

    /**
     * @brief addArgs 添加参数
     * @param option 选项
     * @param arg 参数
     */
    void addArgs(const QString &option, const QString &arg, const QString &method = QString());

    /**
     * @brief cleanArgs 清空所有参数
     */
    void cleanArgs() { args_.clear(); }

    /**
     * @brief appPath 获取应用路径
     * @return 应用路径
     */
    QString appPath() const { return app_; }

    /**
     * @brief appArgs 获取解封后的参数列表
     * @return 解封装后的参数
     */
    QStringList appArgs() const;

    /**
     * @brief appData 获取应用路径和参数组合的数据
     * @return 返回封装后的所有数据
     */
    QString appData() const;

    QString getPath() const {return path_; }
    void setPath(const QString &path) { path_ = path; }

    void save() const;

    void restore();

private:
    void packageArgsFormat(argsFormat &arg) const;
    void unpackageArgsFormat(argsFormat &arg) const;

    QByteArray encrypt(QByteArray data, const QByteArray &key) const;
    QByteArray decrypt(QByteArray data, const QByteArray &key) const;

private:
    QByteArray key_;
    QString path_;
    QString app_;
    QVector<argsFormat> args_;
};

#endif // APPCONSOLE_H
