#include "keyboard.h"
#include "ui_keyboard.h"
#include <QMessageBox>
#include <QDebug>
#include <unordered_map>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>

std::unordered_map<QString, int> keyMap = {
	{" ", KEY_SPACE},
	{"a", KEY_A},
	{"b", KEY_B},
	{"c", KEY_C},
	{"d", KEY_D},
	{"e", KEY_E},
	{"f", KEY_F},
	{"g", KEY_G},
	{"h", KEY_H},
	{"i", KEY_I},
	{"j", KEY_J},
	{"k", KEY_K},
	{"l", KEY_L},
	{"m", KEY_M},
	{"n", KEY_N},
	{"o", KEY_O},
	{"p", KEY_P},
	{"q", KEY_Q},
	{"r", KEY_R},
	{"s", KEY_S},
	{"t", KEY_T},
	{"u", KEY_U},
	{"v", KEY_V},
	{"w", KEY_W},
	{"x", KEY_X},
	{"y", KEY_Y},
	{"z", KEY_Z},
	{"1", KEY_1},
	{"2", KEY_2},
	{"3", KEY_3},
	{"4", KEY_4},
	{"5", KEY_5},
	{"6", KEY_6},
	{"7", KEY_7},
	{"8", KEY_8},
	{"9", KEY_9},
	{"0", KEY_0},
};

Keyboard::Keyboard(QWidget *parent) : QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus), ui(new Ui::Keyboard) {
	ui->setupUi(this);

	this->setFixedSize(this->width(), this->height());
	shift = false;
	caps = false;
	keyList = ui->keyholder->children();
	initKeys();

	uinputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(uinputFd == -1){
		qDebug()<<"Error: failed to open uinput (got error: "<<strerror(errno)<<")";
	}
	ioctl(uinputFd, UI_SET_EVBIT, EV_KEY);
	ioctl(uinputFd, UI_SET_EVBIT, EV_SYN);
	//ydotool sets all of these, so let's copy that.  Any other way I couldn't get ID_INPUT_KEYBOARD to get set
	for(unsigned int i=1;i<=743;i++){
		ioctl(uinputFd, UI_SET_KEYBIT, i);
	}

	struct uinput_setup usetup{};
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x1; //sample vendor
	usetup.id.product = 0x1; //sample product
	strcpy(usetup.name, "Virtual USB Keyboard");

	if(ioctl(uinputFd, UI_DEV_SETUP, &usetup) == -1){
		qDebug()<<"Error: failed to setup uinput device (got error: "<<strerror(errno)<<")";
	}
	if(ioctl(uinputFd, UI_DEV_CREATE) == -1){
		qDebug()<<"Error: failed to create uinput device (got error: "<<strerror(errno)<<")";
	}
}

Keyboard::~Keyboard() {
	delete ui;
	if(uinputFd != -1){
		ioctl(uinputFd, UI_DEV_DESTROY);
		::close(uinputFd);
	}
}

void Keyboard::initKeys() {
	QPushButton *button;

	for (int i = 0; i < keyList.count(); i++) {
		button = qobject_cast<QPushButton *>(keyList.at(i));
		if (button != 0) {
			if ((button->property("btnMode").toInt() == 0) ||
					(button->property("btnMode").toInt() == 1)) {
				connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
			} else if (button->property("btnMode").toInt() == 2) {
				if (button->property("btnNormal") == "shift") {
					connect(button, SIGNAL(clicked()), this, SLOT(shiftClicked()));
				} else if (button->property("btnNormal") == "caps") {
					connect(button, SIGNAL(clicked()), this, SLOT(capsClicked()));
				} else if (button->property("btnNormal") == "enter") {
					connect(button, SIGNAL(clicked()), this, SLOT(enterClicked()));
				} else if (button->property("btnNormal") == "ok") {
					connect(button, SIGNAL(clicked()), this, SLOT(okClicked()));
				} else if (button->property("btnNormal") == "escape") {
					connect(button, SIGNAL(clicked()), this, SLOT(escapeClicked()));
				} else if (button->property("btnNormal") == "space") {
					connect(button, SIGNAL(clicked()), this, SLOT(spaceClicked()));
				} else if (button->property("btnNormal") == "clear") {
					connect(button, SIGNAL(clicked()), this, SLOT(clearClicked()));
				} else if (button->property("btnNormal") == "tab") {
					connect(button, SIGNAL(clicked()), this, SLOT(tabClicked()));
				} else if (button->property("btnNormal") == "backspace") {
					connect(button, SIGNAL(clicked()), this, SLOT(backspaceClicked()));
				} else if (button->property("btnNormal") == "left") {
					connect(button, SIGNAL(clicked()), this, SLOT(leftClicked()));
				} else if (button->property("btnNormal") == "right") {
					connect(button, SIGNAL(clicked()), this, SLOT(rightClicked()));
				}
			}
		}
	}
}

