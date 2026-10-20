#ifndef ERRORSTORAGE_H
#define ERRORSTORAGE_H

#include "trip.h"
#include "tripmetadata.h"

#include <QRangeModel>
#include <QVector>

class ErrorStorage : public QRangeModel
{
    Q_OBJECT

public:
    explicit ErrorStorage(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QVector<QVector<const Trip*>> &errors();
    const QVector<const TripMetaData *> &fileMetadata() const;
    void setFileMetadata(const QVector<TripMetaData> &metadata);
    void clear();
    void beginUpdate();
    void endUpdate();

    bool showPreviousFile();
    bool showNextFile();
    bool showFileAt(qsizetype index);
    bool hasErrorAt(qsizetype index) const;
    bool isError(qsizetype file, const Trip *trip) const;
    bool hasErrors() const;
    qsizetype currentFile() const;
    const Trip *sourceAt(int row) const;
    bool hasPrevious() const;
    bool hasNext() const;

private:
    void showFile(qsizetype index);
    void rebuildRows();
    bool fileHasError(qsizetype index) const;
    qsizetype previousErrorFile() const;
    qsizetype nextErrorFile() const;

    QVector<QVector<const Trip*>> m_triperrors;
    QVector<const TripMetaData*>  m_file_metadata;
    QVector<Trip> m_rows;
    qsizetype m_currentFile = -1;
};

#endif
