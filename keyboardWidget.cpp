#include "keyboardWidget.hpp"
#include "keyboard.h"

void keyboardWidget::initUI(){
	layout = new QVBoxLayout(this);
	setLayout(layout);

	Keyboard* keyboard = new Keyboard(this);
	layout->addWidget(keyboard);
}

