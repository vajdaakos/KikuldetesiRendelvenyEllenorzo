#ifndef COLUMNHEADERS_H
#define COLUMNHEADERS_H

#include <QCoreApplication>
#include <QStringList>

inline QStringList tripColumnHeaders()
{
    return {
        QCoreApplication::translate("ColumnHeaders", "Azonosító"),
        QCoreApplication::translate("ColumnHeaders", "Forrásfájl"),
        QCoreApplication::translate("ColumnHeaders", "Kezdés"),
        QCoreApplication::translate("ColumnHeaders", "Befejezés"),
        QCoreApplication::translate("ColumnHeaders", "Cél"),
        QCoreApplication::translate("ColumnHeaders", "Honnan"),
        QCoreApplication::translate("ColumnHeaders", "Hova"),
        QCoreApplication::translate("ColumnHeaders", "Távolság"),
        QCoreApplication::translate("ColumnHeaders", "Üzemanyagár"),
        QCoreApplication::translate("ColumnHeaders", "Üzemanyagköltség"),
        QCoreApplication::translate("ColumnHeaders", "Fenntartási szorzó 1"),
        QCoreApplication::translate("ColumnHeaders", "Fenntartási szorzó 2"),
        QCoreApplication::translate("ColumnHeaders", "Összesen"),
    };
}

inline QStringList reportColumnHeaders()
{
    QStringList headers{
        QCoreApplication::translate("ColumnHeaders", "Fájlnév"),
        QCoreApplication::translate("ColumnHeaders", "Sorszám"),
        QCoreApplication::translate("ColumnHeaders", "Munkavállaló"),
        QCoreApplication::translate("ColumnHeaders", "Törzsszám"),
    };
    headers += tripColumnHeaders();
    return headers;
}

#endif
