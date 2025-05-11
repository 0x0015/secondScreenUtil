#include "systemInfo.hpp"
#include <QLabel>
#include "util.hpp"
#include <iostream>
#include <filesystem>
#include <map>
#include "json.hpp"
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

bool isDarkMode(){
	const QPalette defaultPalette;
	const auto text = defaultPalette.color(QPalette::WindowText);
	const auto window = defaultPalette.color(QPalette::Window);
	return text.lightness() > window.lightness();
}

dataOverTimeChart::dataOverTimeChart(QWidget* parent, unsigned int numData, float yMax) : chart(new QChart), series(new QLineSeries), numDatapoints(numData){
	auto chartView = new QChartView(chart);
	chart->addSeries(series);

	axisX = new QValueAxis;
	axisX->setRange(0, numDatapoints);
	axisX->setLabelFormat(" ");
	axisX->setTitleText("");

	axisY = new QValueAxis;
	currentMaxY = yMax;
	axisY->setRange(0, yMax);
	axisY->setLabelFormat(" ");

	chart->addAxis(axisX, Qt::AlignBottom);
	series->attachAxis(axisX);
	chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisY);
	chart->legend()->hide();

	auto layout = new QVBoxLayout(this);
	layout->addWidget(chartView);

	for(unsigned int i=0;i<numDatapoints;i++)
		series->append(i, 0);

	if(isDarkMode()){
		chart->setTheme(QChart::ChartThemeDark);
	}
}

void dataOverTimeChart::addDatapoint(float point){
	if(point > currentMaxY){
		currentMaxY = point;
		axisY->setRange(0, currentMaxY);
	}
	auto points = series->points();
	for(unsigned int i=0;i<numDatapoints-1;i++)
		points[i] = QPointF(i, points[i+1].y());
	points[numDatapoints-1] = QPointF(numDatapoints-1, point);
	series->replace(points);
}

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

	QWidget* cpuWidget = new QWidget(this);
	auto* cpuLayout = new QGridLayout(cpuWidget);
	cpuWidget->setLayout(cpuLayout);
	QWidget* gpuWidget = new QWidget(this);
	auto* gpuLayout = new QGridLayout(gpuWidget);
	gpuWidget->setLayout(gpuLayout);
	layout->addWidget(cpuWidget, 0, 0);
	layout->addWidget(gpuWidget, 0, 1);

	unsigned int graphUpdateTime = 90;

	cpuLabel = new QLabel("Cpu: ");
	cpuLayout->addWidget(cpuLabel);
	cpuUsageLabel = new QLabel("Cpu Usage: ");
	cpuLayout->addWidget(cpuUsageLabel);
	cpuUsageChart = new dataOverTimeChart(this, graphUpdateTime, 100);
	cpuLayout->addWidget(cpuUsageChart);
	ramLabel = new QLabel("Ram: ");
	cpuLayout->addWidget(ramLabel);
	updateMemoryStats();
	ramUsageChart = new dataOverTimeChart(this, graphUpdateTime, memoryStats.totalRam);
	cpuLayout->addWidget(ramUsageChart);
	swapLabel = new QLabel("Swap: ");
	swapUsageChart = new dataOverTimeChart(this, graphUpdateTime, memoryStats.totalSwap);
	cpuLayout->addWidget(swapUsageChart);
	cpuLayout->addWidget(swapLabel);
	processCountLabel = new QLabel("Process Count: ");
	cpuLayout->addWidget(processCountLabel);

	gpuLabel = new QLabel("Gpu: ");
	gpuLayout->addWidget(gpuLabel);
	gpuUsageLabel = new QLabel("Gpu Usage: ");
	gpuLayout->addWidget(gpuUsageLabel);
	gpuUsageChart = new dataOverTimeChart(this, graphUpdateTime, 100);
	gpuLayout->addWidget(gpuUsageChart);
	vramLabel = new QLabel("Vram: ");
	gpuLayout->addWidget(vramLabel);
	updateGpuStats();
	vramUsageChart = new dataOverTimeChart(this, graphUpdateTime, gpuStats.totalVram);
	gpuLayout->addWidget(vramUsageChart);
	gpuTempLabel = new QLabel("Gpu Temp: ");
	gpuLayout->addWidget(gpuTempLabel);
	gpuTempChart = new dataOverTimeChart(this, graphUpdateTime, 95);
	gpuLayout->addWidget(gpuTempChart);
	gpuPowerDrawLabel = new QLabel("Gpu Power: ");
	gpuLayout->addWidget(gpuPowerDrawLabel);
	gpuPowerChart = new dataOverTimeChart(this, graphUpdateTime, 8);
	gpuLayout->addWidget(gpuPowerChart);
	updateUI();

	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, [this](){updateUI();});
	timer->start(updateTime);
}

