#include "com_func.h"

QString toHexString(QByteArray a)
{
    QString res = a.toHex().toUpper();
    int pos = res.indexOf(" ");
    if (pos < 0) pos = res.length();
    while (pos > 2)
    {
        res.insert(pos-2," ");
        pos = res.indexOf(" ");
    }
    return res;
}

void delay_ms(uint32_t delay)
{
    QTimer t;
    t.setInterval(delay);
    QEventLoop loop;
    QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start();
    loop.exec();
}
