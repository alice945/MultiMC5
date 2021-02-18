#pragma once

#include <QWidget>

namespace GuiUtil
{
void setClipboardText(const QString &text);
QStringList BrowseForFiles(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget);
QString BrowseForFile(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget);
}
