#include "mainWindow.hpp"
#include "systemInfo.hpp"
#include "keyboardWidget.hpp"
#include "config.hpp"
#include <QApplication>
#include <QSettings>
#include <QWindow>
#include <QGuiApplication>
#include <QDebug>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LayerShellQt/Window>
#include <LayerShellQt/Shell>

void mainWindow::initialLoad(){
	settingsPath = QApplication::applicationDirPath() + "/secondScreenUtilSettings.ini";
	loadSettings();

	if(screenNumber != 0){
		windowHandle()->setScreen(QApplication::screens()[screenNumber-1]);
		showFullScreen();
	}

	switch(theme){
		case Native:
			break;
		case Dark:{
			qDebug()<<"Setting dark palette";
			QPalette darkPalette;
			darkPalette.setColor(QPalette::Window,QColor(53,53,53));
			darkPalette.setColor(QPalette::WindowText,Qt::white);
			darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
			darkPalette.setColor(QPalette::Base,QColor(42,42,42));
			darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
			darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
			darkPalette.setColor(QPalette::ToolTipText,Qt::white);
			darkPalette.setColor(QPalette::Text,Qt::white);
			darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
			darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
			darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
			darkPalette.setColor(QPalette::Button,QColor(53,53,53));
			darkPalette.setColor(QPalette::ButtonText,Qt::white);
			darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
			darkPalette.setColor(QPalette::BrightText,Qt::red);
			darkPalette.setColor(QPalette::Link,QColor(42,130,218));
			darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
			darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
			darkPalette.setColor(QPalette::HighlightedText,Qt::white);
			darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
			
			qApp->setPalette(darkPalette);
			}
			break;
	}
}

void mainWindow::initUI(){
	setWindowTitle("Second Screen Util");
	tabs = new QTabWidget(this);
	setCentralWidget(tabs);

	systemInfo* systemInfoWidget = new systemInfo();
	systemInfoWidget->initUI();
	tabs->addTab(systemInfoWidget, "System Info");
	keyboardWidget* keyboard = new keyboardWidget();
	keyboard->initUI();
	tabs->addTab(keyboard, "Keyboard");
	configUI* config = new configUI();
	config->win = this;
	config->initUI();
	tabs->addTab(config, "Config");

	setNoActivateX11();
}

void mainWindow::loadSettings(){
	QSettings settings(settingsPath, QSettings::IniFormat);
	bool doSave = false;
	if(settings.contains("screenNumber")){
		screenNumber = settings.value("screenNumber").toUInt();
	}else{
		doSave = true;//if no settings exist, make one
	}
	if(settings.contains("theme")){
		theme = (Theme)settings.value("theme").toUInt();
	}else{
		doSave = true;
	}
	if(doSave)
		saveSettings();
}

void mainWindow::saveSettings(){
	QSettings settings(settingsPath, QSettings::IniFormat);
	settings.setValue("screenNumber", screenNumber);
	settings.setValue("theme", (unsigned int)theme);
}

void mainWindow::setNoActivateX11(){
	if(!qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
		return;

	Window id = winId();
	auto* x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
	Display* display = x11Application->display();
	if(!display){
		qWarning("Failed to get X11 display");
		return;
	}

	XWindowAttributes attr;
	if(XGetWindowAttributes(display, id, &attr) != 0){
		XSetWindowAttributes newAttrs{};
		newAttrs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask;
		newAttrs.override_redirect = true;
		
		XChangeWindowAttributes(display, id, CWOverrideRedirect | CWEventMask, &newAttrs);
		XRaiseWindow(display, id);
		XSelectInput(display, id, ExposureMask | ButtonPressMask | KeyPressMask);
	}else{
		qWarning("Failed to get X11 window attributes");
		return;
	}
}

void mainWindow::setNoActivateWayland(){
	if(!qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
		return;

	LayerShellQt::Shell::useLayerShell();

	LayerShellQt::Window* layerWindow = LayerShellQt::Window::get(windowHandle());
	layerWindow->setObjectName("secondScreenUtil");
	layerWindow->setLayer(LayerShellQt::Window::LayerTop);
	layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
	layerWindow->setExclusiveZone(0);
}

