#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <iostream>

typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::chrono::steady_clock::duration NanoS;

//defines a zero time
#define DEFAULT_TIME std::chrono::time_point<std::chrono::steady_clock,NanoS>()


inline TimePoint now() {
	return std::chrono::steady_clock::now();
}

class Stopwatch {
	bool stopped = true;
	long long lapCount{ 0L };
	std::pair<NanoS, TimePoint> totAndLastStart{ 0, DEFAULT_TIME };
public:
	Stopwatch() {}
	inline NanoS getTime() {
		//if currently stopped, return time measured
		if (stopped) return totAndLastStart.first;
		//if not stopped, return time measured + time since last start
		return totAndLastStart.first+ (now() - totAndLastStart.second);
	}
	inline void startStopwatch() {
		if (!stopped) stopStopwatch();
		stopped = false;
		totAndLastStart.second = now();	
	}
	inline void stopStopwatch() {
		if (stopped) return;

		//add time since starting to total
		totAndLastStart.first += now() - totAndLastStart.second;

		stopped = true;
		lapCount += 1;
	}
	inline long long getLaps() {
		return lapCount;
	}
};

namespace Units{
	enum Unit{ s, ms, us, ns};
	static std::map<Units::Unit, unsigned int> divs{
			{Units::Unit::s,		1000000000},
			{Units::Unit::ms,	1000000},
			{Units::Unit::us,	1000},
			{Units::Unit::ns,	1}
	};
}

class StopwatchTreeVer {
	std::string name{ "Unnamed" };
	bool stopped = true;
	long long lapCount{ 0L };
	std::pair<NanoS, TimePoint> totAndLastStart{ 0, DEFAULT_TIME };

	StopwatchTreeVer* parent{ nullptr };
	std::vector<StopwatchTreeVer*> childStopwatches{};
public:
	StopwatchTreeVer() {};
	void setName(std::string n) { name = n; }
	inline long long getTime(Units::Unit unit) {
		//if currently stopped, return time measured
		if (stopped) return totAndLastStart.first.count() / Units::divs[unit];
		//if not stopped, return time measured + time since last start
		return (totAndLastStart.first + (now() - totAndLastStart.second)).count() / Units::divs[unit]
;
	}
	inline long long getAvgLapTime(Units::Unit unit) {
		return getTime(unit) / (lapCount > 0 ? lapCount : 1) / Units::divs[unit];

	}
	inline void startStopwatch() {
		if (!stopped) stopStopwatch();
		stopped = false;
		totAndLastStart.second = now();
	}
	inline void stopStopwatch() {
		if (stopped) return;

		//add time since starting to total
		totAndLastStart.first += now() - totAndLastStart.second;

		stopped = true;
		lapCount += 1;
	}
	inline long long getLaps() {
		return lapCount;
	}
	void reset() {
		totAndLastStart.first = NanoS{ 0 };
		totAndLastStart.second = now();
	}

	inline std::string getName() { return name; }
	bool hasParent() { return parent != nullptr; }
	void setParent(StopwatchTreeVer& stopwatch) { parent = &stopwatch; }
	StopwatchTreeVer* getParent() { return parent; }
	void addChild(StopwatchTreeVer& stopwatch) { childStopwatches.push_back(&stopwatch); }
	std::vector<StopwatchTreeVer*> getChildren() { return childStopwatches; }
	bool hasStarted() {
		return !stopped;
	}
	TimePoint getStartTime() {
		return totAndLastStart.second;
	}
};

class Timer {
	static std::map<std::string, StopwatchTreeVer> stopwatchesT;
public:
	static inline void startTimer(std::string timerName) {
		stopwatchesT[timerName].startStopwatch();

		//tree xperiments
		//if stopwatch has no parent (likely initial creation of stopwatch) try to set its parent using most recently started/not stopped stopwatch
		if (!stopwatchesT[timerName].hasParent()) {
			stopwatchesT[timerName].setName(timerName);
			std::pair<StopwatchTreeVer*, TimePoint> mostRecentStart{ nullptr,DEFAULT_TIME };

			//for all stopwatches currently timing, whichever one was started most recently is assumed to most closely contain this stopwatch
			//FIX: could be problematic when used in tandem with multithreading
			//SOLUTIONS: 
			// 1) force parents to be declared on stopwatch creation (which will add children tot he parents)
			// 2) some sort of thread tracking function? would also be good for tracking time saved by multithreading
			for (std::pair<const std::string, StopwatchTreeVer>& p : stopwatchesT) {
				if (&p.second != &stopwatchesT[timerName] 
					&&p.second.hasStarted() && p.second.getStartTime().time_since_epoch() > mostRecentStart.second.time_since_epoch()) {
					mostRecentStart.first = &p.second;
					mostRecentStart.second = p.second.getStartTime();
				}
			}

			if (mostRecentStart.first != nullptr) {
				stopwatchesT[timerName].setParent(*mostRecentStart.first);
				mostRecentStart.first->addChild(stopwatchesT[timerName]);
				//FIX: should have better implementation I think
				// this resets any uninitialised info i.e. from RenderBuckets.hpp sorting before the proper game loop
				stopwatchesT[timerName].reset();
			}
		}

	}
	static inline void stopTimer(std::string timerName) {
		stopwatchesT[timerName].stopStopwatch();
	}
	static inline long long getStopwatchTime(std::string timerName, Units::Unit unit) {
		return stopwatchesT[timerName].getTime(unit);
	}

	static inline long long calcAverageTime(std::string timerName, Units::Unit unit) {
		return stopwatchesT[timerName].getAvgLapTime(unit);
	}
	static inline long long calcTotalTime(std::string timerName, Units::Unit unit) {
		return stopwatchesT[timerName].getTime(unit);
	}
	static void displayTimerStats(std::vector<std::string> innerTimers, std::string outerTimer);
	static void displayTimerStats(std::string timer, std::size_t maxDepth);
	static long long displayTimerStats(StopwatchTreeVer*& sw, std::size_t maxDepth, std::size_t curDepth);
};

