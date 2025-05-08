#include "systemInfo.hpp"
#include <QLabel>
#include "util.hpp"
#include <iostream>
#include <filesystem>
#include <map>

static inline const char *humanSize(std::size_t bytes){
	const char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
	char length = sizeof(suffix) / sizeof(suffix[0]);

	int i = 0;
	double dblBytes = bytes;

	if (bytes > 1024) {
		for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
			dblBytes = bytes / 1024.0;
	}

	static char output[200];
	sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
	return output;
}

void systemInfo::initUI(unsigned int updateTime){
	layout = new QGridLayout(this);
	setLayout(layout);

	cpuUsageLabel = new QLabel("Cpu Usage: ");
	layout->addWidget(cpuUsageLabel);
	processCountLabel = new QLabel("Process Count: ");
	layout->addWidget(processCountLabel);
	ramLabel = new QLabel("Ram: ");
	layout->addWidget(ramLabel);
	swapLabel = new QLabel("Swap: ");
	layout->addWidget(swapLabel);

	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, [this](){updateUI();});
	timer->start(updateTime);
}

void systemInfo::updateUI(){
	cpuUsageLabel->setText("CPU Usage: " + QString::number(getCpuUsage()));
	processCountLabel->setText("Process Count: " + QString::number(getProcessCount()));
	updateMemoryStats();
	ramLabel->setText("Ram: " + QString(humanSize(memoryStats.usedRam)) + " used / " + QString(humanSize(memoryStats.totalRam)) + " total");
	swapLabel->setText("Swap: " + QString(humanSize(memoryStats.usedSwap)) + " used / " + humanSize(memoryStats.totalSwap) + " total");
}

float systemInfo::getCpuUsage(){
	std::string procStat = readFileIntoString("/proc/stat");
	auto lines = splitString(procStat, "\n");
	if(lines.empty())
		return 0;
	auto vals = splitString(lines[0], " ");
	if(vals[0] != "cpu")
		return 0;

	std::vector<std::size_t> nums;
	for(unsigned int i=1;i<vals.size();i++)
		nums.push_back(std::stoull((std::string)vals[i]));

	std::size_t totalJiffies = 0;
	std::size_t workJiffies = 0;

	for(const auto& num : nums)
		totalJiffies += num;
	for(unsigned int i=0;i<3;i++)
		workJiffies += nums[i];

	std::size_t workOverPeriod = workJiffies - oldWorkJiffies;
	std::size_t totalOverPeriod = totalJiffies - oldTotalJiffies;

	float cpuPercent = (float)workOverPeriod / totalOverPeriod * 100.0f;
	oldWorkJiffies = workJiffies;
	oldTotalJiffies = totalJiffies;
	return cpuPercent;
}

std::size_t systemInfo::getProcessCount(){
	std::size_t output = 0;
	for(const auto& dir_entry : std::filesystem::directory_iterator{"/proc"}){
		if(!dir_entry.is_directory())
			continue;
		const auto& procName = dir_entry.path().filename().string();
		if(!std::all_of(procName.begin(), procName.end(), [](char c){return std::isdigit(c);}))
			continue;
		output++;
	}

	return output;
}

void systemInfo::updateMemoryStats(){
	std::string memInfo = readFileIntoString("/proc/meminfo");
	auto lines = splitString(memInfo, "\n");
	if(lines.empty())
		return;

	std::map<std::string_view, std::size_t> memInfoEntries;
	for(const auto& line : lines){
		const auto& parts = splitString(line, " ");
		if(parts.size() < 2)
			continue;
		std::size_t partSize = std::stoull((std::string)parts[1]);
		if(parts.size() >= 3 && parts[2] == "kB")
			partSize *= 1024;
		memInfoEntries[parts[0]] = partSize;
	}

	memoryStats.totalRam = memInfoEntries["MemTotal:"];
	memoryStats.usedRam = memoryStats.totalRam - memInfoEntries["MemAvailable:"] - memInfoEntries["Buffers:"];
	memoryStats.totalSwap = memInfoEntries["SwapTotal:"];
	memoryStats.usedSwap = memoryStats.totalSwap - memInfoEntries["SwapFree:"];
}

