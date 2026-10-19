#include "tripchecker.h"

void TripChecker::check(const QVector<QVector<Trip>> &trips, QVector<QVector<const Trip*>> &errors)
{
    errors = QVector<QVector<const Trip*>>(trips.size());
    for (qsizetype fileIndex = 0; fileIndex < trips.size(); ++fileIndex) {
        if (trips.at(fileIndex).isEmpty())
            continue;
        for (qsizetype rowindex = 0; rowindex < trips.at(fileIndex).size(); ++rowindex) {
            const Trip &record = trips.at(fileIndex).at(rowindex);
            if (record.m_from == record.m_to && record.m_total_cost > 0)
                errors[fileIndex].append(&trips.at(fileIndex).at(rowindex));
        }
    }
}
