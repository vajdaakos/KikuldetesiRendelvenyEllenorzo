#ifndef COLUMNHEADERS_H
#define COLUMNHEADERS_H

#include <QCoreApplication>
#include <QStringList>
#include <QVector>

#include <utility>

using HeaderSettings = QVector<std::pair<QString,bool>>;

inline constexpr char kColumnHeaderContext[] = "ColumnHeaders";

inline const HeaderSettings kDefaultColumnHeaders{
    std::make_pair("Azonosító", true),
    std::make_pair("Forrásfájl", false),
    std::make_pair("Kezdés", true),
    std::make_pair("Befejezés", true),
    std::make_pair("Cél", true),
    std::make_pair("Honnan", true),
    std::make_pair("Hova", true),
    std::make_pair("Távolság", true),
    std::make_pair("Üzemanyagár", true),
    std::make_pair("Üzemanyagköltség", true),
    std::make_pair("Fenntartási szorzó 1", true),
    std::make_pair("Fenntartási szorzó 2", true),
    std::make_pair("Összesen", true)
};

inline const HeaderSettings kDefaultMetadataHeaders{
    std::make_pair("Fájlnév", false),
    std::make_pair("Sorszám", true),
    std::make_pair("Munkavállaló", true),
    std::make_pair("Törzsszám", true)
};

inline QStringList tripColumnHeaders()
{
    QStringList headers;
    for (const auto &header : kDefaultColumnHeaders) {
        headers.append(QCoreApplication::translate(kColumnHeaderContext, header.first.toUtf8().data()));
    }
    return headers;
}

#endif
