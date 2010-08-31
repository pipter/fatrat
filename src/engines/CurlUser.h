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

In addition, as a special exemption, Luboš Doležel gives permission
to link the code of FatRat with the OpenSSL project's
"OpenSSL" library (or with modified versions of it that use the; same
license as the "OpenSSL" library), and distribute the linked
executables. You must obey the GNU General Public License in all
respects for all of the code used other than "OpenSSL".
*/


#ifndef CURLUSER_H
#define CURLUSER_H
#include "CurlStat.h"
#include <curl/curl.h>
#include <sys/time.h>
#include <QReadWriteLock>
#include <QList>
#include <QPair>

class CurlUser : public CurlStat
{
public:
	CurlUser();
	virtual ~CurlUser();

	virtual bool idleCycle(const timeval& tvNow);

protected:
	virtual CURL* curlHandle() = 0;
	virtual void transferDone(CURLcode result) = 0;
	
	virtual size_t readData(char* buffer, size_t maxData);
	virtual bool writeData(const char* buffer, size_t bytes);
	
	static size_t read_function(char *ptr, size_t size, size_t nmemb, CurlUser* This);
	static size_t write_function(const char* ptr, size_t size, size_t nmemb, CurlUser* This);

	void setSegmentMaster(CurlStat* master);
	CurlStat* segmentMaster() const;

private:
	CurlStat* m_master;
	
	friend class CurlPoller;
	friend class CurlPollingMaster;
};

#endif
