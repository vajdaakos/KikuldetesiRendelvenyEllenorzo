#include "tripmetadatawidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>

TripMetadataWidget::TripMetadataWidget(QWidget *parent)
    : QWidget(parent)
    , m_serialLabel(createLabel())
    , m_employeeLabel(createLabel())
    , m_employeeIdLabel(createLabel())
    , m_fileNameLabel(createLabel())
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_serialLabel);
    layout->addWidget(m_employeeLabel);
    layout->addWidget(m_employeeIdLabel);
    layout->addWidget(m_fileNameLabel);
    setMetadata({});
}

void TripMetadataWidget::setMetadata(const TripMetaData &metadata)
{
    const bool loaded = !metadata.m_filename.isEmpty();
    m_serialLabel->setText(tr("Sorszám: %1").arg(metadata.m_serial_number));
    m_employeeLabel->setText(tr("Munkavállaló: %1").arg(metadata.m_employee));
    m_employeeIdLabel->setText(tr("Törzsszám: %1").arg(loaded ? QString::number(metadata.m_employee_id) : QString()));
    m_fileNameLabel->setText(tr("Fájlnév: %1").arg(metadata.m_filename));
}

QLabel *TripMetadataWidget::createLabel()
{
    auto *label = new QLabel(this);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    return label;
}
