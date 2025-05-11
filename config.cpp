#include "config.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include "mainWindow.hpp"

void configUI::initUI(){
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	
	QPushButton* quitButton = new QPushButton("Quit");
	layout->addWidget(quitButton);
	QObject::connect(quitButton, &QPushButton::pressed, [](){QApplication::quit();});
}

