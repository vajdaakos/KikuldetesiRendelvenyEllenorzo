#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include "columnheaders.h"

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject &parent);

    const HeaderSettings &columnHeaders() const { return m_columnHeaders; }
    const HeaderSettings &metadataHeaders() const { return m_metadataHeaders; }

signals:
    void settingsLoaded(const HeaderSettings &columnHeaders,
                        const HeaderSettings &metadataHeaders);

public slots:
    void readSettings();
    void applySettings(const HeaderSettings &columnHeaders,
                       const HeaderSettings &metadataHeaders);

private:

    HeaderSettings m_columnHeaders = kDefaultColumnHeaders;
    HeaderSettings m_metadataHeaders = kDefaultMetadataHeaders;
};

#endif
