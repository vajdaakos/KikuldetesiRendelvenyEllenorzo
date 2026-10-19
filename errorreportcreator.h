#ifndef ERRORREPORTCREATOR_H
#define ERRORREPORTCREATOR_H

#include "trip.h"
#include "tripmetadata.h"

#include <QRangeModel>
#include <QStringList>
#include <QVector>

class ErrorReportCreator : public QRangeModel
{
    Q_OBJECT

public:
    explicit ErrorReportCreator(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void create(const QVector<QVector<Trip>> &trips,
                const QVector<QVector<const Trip*>> &errors,
                const QVector<const TripMetaData *> &metadata,
                const QString &directoryPath);

private:
    QVector<QStringList> m_rows;
};

#endif
