# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: .
# Target is an application:  fatrat


FORMS += MainWindow.ui \
         QueueDlg.ui \
         HttpOptsWidget.ui \
         WidgetHostDlg.ui \
         SettingsDlg.ui \
         SettingsGeneralForm.ui \
         SettingsTorrentForm.ui \
         DetailsTorrentForm.ui  \
         SettingsHttpForm.ui \
         ProxyDlg.ui \
         UserAuthDlg.ui \
         TorrentOptsWidget.ui \
 HttpUrlOptsDlg.ui \
 NewTransferDlg.ui \
 GenericOptsForm.ui \
 FtpUploadOptsForm.ui
TRANSLATIONS += locale/fatrat_cs_CZ.ts 
RESOURCES += gfx/resources.qrc 
HEADERS += fatrat.h \
           MainWindow.h \
           QueueDlg.h \
           QueueMgr.h \
           Queue.h \
           Transfer.h \
           TransfersModel.h \
           FakeDownload.h \
           GeneralDownload.h \
           WidgetHostDlg.h \
           GeneralNetwork.h \
           InfoBar.h \
           TorrentDownload.h \
           SettingsDlg.h \
           WidgetHostChild.h \
           SettingsGeneralForm.h \
           torrent/bencodeparser.h \
           torrent/connectionmanager.h \
           torrent/filemanager.h \
           torrent/metainfo.h \
           torrent/peerwireclient.h \
           torrent/ratecontroller.h \
           torrent/sha1.h \
           torrent/torrentclient.h \
           torrent/torrentserver.h \
           torrent/trackerclient.h  \
           ProxyDlg.h \
           dbus_adaptor.h \
           SimpleEmail.h \
           UserAuthDlg.h \
           SpeedGraph.h \
 TransferLog.h \
 DropBox.h \
 RapidshareUpload.h \
 HttpFtpSettings.h \
 FtpUpload.h \
 NewTransferDlg.h \
 GenericOptsForm.h
SOURCES += fatrat.cpp \
           MainWindow.cpp \
           QueueMgr.cpp \
           Queue.cpp \
           Transfer.cpp \
           TransfersModel.cpp \
           GeneralDownload.cpp \
           GeneralNetwork.cpp \
           InfoBar.cpp \
           TorrentDownload.cpp \
           torrent/bencodeparser.cpp \
           torrent/connectionmanager.cpp \
           torrent/filemanager.cpp \
           torrent/metainfo.cpp \
           torrent/peerwireclient.cpp \
           torrent/ratecontroller.cpp \
           torrent/sha1.c \
           torrent/torrentclient.cpp \
           torrent/torrentserver.cpp \
           torrent/trackerclient.cpp  \
           dbus_adaptor.cpp \
           SimpleEmail.cpp \
           SpeedGraph.cpp \
 DropBox.cpp \
 RapidshareUpload.cpp \
 HttpFtpSettings.cpp \
 FtpUpload.cpp
TEMPLATE = app
TARGET = fatrat
CONFIG += qdbus debug
QT += xml network

locale.files = locale/*.qm 
locale.path = /usr/share/locale/fatrat
INSTALLS += locale

target.path = /usr/bin

INSTALLS += target