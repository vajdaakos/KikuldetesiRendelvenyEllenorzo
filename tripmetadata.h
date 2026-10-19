#ifndef TRIPMETADATA_H
#define TRIPMETADATA_H

#include <QString>
#include <QtTypes>

class TripMetaData
{
public:
    QString m_serial_number;
    QString m_employee;
    quint32 m_employee_id = 0;
    QString m_filename;
};

#endif
