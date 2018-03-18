#include "UpdateChecker.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QDesktopServices>
#include <qmessagebox.h>


UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent)
{
}


void UpdateChecker::checkUpdate(int version)
{
	version_ = version;
	connect(&net_manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(netRequestFinished(QNetworkReply*)));
	net_manager_.get(QNetworkRequest(QUrl(releases_url)));
}

void UpdateChecker::netRequestFinished(QNetworkReply* reply) const
{
	if (!reply->error())
	{
		auto json = QJsonDocument::fromJson(reply->readAll());
		if (!json.isNull() && json.isArray())
		{
			auto last_release_object = json.array()[0].toObject();

			auto tag = last_release_object.take("tag_name").toString();
			auto tag_substrings = tag.split('.');
			if (tag_substrings.length() >= 3)
			{
				int version = 0;
				version = (version << 4) + static_cast<char>(std::atoi(tag_substrings[0].toStdString().data()));
				version = (version << 4) + static_cast<char>(std::atoi(tag_substrings[1].toStdString().data()));
				version = (version << 4) + static_cast<char>(std::atoi(tag_substrings[2].toStdString().data()));

				if (version > version_)
				{
					if (QMessageBox::information(nullptr, 
						"GloSC", "There is a new version available!\nDownload now?",
						QMessageBox::Yes | QMessageBox::No)
						== QMessageBox::Yes)
					{
						QDesktopServices::openUrl(QUrl(last_release_object.take("html_url").toString()));
					}
				}
			}
		}
	}
}
