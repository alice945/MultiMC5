#include "GuiUtil.h"

#include <QClipboard>
#include <QApplication>
#include <QFileDialog>

#include "dialogs/ProgressDialog.h"
#include "dialogs/CustomMessageBox.h"

#include "MultiMC.h"
#include <settings/SettingsObject.h>
#include <DesktopServices.h>
#include <BuildConfig.h>

void GuiUtil::setClipboardText(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

static QStringList BrowseForFileInternal(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget, bool single)
{
    static QMap<QString, QString> savedPaths;

    QFileDialog w(parentWidget, caption);
    QSet<QString> locations;
    auto f = [&](QStandardPaths::StandardLocation l)
    {
        QString location = QStandardPaths::writableLocation(l);
        QFileInfo finfo(location);
        if (!finfo.exists())
            return;
        locations.insert(location);
    };
    f(QStandardPaths::DesktopLocation);
    f(QStandardPaths::DocumentsLocation);
    f(QStandardPaths::DownloadLocation);
    f(QStandardPaths::HomeLocation);
    QList<QUrl> urls;
    for (auto location : locations)
    {
        urls.append(QUrl::fromLocalFile(location));
    }
    urls.append(QUrl::fromLocalFile(defaultPath));

    w.setFileMode(single ? QFileDialog::ExistingFile : QFileDialog::ExistingFiles);
    w.setAcceptMode(QFileDialog::AcceptOpen);
    w.setNameFilter(filter);

    QString pathToOpen;
    if(savedPaths.contains(context))
    {
        pathToOpen = savedPaths[context];
    }
    else
    {
        pathToOpen = defaultPath;
    }
    if(!pathToOpen.isEmpty())
    {
        QFileInfo finfo(pathToOpen);
        if(finfo.exists() && finfo.isDir())
        {
            w.setDirectory(finfo.absoluteFilePath());
        }
    }

    w.setSidebarUrls(urls);

    if (w.exec())
    {
        savedPaths[context] = w.directory().absolutePath();
        return w.selectedFiles();
    }
    savedPaths[context] = w.directory().absolutePath();
    return {};
}

QString GuiUtil::BrowseForFile(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget)
{
    auto resultList = BrowseForFileInternal(context, caption, filter, defaultPath, parentWidget, true);
    if(resultList.size())
    {
        return resultList[0];
    }
    return QString();
}


QStringList GuiUtil::BrowseForFiles(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget)
{
    return BrowseForFileInternal(context, caption, filter, defaultPath, parentWidget, false);
}
