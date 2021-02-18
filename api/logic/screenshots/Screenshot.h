#pragma once

#include <QDateTime>
#include <QString>
#include <QFileInfo>
#include <memory>

struct ScreenShot
{
    ScreenShot(QFileInfo file)
    {
        m_file = file;
    }
    QFileInfo m_file;
    QString m_url;
};

typedef std::shared_ptr<ScreenShot> ScreenshotPtr;
