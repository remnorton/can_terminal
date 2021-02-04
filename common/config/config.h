#ifndef QCONFIG_H
#define QCONFIG_H

#include <QtCore/QObject>

//
//
//

#define SYNC_LIFETIME 5000  //! sync. perios (msec.)

//
//
//

class QConfig : public QObject
{
    Q_OBJECT
public:
    explicit QConfig(QObject *parent = 0);
    ~QConfig();

    static void init(QString path, QString fileName);
    static QVariant getValue(QString key);
    static void setValue(QString key, QVariant value);
    static bool isKeyExists(QString key);
    static QStringList sections(QRegExp allowed);
    static QStringList keys(QString section, QRegExp allowed);

signals:

public slots:
};

#endif // QCONFIG_H
