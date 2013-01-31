/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtNetwork>
#include <QNetworkProxy>
#include <windows.h>
#include <QImage>

#include "httpwindow.h"
#include "ui_authenticationdialog.h"

QString index_filename = "D:/index.html";
QString bing_filename = "D:/bing.jpg";

QString bing_url_ch = "http://www.bing.com/?cc=cn";
QString bing_url_us = "http://www.bing.com/?cc=us";
QString bing_url_au = "http://www.bing.com/?cc=au";
QString bing_url_de = "http://www.bing.com/?cc=de";
#define MAX_URL_NUMBER 4
QString bing_url;

QString no_pic_hint = "Are you kidding?";

qint8 Url_index = 0;

HttpWindow::HttpWindow(QWidget *parent)
#ifdef Q_WS_MAEMO_5
    : QWidget(parent)
#else
    : QDialog(parent)
#endif
{
#ifndef QT_NO_OPENSSL
    urlLineEdit = new QLineEdit("https://qt.nokia.com/");
#else
    urlLineEdit = new QLineEdit("http://qt.nokia.com/");
#endif

    urlLabel = new QLabel(tr("&URL:"));

    urlLabel->setBuddy(urlLineEdit);
//    statusLabel = new QLabel(tr("Please enter the URL of a file you want to download."));
    statusLabel = new QLabel(tr("Don't Like this pic?"));
//    statusLabel->setWordWrap(true);

    downloadButton = new QPushButton("Set DeskTop");
    downloadButton->setDefault(true);
    quitButton = new QPushButton("Quit");
    quitButton->setAutoDefault(false);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(downloadButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

#ifndef Q_WS_MAEMO_5
    progressDialog = new QProgressDialog(this);
    progressDialog->hide();
#endif

    connect(urlLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableDownloadButton()));

    connect(&qnam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&qnam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif
#ifndef Q_WS_MAEMO_5
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
#endif
    connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(urlLabel);
    topLayout->addWidget(urlLineEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Desktop for CRH"));
    urlLineEdit->setFocus();

    proxy = new QNetworkProxy;
    proxy->setType(QNetworkProxy::HttpProxy);
    proxy->setHostName("138.120.252.81");
    proxy->setPort(80);
    proxy->setUser("");
    proxy->setPassword("");
    QNetworkProxy::setApplicationProxy(*proxy);

    bing_url = bing_url_ch;
    urlLineEdit->setText(bing_url);
    urlLineEdit->hide();
    urlLabel->hide();
    statusLabel->hide();
}

void HttpWindow::changeUrl(QString &bing_url)
{
    switch (Url_index)
    {
        case 0:
        {
            bing_url = bing_url_ch;
            break;
        }
        case 1:
        {
            bing_url = bing_url_us;
            break;
        }
        case 2:
        {
            bing_url = bing_url_au;
            break;
        }
        case 3:
        {
            bing_url = bing_url_de;
            break;
        }
    default :
        bing_url = bing_url_ch;
        Url_index = -1;
    }
    Url_index++;

}

void HttpWindow::startRequest(QUrl url)
{
    qDebug("Start Downloading...");
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),
            this, SLOT(httpFinished()));
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDataReadProgress(qint64,qint64)));
}

void HttpWindow::downloadFile()
{

    statusLabel->setText("Waiting for download pic ...");
    statusLabel->show();
    changeUrl(bing_url);
    urlLineEdit->setText(bing_url);

    url = urlLineEdit->text();
    QFile::remove(index_filename);



    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty())
        fileName = "D:/index.html";
    else
        fileName = "D:/bing.jpg";

    if (QFile::exists(fileName)) {
        if (QMessageBox::question(this, tr("HTTP"),
                                  tr("There already exists a file called %1 in "
                                     "the current directory. Overwrite?").arg(fileName),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
            == QMessageBox::No)
            return;
        QFile::remove(fileName);
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        return;
    }

#ifndef Q_WS_MAEMO_5
    progressDialog->setWindowTitle(tr("HTTP"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));
#endif
    downloadButton->setEnabled(false);

    // schedule the request
    httpRequestAborted = false;
    startRequest(url);
}

void HttpWindow::cancelDownload()
{
    statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    reply->abort();
    downloadButton->setEnabled(true);
}

void HttpWindow::httpFinished()
{
    if (httpRequestAborted) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();
#ifndef Q_WS_MAEMO_5
        progressDialog->hide();
#endif
        return;
    }
    qDebug("Download Finish.....");

#ifndef Q_WS_MAEMO_5
    progressDialog->hide();
