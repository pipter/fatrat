/*
FatRat download manager
http://fatrat.dolezel.info

Copyright (C) 2006-2008 Lubos Dolezel <lubos a dolezel.info>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef RAPIDSHAREFREEDOWNLOAD_H
#define RAPIDSHAREFREEDOWNLOAD_H
#include "CurlDownload.h"
#include <QUrl>
#include <QTimer>

class QHttp;
class QBuffer;

class RapidshareFreeDownload : public CurlDownload
{
Q_OBJECT
public:
	RapidshareFreeDownload();
	virtual ~RapidshareFreeDownload();
	
	virtual void init(QString source, QString target);
	virtual QString myClass() const { return "RapidshareFreeDownload"; }
	virtual QString name() const;
	
	virtual void load(const QDomNode& map);
	virtual void save(QDomDocument& doc, QDomNode& map) const;
	virtual void changeActive(bool bActive);
	
	virtual WidgetHostChild* createOptionsWidget(QWidget* w) { return 0; }
	virtual void fillContextMenu(QMenu& menu) {}
	
	virtual QString object() const { return m_strTarget; }
	virtual void setObject(QString newdir);
	
	virtual qulonglong done() const;
	
	static Transfer* createInstance() { return new RapidshareFreeDownload; }
	static int acceptable(QString uri, bool);
protected slots:
	void firstPageDone(bool error);
	void secondPageDone(bool error);
	void secondElapsed();
protected:
	void deriveName();
	virtual void transferDone(CURLcode result);
private:
	QString m_strOriginal, m_strName, m_strTarget;
	QUrl m_downloadUrl;
	QHttp* m_http;
	QBuffer* m_buffer;
	int m_nSecondsLeft;
	QTimer m_timer;
	bool m_bHasLock;
};

#endif