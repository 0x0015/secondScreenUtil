//Adapted from https://github.com/mazj/VirtualInput

#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <QtUiPlugin/qdesignerexportwidget.h>
#include <QDialog>
#include <QLineEdit>

namespace Ui {
	class Keyboard;
}

class Keyboard : public QWidget{
	Q_OBJECT
public:
	explicit Keyboard(QWidget *parent = 0);
	~Keyboard();

	void activate(QVariant initValue,int maxChars,bool multiLine=false);
	void setEchoMode(QLineEdit::EchoMode);
	void setValidator(const QValidator *);
private slots:
	void buttonClicked();
	void shiftClicked();
	void capsClicked();
	void enterClicked();
	void escapeClicked();
	void spaceClicked();
	void deleteClicked();
	void tabClicked();
	void backspaceClicked();
	void leftClicked();
	void rightClicked();
private:
	int uinputFd = -1;
	Ui::Keyboard *ui;
	QObjectList keyList;
	bool shift,caps, multiLine;
	void initKeys();
	void morphKeys();

signals:
	void dataSet(QVariant *);
};

#endif // KEYBOARD_H
