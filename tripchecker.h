#ifndef TRIPCHECKER_H
#define TRIPCHECKER_H

#include "trip.h"

#include <QVector>

class TripChecker
{
public:
    void check(const QVector<QVector<Trip>> &trips, QVector<QVector<const Trip*>> &errors);
};

#endif
