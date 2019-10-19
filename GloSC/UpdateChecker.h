/*
Copyright 2018-2019 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>

class UpdateChecker : public QObject
{
	Q_OBJECT
public:
	explicit UpdateChecker(QObject *parent = nullptr);

	void checkUpdate(int version);

public slots:
	void netRequestFinished(QNetworkReply* reply) const;

private:

	QNetworkAccessManager net_manager_;
	int version_ = 0;
	constexpr static const char* releases_url = "https://api.github.com/repos/Alia5/GloSC/releases";
};

