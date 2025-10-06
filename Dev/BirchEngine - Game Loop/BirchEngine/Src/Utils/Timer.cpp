#include "Timer.hpp"
std::map<std::string, StopwatchTreeVer> Timer::stopwatchesT{};

void Timer::displayTimerStats(std::vector<std::string> innerTimers, std::string outerTimer)
{
	long long outerTime = Timer::calcTotalTime(outerTimer, Units::ms);
	long long tot = outerTime;
	//output outer time stats
	std::cout << outerTimer << ": (Tot: " << outerTime << "ms|Avg: " << static_cast<float>(Timer::calcAverageTime(outerTimer,Units::ns))/1000000 <<"ms) / 100% \n";
	
	for (std::string& inner : innerTimers) {
		long long it = Timer::calcTotalTime(inner, Units::ms);
		std::cout << "  " << inner << ":";
		for (int i = inner.size(); i < 30; ++i) std::cout << " ";
		std::cout << "(Tot: " << it << "ms|Avg: " << static_cast<float>(Timer::calcAverageTime(inner, Units::ns))/1000000 <<"ms) / " << it * 100 / (outerTime > 0? outerTime : 0) << "% \n";
		tot -= it;
	}
	std::cout << "	other/idle:";
	for (int i = 8; i < 30; ++i) std::cout << " ";
	std::cout << tot << "ms / " << tot * 100 / outerTime << "%\n";
}

void Timer::displayTimerStats(std::string timer, std::size_t maxDepth)
{
	StopwatchTreeVer* sw = &stopwatchesT[timer];
	long long outerTime = sw->getTime(Units::ms);
	long long tot = outerTime;
	displayTimerStats(sw, maxDepth, 0u);
}

long long Timer::displayTimerStats(StopwatchTreeVer*& sw, std::size_t maxDepth, std::size_t curDepth)
{
	long long swTimeMs = sw->getTime(Units::ms);
	long long idleTime = swTimeMs;
	long long parentTimeMs = swTimeMs;
	if(sw->getParent() != nullptr ) parentTimeMs = sw->getParent()->getTime(Units::ms);

	//indentations to show parent-child relationship
	std::vector<int> paddings{ 2 * int(curDepth), 30, 7, 15 };
	std::string indent{ "" }; indent.append(paddings[0], ' ');
	std::cout << indent << sw->getName() << ":";
	indent = ""; indent.append(paddings[1] - sw->getName().size(), ' ');
	std::cout << indent << "(Tot: " << swTimeMs << "ms";
	indent = ""; indent.append(paddings[2] - std::to_string(swTimeMs).size(), ' ');
	std::cout << indent << "| Avg :" << sw->getAvgLapTime(Units::ns) / 1000000.0f << "ms)";
	indent = ""; indent.append(paddings[3] - std::to_string(sw->getAvgLapTime(Units::ns) / 1000000.0f).size(), ' ');
	std::cout << indent << "/ " << swTimeMs * 100 / (parentTimeMs ==0 ? 1 : parentTimeMs) << " % of parent\n";
	
	if (curDepth < maxDepth && sw->getChildren().size()>0) {
		for (StopwatchTreeVer*& inner : sw->getChildren()) {
			idleTime -= displayTimerStats(inner, maxDepth, curDepth + 1);
		}
		indent = ""; indent.append(paddings[0], ' ');
		std::cout << indent<<"  other/idle: ";
		std::cout << idleTime << "ms / " << idleTime * 100 / swTimeMs << "%\n";
	}
	
	return swTimeMs;
}
