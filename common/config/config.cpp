#include <QDateTime>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QStringList>
#include "config.h"

static QSettings* cfg = 0;
static QDateTime lastSync;

QConfig::QConfig(QObject *parent)
    : QObject(parent)
{

}

QConfig::~QConfig()
{
    if (cfg)
        cfg->deleteLater();
}

void QConfig::init(QString path, QString fileName)
{
    QDir d(path);
    cfg = new QSettings(d.absoluteFilePath(fileName), QSettings::IniFormat);
    cfg->sync();
    lastSync = QDateTime::currentDateTime();
}

QVariant QConfig::getValue(QString key)
{
    if (!cfg) return QVariant();
    if (lastSync.msecsTo(QDateTime::currentDateTime()) > SYNC_LIFETIME)
    {
        cfg->sync();
        lastSync = QDateTime::currentDateTime();
    }
    return cfg->value(key);
}

void QConfig::setValue(QString key, QVariant value)
{
    if (!cfg) return;
    cfg->setValue(key, value);
    cfg->sync();
    lastSync = QDateTime::currentDateTime();
}

bool QConfig::isKeyExists(QString key)
{
    if (!cfg) return false;
    QVariant v = cfg->value(key);
    return !v.isNull();
}

QStringList QConfig::sections(QRegExp allowed)
{
    QStringList res;

    if (cfg)
    {
        if (lastSync.msecsTo(QDateTime::currentDateTime()) > SYNC_LIFETIME)
        {
            cfg->sync();
            lastSync = QDateTime::currentDateTime();
        }

        while (!cfg->group().isEmpty())
            cfg->endGroup();

        res = cfg->childGroups().filter(allowed);
    }

    return res;
}

QStringList QConfig::keys(QString section, QRegExp allowed)
{
    QStringList res;

    if (cfg)
    {
        if (lastSync.msecsTo(QDateTime::currentDateTime()) > SYNC_LIFETIME)
        {
            cfg->sync();
            lastSync = QDateTime::currentDateTime();
        }

        while (!cfg->group().isEmpty())
            cfg->endGroup();

        cfg->beginGroup(section);
        res = cfg->childKeys().filter(allowed);
        cfg->endGroup();
    }

    return res;
}