void systemInfo::updateUI(){
	cpuLabel->setText("Cpu: " + QString::fromStdString(getCpuName()));
	float cpuUsage = getCpuUsage();
	cpuUsageLabel->setText("CPU Usage: " + QString::number(cpuUsage) + "%");
	cpuUsageChart->addDatapoint(cpuUsage);
	processCountLabel->setText("Process Count: " + QString::number(getProcessCount()));
	updateMemoryStats();
	ramLabel->setText("Ram: " + QString(humanSize(memoryStats.usedRam)) + " used / " + QString(humanSize(memoryStats.totalRam)) + " total");
	ramUsageChart->addDatapoint(memoryStats.usedRam);
	swapLabel->setText("Swap: " + QString(humanSize(memoryStats.usedSwap)) + " used / " + QString(humanSize(memoryStats.totalSwap)) + " total");
	swapUsageChart->addDatapoint(memoryStats.usedSwap);
	updateGpuStats();
	gpuLabel->setText("Gpu: " + QString::fromStdString(gpuStats.name) + " (" + QString::fromStdString(gpuStats.gfxName) + ")");
	vramLabel->setText("Vram: " + QString(humanSize(gpuStats.usedVram)) + " used / " + QString(humanSize(gpuStats.totalVram)) + " total");
	vramUsageChart->addDatapoint(gpuStats.usedVram);
	gpuTempLabel->setText("Gpu Temp: " + QString::number(gpuStats.temp));
	gpuTempChart->addDatapoint(gpuStats.temp);
	gpuPowerDrawLabel->setText("Gpu Power: " + QString::number(gpuStats.powerDraw) + " W");
	gpuPowerChart->addDatapoint(gpuStats.powerDraw);
	gpuUsageLabel->setText("GPU Usage: " + QString::number(gpuStats.usage) + "%");
	gpuUsageChart->addDatapoint(gpuStats.usage);
}

float systemInfo::getCpuUsage(){
	std::string procStat = readFileIntoString("/proc/stat");
	auto lines = splitString(procStat, "\n");
	if(lines.empty())
		return 0;
	auto vals = splitStringOnWhitespace(lines[0]);
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
		const auto& parts = splitStringOnWhitespace(line);
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

std::string systemInfo::getCpuName(){
	std::string cpuInfo = readFileIntoString("/proc/cpuinfo");
	auto lines = splitString(cpuInfo, "\n");
	if(lines.empty())
		return "";
	for(const auto& line : lines){
		if(line.starts_with("model name")){
			return (std::string)stripStr(splitString(line, ":")[1]);
		}
	}

	return "";
}

std::string execCommand(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
	    throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
	    result += buffer.data();
	}
	return result;
}

std::size_t memDenomToSize(std::string_view denom){
	if(denom == "KB" || denom == "KiB")
		return 1024;
	if(denom == "MB" || denom == "MiB")
		return 1024 * 1024;
	if(denom == "GB" || denom == "GiB")
		return 1024 * 1024 * 1024;
	return 0;
}

void systemInfo::updateGpuStats(){
	std::string amdinfo = execCommand("amdgpu_top -d -J");
	if(amdinfo.empty())
		return;
	gpuStats.temp = 0;
	auto j = nlohmann::json::parse(amdinfo);
	const auto& gpu = j[0];
	if(!gpu.contains("DeviceName") || !gpu["DeviceName"].is_string()){
		std::cerr<<"Error: amdgpu_top output did not contain a DeviceName field"<<std::endl;
		return;
	}
	gpuStats.name = gpu["DeviceName"];
	if(!gpu.contains("gfx_target_version") || !gpu["gfx_target_version"].is_string()){
		std::cerr<<"Error: amdgpu_top output did not contain a gfx_target_version field"<<std::endl;
		return;
	}
	gpuStats.gfxName = gpu["gfx_target_version"];
	if(!gpu.contains("Sensors") || !gpu["Sensors"].contains("Average Power") || !gpu["Sensors"]["Average Power"].contains("value") || !gpu["Sensors"]["Average Power"]["value"].is_number()){
		std::cerr<<"Error: amdgpu_top output did not contain a Sensors->Average Power->value field"<<std::endl;
		return;
	}
	gpuStats.powerDraw = gpu["Sensors"]["Average Power"]["value"];
	for(const auto& sensor : gpu["Sensors"]){
		if(sensor.contains("unit") && sensor.contains("value") && sensor["value"].is_number() && sensor["unit"] == "C")
			gpuStats.temp = std::max(gpuStats.temp, sensor["value"].get<float>());
	}
	if(!gpu.contains("VRAM") || !gpu["VRAM"].contains("Total VRAM") || !gpu["VRAM"]["Total VRAM"].contains("value") || !gpu["VRAM"]["Total VRAM"]["value"].is_number()){
		std::cerr<<"Error: amdgpu_top output did not contain a VRAM->Total VRAM->value field"<<std::endl;
		return;
	}
	gpuStats.totalVram = gpu["VRAM"]["Total VRAM"]["value"].get<std::size_t>() * memDenomToSize(gpu["VRAM"]["Total VRAM"]["unit"].get<std::string>());
	if(!gpu.contains("VRAM") || !gpu["VRAM"].contains("Total VRAM Usage") || !gpu["VRAM"]["Total VRAM Usage"].contains("value") || !gpu["VRAM"]["Total VRAM Usage"]["value"].is_number()){
		std::cerr<<"Error: amdgpu_top output did not contain a VRAM->Total VRAM Usage->value field"<<std::endl;
		return;
	}
	gpuStats.usedVram = gpu["VRAM"]["Total VRAM Usage"]["value"].get<std::size_t>() * memDenomToSize(gpu["VRAM"]["Total VRAM Usage"]["unit"].get<std::string>());
	gpuStats.usage = 0;
	if(!gpu.contains("gpu_activity")){
		std::cerr<<"Error: amdgpu_top output did not contain a gpu_activity field"<<std::endl;
		return;
	}
	for(const auto& activity : gpu["gpu_activity"]){
		if(activity.contains("unit") && activity.contains("value") && activity["value"].is_number() && activity["unit"] == "%")
			gpuStats.usage = std::max(gpuStats.usage, activity["value"].get<float>());
	}
}

