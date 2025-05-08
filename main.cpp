#include <QApplication>
#include "mainWindow.hpp"

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QModernWindowsStylePlugin)
#endif

int main(int argc, char** argv){
	QApplication::setApplicationName("secondScreenUtil");
	QApplication app(argc, argv);

	mainWindow* win = new mainWindow();
	win->createWinId();
	win->setNoActivateWayland();
	win->show();
	win->initUI();

	auto ret = app.exec();
	return ret;
}
