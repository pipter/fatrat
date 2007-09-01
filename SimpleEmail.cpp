#include "SimpleEmail.h"
#include <QtDebug>

SimpleEmail::SimpleEmail(QString server, QString from, QString to, QString message)
	: m_strFrom(from), m_strTo(to), m_strMessage(message)
{
	m_pSocket = new QTcpSocket(this);
	connect(m_pSocket, SIGNAL(connected()), this, SLOT(connected()));
	connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	
	m_pSocket->connectToHost(server, 25);
}

void SimpleEmail::connected()
{
	m_state = Init;
	m_pStream = new QTextStream(m_pSocket);
	qDebug() << "connected\n";
}

void SimpleEmail::readyRead()
{
	QString line;
	do
	{
		line = m_pSocket->readLine();
	}
	while(m_pSocket->canReadLine() && line[3] != ' ');
	
	if ( m_state == Init && line[0] == '2')
	{
		*m_pStream << "HELO fatrat\r\n";
		m_state = Mail;
	}
	else if (m_state == Mail && line[0] == '2')
	{
		*m_pStream << "MAIL FROM: <" << m_strFrom << ">\r\n";
		m_state = Rcpt;
	}
	else if (m_state == Rcpt && line[0] == '2')
	{
		*m_pStream << "RCPT TO: <" << m_strTo << ">\r\n";
		m_state = Data;
	}
	else if (m_state == Data && line[0] == '2')
	{
		*m_pStream << "DATA\r\n";
		m_state = Body;
	}
	else if (m_state == Body && line[0] == '3')
	{
		*m_pStream << m_strMessage << "\r\n.\r\n";
		m_state = Quit;
	}
	else if (m_state == Quit && line[0] == '2')
	{
		*m_pStream << "QUIT\r\n";
		m_state = Close;
	}
	else if (m_state == Close)
	{
		deleteLater();
		return;
	}
	else
	{
		qDebug() << "Failed to send e-mail - bad response";
		deleteLater();
	}
	m_pStream->flush();
}

void SimpleEmail::error(QAbstractSocket::SocketError)
{
	qDebug() << "Failed to send e-mail";
	deleteLater();
}