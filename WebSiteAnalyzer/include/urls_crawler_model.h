#pragma once

#include "stdlibs.h"
#include "crawler_model.h"

namespace WebSiteAnalyzer
{

class UrlsCrawlerModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	UrlsCrawlerModel(QObject* parent = nullptr);
	
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	Q_SLOT void slot_addUrl(const std::string& url, 
		const std::string& title, 
		const std::string& description, 
		const std::string& charset, 
		int responseCode
	);

private:
	std::deque<std::array<QString, 4>> m_urls;
	QStringList m_headerData;
};

}
