#include "config.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QApplication>
#include "mainWindow.hpp"

void configUI::initUI(){
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	
	QPushButton* quitButton = new QPushButton("Quit");
	layout->addWidget(quitButton);
	QObject::connect(quitButton, &QPushButton::pressed, [](){QApplication::quit();});

	QComboBox* themeSelector = new QComboBox(this);
	themeSelector->addItem("Native");
	themeSelector->addItem("Dark");
	themeSelector->setCurrentIndex((unsigned int)win->theme);
	layout->addWidget(themeSelector);
	QObject::connect(themeSelector, &QComboBox::currentIndexChanged, [this](int index){
			win->theme = (mainWindow::Theme)index;
			win->saveSettings();
	});
}

