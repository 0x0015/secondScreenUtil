#pragma once
#include <QWidget>
#include <QVBoxLayout>

class keyboardWidget : public QWidget{
public:
	QVBoxLayout* layout;
	void initUI();
};
