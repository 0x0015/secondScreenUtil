#pragma once
#include <QMainWindow>
#include <QTabWidget>

class mainWindow : public QMainWindow{
public:
	QTabWidget* tabs;
	QString settingsPath;
	void initUI();
	void initialLoad();
	void loadSettings();
	void saveSettings();
	void setNoActivateX11();
	void setNoActivateWayland();
private:
	unsigned int screenNumber = 1;
};
