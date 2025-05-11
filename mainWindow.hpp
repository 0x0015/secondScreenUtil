#pragma once
#include <QMainWindow>
#include <QTabWidget>

class mainWindow : public QMainWindow{
public:
	enum Theme{
		Native,
		Dark
	} theme;
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
