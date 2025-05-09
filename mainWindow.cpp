#include "mainWindow.hpp"
#include "systemInfo.hpp"
#include "keyboardWidget.hpp"
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

	setNoActivateX11();
}

void mainWindow::loadSettings(){
	QSettings settings(settingsPath, QSettings::IniFormat);
	if(settings.contains("screenNumber")){
		screenNumber = settings.value("screenNumber").toUInt();
	}else{
		saveSettings();//if no settings exist, make one
	}
}

void mainWindow::saveSettings(){
	QSettings settings(settingsPath, QSettings::IniFormat);
	settings.setValue("screenNumber", screenNumber);
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

