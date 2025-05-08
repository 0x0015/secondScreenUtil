#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

class systemInfo : public QWidget{
public:
	QTimer* timer;
	QGridLayout* layout;

	QLabel* cpuUsageLabel;
	QLabel* processCountLabel;
	QLabel* ramLabel;
	QLabel* swapLabel;
	void initUI(unsigned int updateTime = 500);
	void updateUI();
	std::size_t oldWorkJiffies = 0;
	std::size_t oldTotalJiffies = 0;
	float getCpuUsage();//usage since last check
	std::size_t getProcessCount();
	struct{
		std::size_t totalRam, usedRam, totalSwap, usedSwap;
	} memoryStats;
	void updateMemoryStats();
};