#endif
    file->flush();
    file->close();


    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        file->remove();
        QMessageBox::information(this, tr("HTTP"),
                                 tr("Download failed: %1.")
                                 .arg(reply->errorString()));
        downloadButton->setEnabled(true);
    } else if (!redirectionTarget.isNull()) {        
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(this, tr("HTTP"),
                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            url = newUrl;
            reply->deleteLater();
            file->open(QIODevice::WriteOnly);
            file->resize(0);
            startRequest(url);
            return;
        }
    } else {
//        QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
//        statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));

    }

    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;


    if(FALSE == QFile::exists(bing_filename))
    {
        QString jpgUrl = "";
        if(TRUE == QFile::exists(index_filename))
        {
            RetriveJpgUrlFromFile(index_filename, jpgUrl);
            if(FALSE == jpgUrl.isEmpty())
            {
                file = new QFile(bing_filename);
                file->open(QIODevice::WriteOnly);
                startRequest(jpgUrl);
            }
        }

    }
    else
    {
        SetDesktopWallpaper(bing_filename);
        downloadButton->setEnabled(true);
        statusLabel->setText("Don't Like this pic?");
        QFile::remove(index_filename);
        QFile::remove(bing_filename);

        downloadButton->setText("Change Again");
        if(MAX_URL_NUMBER == Url_index )
        {
            statusLabel->setText(no_pic_hint);
        }
    }
}

void HttpWindow::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (file)
        file->write(reply->readAll());
    qDebug("Download Data...");
}

void HttpWindow::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;

#ifndef Q_WS_MAEMO_5
 //   progressDialog->setMaximum(totalBytes);
 //   progressDialog->setValue(bytesRead);
    progressDialog->hide();
#else
    Q_UNUSED(bytesRead);
    Q_UNUSED(totalBytes);
#endif
}

void HttpWindow::enableDownloadButton()
{
    downloadButton->setEnabled(!urlLineEdit->text().isEmpty());
}

void HttpWindow::slotAuthenticationRequired(QNetworkReply*,QAuthenticator *authenticator)
{
    QDialog dlg;
    Ui::Dialog ui;
    ui.setupUi(&dlg);
    dlg.adjustSize();
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(url.userName());
    ui.passwordEdit->setText(url.password());

    if (dlg.exec() == QDialog::Accepted) {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }
}

#ifndef QT_NO_OPENSSL
void HttpWindow::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
    QString errorString;
    foreach (const QSslError &error, errors) {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }
    
    if (QMessageBox::warning(this, tr("HTTP"),
                             tr("One or more SSL errors has occurred: %1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
        reply->ignoreSslErrors();
    }
}
#endif

bool HttpWindow::RetriveJpgUrlFromFile(QString index_file, QString &jpgUrl)
{

    QString begin_tag("g_img={url:'");

    QFile file(index_file);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("Cann't open file for read");
        return FALSE;
    }
    QTextStream in(&file);
    while(!in.atEnd())
    {
        qint16 i_begin = 0;
        qint16 i_end = 0;
        QString line = in.readLine();
        i_begin = line.indexOf(begin_tag,0);
        if(-1 != i_begin)
        {
            i_begin = line.indexOf("'", i_begin);
            i_end = line.indexOf("'", i_begin + 1);
            jpgUrl = line.mid(i_begin + 1, i_end - i_begin - 1);
            jpgUrl.insert(0,"http://www.bing.com");
            qDebug(jpgUrl.toAscii());
            break;
        }

    }
    qDebug(jpgUrl.toAscii());
    file.close();
    return TRUE;
}

bool HttpWindow::ConverBmp2Jpg(QString jpg_FileName,QString &bmp_FileName)
{
    QImage img(jpg_FileName);

    jpg_FileName.remove(jpg_FileName.size() - 4 ,4);
    bmp_FileName = jpg_FileName + ".bmp";
    qDebug()<<"bing pic = "<<bmp_FileName;
    if (FALSE == img.save(bmp_FileName,"bmp", 100))
    {
        qDebug("Sorry, convert failed!");
        return FALSE;
    }
    qDebug("convert pic OK!");
    return TRUE;
}

bool HttpWindow::SetDesktopWallpaper(QString fileName)
{
    DWORD rc = 0;
    qDebug() << "set desktop wallpaper ...";
    if(TRUE == fileName.endsWith(".jpg"))
    {
       QString bmp_FileName = "";
       if(FALSE == ConverBmp2Jpg(fileName, bmp_FileName))
       {
           return false;
       }
       fileName = bmp_FileName;
       qDebug()<<fileName;
    }

    wchar_t * encodedName = (wchar_t *)(fileName.utf16());

    if(SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)encodedName, SPIF_UPDATEINIFILE) == FALSE)
    {
        qDebug() << "set desktop wallpaper Failed!";
        rc = GetLastError();
        qDebug()<<rc;
        return FALSE;
    }
    qDebug() << "set desktop wallpaper OK!";
    return TRUE;
}
