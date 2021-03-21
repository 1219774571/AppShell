

#ifndef ARGSFORMATDATA_H
#define ARGSFORMATDATA_H


#include <QObject>
#include <QDomElement>
#include <QProcess>
#include <QSharedPointer>


struct nArgsFormat;
struct nArgsFormatBase;

struct ArgsFormatBase {
    enum mMethod{
        kNone,      ///< 常规
        kPassword,  ///< 加密
    };

    QString option; ///< 参数选项
    QString value;  ///< 参数值
    mMethod method; ///< 模式

    ArgsFormatBase() :method(kNone){}
    explicit ArgsFormatBase(const nArgsFormatBase &args) { this->Conver(args); }
    ArgsFormatBase(const QString &option_, const QString &value_, const mMethod &method_) :option(option_), value(value_), method(method_) {}

    /// 设置了method后需要更新数据
    void UpdateMethod(const QByteArray &key);
    bool operator==(const ArgsFormatBase &f) { return option == f.option; }
    ArgsFormatBase& operator=(const nArgsFormatBase &args) { this->Conver(args); return *this; }
    void Conver(const nArgsFormatBase &args);
    QStringList Args(const QByteArray &key) const;
    QStringList Args() const;
};

struct nArgsFormatBase {
    QString option; ///< 参数选项
    QString value;  ///< 参数值
    QString method; ///< 模式

    nArgsFormatBase() :method("kNone"){}
    explicit nArgsFormatBase(const ArgsFormatBase &args) { this->Conver(args); }
    nArgsFormatBase(const QString &option_, const QString &value_, const QString &method_) :option(option_), value(value_), method(method_) {}
    nArgsFormatBase& operator=(const ArgsFormatBase &args) { this->Conver(args); return *this; }
    void Conver(const ArgsFormatBase &args);
};


struct ArgsFormat {
    bool    enable;                 ///< 启用
    QString path;                   ///< 应用程序路径
    QVector<ArgsFormatBase> args;   ///< 参数组

    ArgsFormat() :enable(false){}
    explicit ArgsFormat(const nArgsFormat &args) { this->Conver(args); }
    ArgsFormat(const QString &path_, bool enable_) :enable(enable_), path(path_) {}
    ArgsFormat(const QString &path_, const ArgsFormatBase& args_, bool enable_) :enable(enable_), path(path_), args({args_}) {}
    QStringList Args(const QByteArray &key) const;
    QStringList Args() const;
    ArgsFormat& operator=(const nArgsFormat &args) { this->Conver(args); return *this; }
    void Conver(const nArgsFormat &args);
};

struct nArgsFormat {
    QString enable;
    QString path;
    QVector<ArgsFormatBase> args;

    nArgsFormat() :enable("false"){}
    explicit nArgsFormat(const ArgsFormat &args){ this->Conver(args); }

    /**
     * @brief Parse 解析xml数据赋值给成员
     * @param element xml元素对象
     */
    void ParseXml(const QDomElement &element);

    /**
     * @brief Create 创建xml元素
     * @param doc xml文档
     * @param root root节点
     */
    void CreateXml(QDomDocument &doc, QDomElement &root) const;

    nArgsFormat& operator=(const ArgsFormat &args) { this->Conver(args); return *this; }
    void Conver(const ArgsFormat &args);
};


struct AppArgsStruct {
    QSharedPointer<QProcess> process;
    ArgsFormat args;
    AppArgsStruct() :process(new QProcess){}
    explicit AppArgsStruct(const ArgsFormat &args_) :process(new QProcess), args(args_){}
    ~AppArgsStruct() {
        if (process) {
            process->kill();
        }
    }
};

#endif // ARGSFORMATDATA_H
