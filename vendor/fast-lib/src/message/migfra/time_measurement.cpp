#include <fast-lib/message/migfra/time_measurement.hpp>
#include <fast-lib/log.hpp>

#include <stdexcept>
#include <chrono>

FASTLIB_LOG_INIT(tm_log, "Time_measurement")

namespace fast {
namespace msg {
namespace migfra {

void Times::clear()
{
	wall = 0LL;
}

void get_times(Times &current)
{
	std::chrono::duration<nanosecond_type, std::nano> x(std::chrono::high_resolution_clock::now().time_since_epoch());
	current.wall = x.count();
}

Timer::Timer()
{
	start();
}

bool Timer::is_stopped() const noexcept
{
	return stopped;
}

Times Timer::elapsed() const noexcept
{
	if (is_stopped())
		return times;
	Times current;
	get_times(current);
	current.wall -= times.wall;
	return current;
}

std::string Timer::format() const
{
	const double sec = 1000000000.0L;
	double wall_sec = static_cast<double>(times.wall) / sec;
	return std::to_string(wall_sec);
}

void Timer::start() noexcept
{
	stopped = false;
	get_times(times);
}

void Timer::stop() noexcept
{
	if (is_stopped())
		return;
	stopped = true;
	Times current;
	get_times(current);
	times.wall = current.wall - times.wall;
}

void Timer::resume() noexcept
{
	Times current(times);
	start();
	times.wall -= current.wall;
}

Time_measurement::Time_measurement(bool enable_time_measurement) :
	enabled(enable_time_measurement)
{
}

Time_measurement::~Time_measurement()
{
	for (auto &timer : timers) {
		if (!timer.second.is_stopped()) {
			FASTLIB_LOG(tm_log, error) << "Timer with name \"" + timer.first + "\" has not been stopped, but task is finished. Search for a tick without subsequent tock or ignore if there was a preceding error.";
		}
	}
}

void Time_measurement::tick(const std::string &timer_name)
{
	if (enabled) {
		if (timers.find(timer_name) != timers.end())
			throw std::runtime_error("Timer with name \"" + timer_name + "\" already exists.");
		timers[timer_name];
	}
}

void Time_measurement::tock(const std::string &timer_name)
{
	if (enabled) {
		try {
			timers.at(timer_name).stop();
		} catch (const std::out_of_range &e) {
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
		node[timer.first] = timer.second.format();
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
