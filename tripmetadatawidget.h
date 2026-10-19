#ifndef TRIPMETADATAWIDGET_H
#define TRIPMETADATAWIDGET_H

#include "tripmetadata.h"

#include <QWidget>

class QLabel;

class TripMetadataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TripMetadataWidget(QWidget *parent = nullptr);

    void setMetadata(const TripMetaData &metadata);

private:
    QLabel *createLabel();

    QLabel *m_serialLabel = nullptr;
    QLabel *m_employeeLabel = nullptr;
    QLabel *m_employeeIdLabel = nullptr;
    QLabel *m_fileNameLabel = nullptr;
};

#endif
