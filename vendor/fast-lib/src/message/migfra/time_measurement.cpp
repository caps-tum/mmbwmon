#include <fast-lib/message/migfra/time_measurement.hpp>
#include <fast-lib/log.hpp>

#include <stdexcept>

FASTLIB_LOG_INIT(tm_log, "Time_measurement")

namespace fast {
namespace msg {
namespace migfra {

const double sec_in_nano = 1000000000.0L;

//
// Timer implementation
//

Timer::Timer(timepoint_type base_point) :
	base_point(base_point)
{
	start();
}

bool Timer::is_stopped() const noexcept
{
	return stopped;
}

Timer::duration_type Timer::elapsed() const
{
	if (is_stopped())
		return stop_point - start_point;
	return clock::now() - start_point;
}

double Timer::wall_sec() const
{
	auto duration = elapsed();
	return static_cast<double>(duration.count()) / sec_in_nano;
}

double Timer::start_sec() const
{
	duration_type start_duration = start_point - base_point;
	return static_cast<double>(start_duration.count()) / sec_in_nano;
}

double Timer::stop_sec() const
{
	duration_type stop_duration = stop_point - base_point;
	return static_cast<double>(stop_duration.count()) / sec_in_nano;
}

std::string Timer::format(const std::string &format) const
{
	if (format == "timestamps") {
		return "wall: " + std::to_string(wall_sec()) + " started: " + std::to_string(start_sec()) + " stopped: " + std::to_string(stop_sec());
	} else {
		return std::to_string(wall_sec());
	}
}

void Timer::start() noexcept
{
	start_point = clock::now();
	stopped = false;
}

void Timer::stop() noexcept
{
	if (is_stopped())
		return;
	stop_point = clock::now();
	stopped = true;
}

void Timer::resume() noexcept
{
	start_point = clock::now() - elapsed();
	stopped = false;
}

//
// Time_measurement implementation
//

Time_measurement::Time_measurement(bool enable_time_measurement, std::string format, Timer::timepoint_type base_point) :
	enabled(enable_time_measurement),
	format(format),
	base_point(base_point)
{
}

Time_measurement::~Time_measurement()
{
	for (auto &timer : timers) {
		if (!timer.second.is_stopped()) {
			FASTLIB_LOG(tm_log, warn) << "Timer with name \"" + timer.first + "\" has not been stopped, but task is finished. Search for a tick without subsequent tock or ignore if there was a preceding error.";
		}
	}
}

void Time_measurement::tick(const std::string &timer_name)
{
	if (enabled) {
		if (timers.find(timer_name) != timers.end())
			throw std::runtime_error("Timer with name \"" + timer_name + "\" already exists.");
		timers[timer_name] = Timer(base_point);
	}
}

void Time_measurement::tock(const std::string &timer_name)
{
	if (enabled) {
		try {
			timers.at(timer_name).stop();
		} catch (const std::out_of_range /*&e*/) {
			throw std::runtime_error("Timer with name \"" + timer_name + "\" not found. Search for a tock without preceding tick.");
		}
	}
}

bool Time_measurement::empty() const
{
	return timers.empty();
}

YAML::Node Time_measurement::emit() const
{
	YAML::Node node;
	for (auto &timer : timers) {
		node[timer.first] = timer.second.format(format);
	}
	return node;
}

void Time_measurement::load(const YAML::Node &node)
{
	(void) node;
	const std::string err_str("Class Time_measurement does not support loading from YAML.");
	FASTLIB_LOG(tm_log, error) << err_str;
	throw std::runtime_error(err_str);
}

}
}
}