void uinput_emit(int fd, int type, int code, int val){
	struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;
	/* timestamp values below are ignored */
	ie.time.tv_sec = 0;
	ie.time.tv_usec = 0;

	if(write(fd, &ie, sizeof(ie)) < 0){
		qDebug() << "Error: unable to write input event" <<strerror(errno);
	}
}

void Keyboard::buttonClicked() {
	QPushButton *tmp;
	tmp = qobject_cast<QPushButton *>(sender());

	const auto& txt = tmp->text();
	if(keyMap.contains(txt)){
		int keycode = keyMap.at(txt);
		uinput_emit(uinputFd, EV_KEY, keycode, 1);
		uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
		uinput_emit(uinputFd, EV_KEY, keycode, 0);
		uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	}else if(txt.size() > 0 && isupper(txt[0].unicode())){
		int keycode = keyMap.at(QString(QChar(tolower(txt[0].unicode()))));
		uinput_emit(uinputFd, EV_KEY, KEY_LEFTSHIFT, 1);
		uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
		uinput_emit(uinputFd, EV_KEY, keycode, 1);
		uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
		uinput_emit(uinputFd, EV_KEY, keycode, 0);
		uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
		uinput_emit(uinputFd, EV_KEY, KEY_LEFTSHIFT, 0);
		uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	}else{
		qDebug()<<"Unknown key"<<txt<<"was pressed";
	}

	if (shift == true) {
		shift = false;
		morphKeys();
	}
}

void Keyboard::shiftClicked() {
	shift ^= true;
	morphKeys();
	uinput_emit(uinputFd, EV_KEY, KEY_LEFTSHIFT, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_LEFTSHIFT, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::capsClicked() {
	shift = false;
	caps ^= true;
	morphKeys();
	uinput_emit(uinputFd, EV_KEY, KEY_CAPSLOCK, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_CAPSLOCK, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::enterClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_ENTER, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_ENTER, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::morphKeys() {
	QPushButton *button;
	for (int i = 0; i < keyList.count(); i++) {
		button = qobject_cast<QPushButton *>(keyList.at(i));
		if (button != 0) {
			if (button->property("btnMode").toInt() == 0) {
				if (caps || shift) {
					button->setText(button->property("btnShift").toString());
				} else {
					button->setText(button->property("btnNormal").toString());
				}
			}
			if (button->property("btnMode").toInt() == 1) {
				if (shift) {
					button->setText(button->property("btnShift").toString());
				} else {
					button->setText(button->property("btnNormal").toString());
				}
			}
		}
	}
}

void Keyboard::escapeClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_ESC, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_ESC, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::spaceClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_SPACE, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_SPACE, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::deleteClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_DELETE, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_DELETE, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::tabClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_TAB, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_TAB, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::backspaceClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_BACKSPACE, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_BACKSPACE, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::activate(QVariant initValue, int maxChars, bool multiLine) {
	this->multiLine = multiLine;
	show();
}

void Keyboard::leftClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_LEFT, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_LEFT, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

void Keyboard::rightClicked() {
	uinput_emit(uinputFd, EV_KEY, KEY_RIGHT, 1);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
	uinput_emit(uinputFd, EV_KEY, KEY_RIGHT, 0);
	uinput_emit(uinputFd, EV_SYN, SYN_REPORT, 0);
}

