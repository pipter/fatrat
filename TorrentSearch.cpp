#include "TorrentSearch.h"
#include "RuntimeException.h"
#include "fatrat.h"
#include "MainWindow.h"
#include <QDir>
#include <QFile>
#include <QDomDocument>
#include <QMessageBox>
#include <QHttp>
#include <QBuffer>
#include <QUrl>
#include <QMap>
#include <QRegExp>
#include <QTextDocument>
#include <QHeaderView>
#include <QTimer>
#include <QSettings>
#include <QtDebug>

extern QSettings* g_settings;

TorrentSearch::TorrentSearch()
	: m_bSearching(false)
{
	setupUi(this);
	
	connect(pushSearch, SIGNAL(clicked()), this, SLOT(search()));
	connect(treeResults, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
	
	loadEngines();
	
	g_settings->beginGroup("torrent_search");
	
	foreach(Engine e, m_engines)
	{
		QListWidgetItem* item = new QListWidgetItem(e.name, listEngines);
		bool checked;
		
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		
		checked = g_settings->value(e.name, false).toBool();
		
		item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
		listEngines->addItem(item);
	}
	
	g_settings->endGroup();
	
	QStringList hdr = QStringList() << tr("Name") << tr("Size") << tr("Seeders") << tr("Leechers") << tr("Source");
	treeResults->setHeaderLabels(hdr);
	treeResults->sortItems(3, Qt::DescendingOrder);
	
	QHeaderView* phdr = treeResults->header();
	phdr->resizeSection(0, 350);
	
	QTimer::singleShot(100, this, SLOT(setSearchFocus()));
}

TorrentSearch::~TorrentSearch()
{
	g_settings->beginGroup("torrent_search");
	
	for(int i=0;i<listEngines->count();i++)
	{
		QListWidgetItem* item = listEngines->item(i);
		g_settings->setValue(item->text(), item->checkState() == Qt::Checked);
	}
	
	g_settings->endGroup();
}

void TorrentSearch::setSearchFocus()
{
	lineExpr->setFocus(Qt::OtherFocusReason);
}

void TorrentSearch::loadEngines()
{
	QDomDocument doc;
	QFile file;
	QDir dir(DATA_LOCATION);
	
	dir.cd("data");
	file.setFileName( dir.absoluteFilePath("btsearch.xml") );
	
	if(!file.open(QIODevice::ReadOnly) || !doc.setContent(&file))
	{
		QMessageBox::critical(getMainWindow(), "FatRat", tr("Failed to load BitTorrent search engine information."));
		return;
	}
	
	QDomElement n = doc.documentElement().firstChildElement("engine");
	while(!n.isNull())
	{
		Engine e;
		
		e.http = 0;
		e.buffer = 0;
		
		e.name = n.attribute("name");
		e.id = n.attribute("id");
		
		QDomElement sn = n.firstChildElement("param");
		while(!sn.isNull())
		{
			QString contents = sn.firstChild().toText().data();
			QString type = sn.attribute("name");
			
			if(type == "query")
				e.query = contents;
			else if(type == "postdata")
				e.postData = contents;
			else if(type == "beginning")
				e.beginning = contents;
			else if(type == "splitter")
				e.splitter = contents;
			else if(type == "ending")
				e.ending = contents;
			
			sn = sn.nextSiblingElement("param");
		}
		
		sn = n.firstChildElement("regexp");
		while(!sn.isNull())
		{
			RegExpParam param;
			QString type = sn.attribute("name");
			
			param.regexp = sn.firstChild().toText().data();
			param.field = sn.attribute("field", "0").toInt();
			param.match = sn.attribute("match", "0").toInt();
			
			e.regexps << QPair<QString, RegExpParam>(type, param);
			
			sn = sn.nextSiblingElement("regexp");
		}
		
		m_engines << e;
		
		n = n.nextSiblingElement("engine");
	}
}

void TorrentSearch::search()
{
	if(m_bSearching)
	{
		for(int i=0;i<m_engines.size();i++)
		{
			if(m_engines[i].http)
			{
				m_engines[i].http->abort();
				m_engines[i].http->deleteLater();
				m_engines[i].http = 0;
				
				m_engines[i].buffer->deleteLater();
				m_engines[i].buffer = 0;
			}
		}
		
		m_bSearching = false;
		updateUi();
	}
	else
	{
		bool bSel = false;
		QString expr = lineExpr->text();
		
		if(expr.isEmpty())
			return;
		
		emit changeTabTitle("BitTorrent search: " + expr);
		
		for(int i=0;i<listEngines->count();i++)
		{
			if(listEngines->item(i)->checkState() == Qt::Checked)
			{
				QUrl url = m_engines[i].query.arg(expr);
				QString path = url.path();
				
				path.replace(' ', '+');
				
				m_engines[i].http = new QHttp(url.host(), url.port(80), this);
				m_engines[i].buffer = new QBuffer(this);
				
				m_engines[i].buffer->open(QIODevice::ReadWrite);
				
				connect(m_engines[i].http, SIGNAL(done(bool)), this, SLOT(searchDone(bool)));
				
				if(!m_engines[i].postData.isEmpty())
				{
					QString postQuery;
					QByteArray postEnc;
					QHttpRequestHeader hdr ("POST", path+"?"+url.encodedQuery());
					
					postQuery = m_engines[i].postData.arg(expr);
					postQuery.replace(' ', '+');
					
					postEnc = postQuery.toUtf8();
					
					hdr.addValue("host", url.host());
					hdr.addValue("content-length", QString::number(postEnc.size()));
					hdr.addValue("content-type", "application/x-www-form-urlencoded");
					
					m_engines[i].http->request(hdr, postEnc, m_engines[i].buffer);
				}
				else
					m_engines[i].http->get(path+"?"+url.encodedQuery(), m_engines[i].buffer);
				
				bSel = true;
			}
		}
		
		if(bSel)
		{
			treeResults->clear();
			
			m_bSearching = true;
			updateUi();
		}
		else
		{
			QMessageBox::warning(getMainWindow(), "FatRat", tr("Please enable at least one search engine."));
		}
	}
}

void TorrentSearch::parseResults(Engine* e)
{
	try
	{
		const QByteArray& data = e->buffer->data();
		QList<QByteArray> results;
		int end, start;
		
		start = data.indexOf(e->beginning);
		if(start < 0)
			throw RuntimeException("Error parsing search engine's response - 'start'");
		
		end = data.indexOf(e->ending, start);
		if(end < 0)
			throw RuntimeException("Error parsing search engine's response - 'end'");
		
		start += e->beginning.size();
		results = splitArray(data.mid(start, end-start), e->splitter);
		
		delete e->buffer;
		e->buffer = 0;
		
		qDebug() << "Results:" << results.size();
		
		foreach(QByteArray ar, results)
		{
			try
			{
				QMap<QString, QString> map;
				
				for(int i=0;i<e->regexps.size();i++)
				{
					QRegExp re(e->regexps[i].second.regexp);
					int pos = 0;
					
					for(int k=0;k<e->regexps[i].second.match+1;k++)
					{
						pos = re.indexIn(ar, pos);
						
						if(pos < 0)
							throw RuntimeException(QString("Failed to match \"%1\" in \"%2\"").arg(e->regexps[i].first).arg(QString(ar)));
						else
							pos++; // FIXME
					}
					
					QTextDocument doc; // FIXME: ineffective?
					doc.setHtml(re.cap(e->regexps[i].second.field+1));
					map[e->regexps[i].first] = doc.toPlainText();
				}
				
				SearchTreeWidgetItem* item = new SearchTreeWidgetItem(treeResults);
				item->setText(0, map["name"]);
				item->setText(1, map["size"]);
				item->parseSize(map["size"]);
				item->setText(2, map["seeders"]);
				item->setText(3, map["leechers"]);
				item->setText(4, e->name);
				item->m_strLink = map["link"];
				
				if(!item->m_strLink.startsWith("http://"))
				{
					QUrl url(e->query);
					
					if(item->m_strLink[0] != '/')
					{
						QString path = url.path();
						int ix = path.lastIndexOf('/');
						if(ix != -1)
						{
							path.resize(ix+1);
							item->m_strLink.prepend(path);
						}
					}
					
					item->m_strLink = QString("%1://%2:%3%4")
							.arg(url.scheme()).arg(url.host()).arg(url.port(80)).arg(map["link"]);
				}
				
				treeResults->addTopLevelItem(item);
			}
			catch(const RuntimeException& e)
			{
				qDebug() << e.what();
			}
		}
	}
	catch(const RuntimeException& e)
	{
		qDebug() << e.what();
	}
}

void TorrentSearch::searchDone(bool error)
{
	QHttp* http = (QHttp*) sender(); // this sucks
	Engine* e = 0;
	
	for(int i=0;i<m_engines.size();i++)
	{
		if(m_engines[i].http == http)
		{
			e = &m_engines[i];
			break;
		}
	}
	
	if(!e)
		return;
	
	http->deleteLater();
	e->http = 0;
	
	if(!error)
		parseResults(e);
	
	m_bSearching = false;
	for(int i=0;i<m_engines.size();i++)
	{
		if(m_engines[i].http != 0)
		{
			m_bSearching = true;
			break;
		}
	}
	
	updateUi();
}

void TorrentSearch::updateUi()
{
	pushSearch->setText(m_bSearching ? tr("Stop searching") : tr("Search"));
	lineExpr->setDisabled(m_bSearching);
	listEngines->setDisabled(m_bSearching);
}

QList<QByteArray> TorrentSearch::splitArray(const QByteArray& src, QString sep)
{
	int n, split = 0;
	QList<QByteArray> out;
	
	while((n = src.indexOf(sep, split)) != -1)
	{
		out << src.mid(split, n-split);
		split = n + sep.size();
	}
	
	out << src.mid(split);
	
	return out;
}

void TorrentSearch::itemDoubleClicked(QTreeWidgetItem* item)
{
	SearchTreeWidgetItem* it = (SearchTreeWidgetItem*) item;
	MainWindow* wnd = (MainWindow*) getMainWindow();
	
	wnd->addTransfer(it->m_strLink, Transfer::Download, "TorrentDownload");
}

/////////////////////////////////

bool SearchTreeWidgetItem::operator<(const QTreeWidgetItem& other) const
{
	int column = treeWidget()->sortColumn();
	if(column == 1) // size
	{
		const SearchTreeWidgetItem* item = (SearchTreeWidgetItem*) &other;
		return m_nSize < item->m_nSize;
	}
	else if(column == 0) // name
		return text(column).toLower() < other.text(column).toLower();
	else if(column == 2 || column == 3) // seeders & leechers
		return text(column).toInt() < other.text(column).toInt();
	else
		return QTreeWidgetItem::operator<(other);
}

void SearchTreeWidgetItem::parseSize(QString in)
{
	int split = -1;
	
	for(int i=0;i<in.size();i++)
	{
		if(!in[i].isDigit() && in[i] != '.')
		{
			split = i;
			break;
		}
	}
	
	if(split < 0)
	{
		//qDebug() << "Unable to parse size:" << in;
		split = in.size();
		in += "mb";
	}
	//else
	{
		QString units;
		double size = in.left(split).toDouble();
		
		units = in.mid(split).trimmed();
		
		if(!units.compare("k", Qt::CaseInsensitive)
			|| !units.compare("kb", Qt::CaseInsensitive)
			|| !units.compare("kib", Qt::CaseInsensitive))
		{
			size *= 1024;
		}
		else if(!units.compare("m", Qt::CaseInsensitive)
			|| !units.compare("mb", Qt::CaseInsensitive)
			|| !units.compare("mib", Qt::CaseInsensitive))
		{
			size *= 1024LL*1024LL;
		}
		else if(!units.compare("g", Qt::CaseInsensitive)
			|| !units.compare("gb", Qt::CaseInsensitive)
			|| !units.compare("gib", Qt::CaseInsensitive))
		{
			size *= 1024LL*1024LL*1024LL;
		}
		
		m_nSize = qint64(size);
		setText(1, formatSize(m_nSize));
	}
}

