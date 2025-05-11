#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QtCharts/QtCharts>

class dataOverTimeChart : public QWidget{
public:
	QChart* chart;
	QLineSeries* series;
	unsigned int numDatapoints;
	QValueAxis* axisX;
	QValueAxis* axisY;
	float currentMaxY;
	dataOverTimeChart(QWidget* parent, unsigned int numData, float yMax);
	void addDatapoint(float point);
};

class systemInfo : public QWidget{
public:
	QTimer* timer;
	QGridLayout* layout;

	using QLabel_ptr = QLabel*;//easier to stack defs
	QLabel_ptr cpuLabel, cpuUsageLabel, processCountLabel, ramLabel, swapLabel, gpuLabel, vramLabel, gpuTempLabel, gpuPowerDrawLabel, gpuUsageLabel;
	using DOTChart_ptr = dataOverTimeChart*;
	DOTChart_ptr cpuUsageChart, ramUsageChart, swapUsageChart, vramUsageChart, gpuTempChart, gpuPowerChart, gpuUsageChart;
	void initUI(unsigned int updateTime = 500);
	void updateUI();
	std::size_t oldWorkJiffies = 0;
	std::size_t oldTotalJiffies = 0;
	float getCpuUsage();//usage since last check
	std::string getCpuName();
	std::size_t getProcessCount();
	struct{
		std::size_t totalRam, usedRam, totalSwap, usedSwap;
	} memoryStats;
	void updateMemoryStats();
	struct{
		std::string name, gfxName;
		std::size_t usedVram, totalVram;
		float usage;
		float temp;
		float powerDraw;
	} gpuStats;
	void updateGpuStats();
};
