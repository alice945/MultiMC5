/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LoginDialog.h"
#include "ui_LoginDialog.h"

#include "minecraft/auth/YggdrasilTask.h"

#include <QtWidgets/QPushButton>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

// Stage 1: User interaction
void LoginDialog::accept()
{
    setUserInputsEnabled(false);
    ui->progressBar->setVisible(true);

    if (!ui->passTextBox->text().isEmpty()) {
        // Online Mode
        // Setup the login task and start it
        m_account = MojangAccount::createFromUsername(ui->userTextBox->text());
        m_loginTask = m_account->login(nullptr, ui->passTextBox->text());
        connect(m_loginTask.get(), &Task::failed, this, &LoginDialog::onTaskFailed);
        connect(m_loginTask.get(), &Task::succeeded, this,
                &LoginDialog::onTaskSucceeded);
        connect(m_loginTask.get(), &Task::status, this, &LoginDialog::onTaskStatus);
        connect(m_loginTask.get(), &Task::progress, this, &LoginDialog::onTaskProgress);
        m_loginTask->start();
    }
    else {
        // Offline Mode / No Password
        username = ui->userTextBox->text();
        QUrl uuidAPI = QUrl("https://api.mojang.com/users/profiles/minecraft/" + username);

        auto job = new NetJob("Get Player UUID");
        job->addNetAction(Net::Download::makeByteArray(uuidAPI, &uuidData));
        connect(job, &NetJob::succeeded, this, &LoginDialog::uuidCheckFinished);
        connect(job, &NetJob::failed, this, &LoginDialog::uuidCheckFailed);
        job->start();
    }
}

void LoginDialog::uuidCheckFinished() {
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(uuidData, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        qCritical() << "Failed to parse UUID request. JSON error"
                    << jsonError.errorString() << "at offset" << jsonError.offset;
        QString rndUUID = QCryptographicHash::hash(username.toLocal8Bit(), QCryptographicHash::Md5).toHex();
        m_account = MojangAccount::createFromUsernameOffline(username, rndUUID);
        QDialog::accept();
    } else {
        QJsonObject object = jsonDoc.object();
        if (object.value("name").toVariant().toString() != username) {
            qCritical() << "UUID Json has different username than requested:"
                        << object.value("player").toVariant().toString();
        }
        m_account = MojangAccount::createFromUsernameOffline(username, object.value("id").toVariant().toString());
        QDialog::accept();
    }
}

void LoginDialog::uuidCheckFailed() {
    qCritical() << "UUID json get failed.";
    QString rndUUID = QCryptographicHash::hash(username.toLocal8Bit(), QCryptographicHash::Md5).toHex();
    m_account = MojangAccount::createFromUsernameOffline(username, rndUUID);
    QDialog::accept();
}

void LoginDialog::setUserInputsEnabled(bool enable)
{
    ui->userTextBox->setEnabled(enable);
    ui->passTextBox->setEnabled(enable);
    ui->buttonBox->setEnabled(enable);
}

// Enable the OK button only when both textboxes contain something.
void LoginDialog::on_userTextBox_textEdited(const QString &newText)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(!newText.isEmpty());
}
void LoginDialog::on_passTextBox_textEdited(const QString &newText)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(!ui->userTextBox->text().isEmpty());
}

void LoginDialog::onTaskFailed(const QString &reason)
{
    // Set message
    ui->label->setText("<span style='color:red'>" + reason + "</span>");

    // Re-enable user-interaction
    setUserInputsEnabled(true);
    ui->progressBar->setVisible(false);
}

void LoginDialog::onTaskSucceeded()
{
    QDialog::accept();
}

void LoginDialog::onTaskStatus(const QString &status)
{
    ui->label->setText(status);
}

void LoginDialog::onTaskProgress(qint64 current, qint64 total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(current);
}

// Public interface
MojangAccountPtr LoginDialog::newAccount(QWidget *parent, QString msg)
{
    LoginDialog dlg(parent);
    dlg.ui->label->setText(msg);
    if (dlg.exec() == QDialog::Accepted)
    {
        return dlg.m_account;
    }
    return 0;
}
