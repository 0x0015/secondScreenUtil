#pragma once
#include <QWidget>

class mainWindow;
class configUI : public QWidget{
public:
	mainWindow* win;
	void initUI();
};
