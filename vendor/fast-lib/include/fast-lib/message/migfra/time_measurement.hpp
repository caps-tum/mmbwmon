#ifndef FAST_LIB_MESSAGE_MIGFRA_TIME_MEASUREMENT_HPP
#define FAST_LIB_MESSAGE_MIGFRA_TIME_MEASUREMENT_HPP

#include <fast-lib/serializable.hpp>

#include <string>
#include <unordered_map>
#include <chrono>

namespace fast {
namespace msg {
namespace migfra {

class Timer
{
public:
	using nanosecond_type = std::int_least64_t;
	using clock = std::chrono::high_resolution_clock;
	using duration_type = std::chrono::duration<nanosecond_type, std::nano>;
	using timepoint_type = std::chrono::time_point<clock, duration_type>;

	Timer(timepoint_type base_point = timepoint_type());
	~Timer() = default;
	Timer(const Timer &rhs) = default;
	Timer & operator=(const Timer &rhs) = default;

	bool is_stopped() const noexcept;
	duration_type elapsed() const;
	double wall_sec() const;
	double start_sec() const;
	double stop_sec() const;
	std::string format(const std::string &format = "") const;

	void start() noexcept;
	void stop() noexcept;
	// Note that resume may alter the start_point.
	void resume() noexcept;

	static void set_base_point();
private:
	timepoint_type start_point;
	timepoint_type stop_point;
	bool stopped;
	timepoint_type base_point;
};

// TODO: Add timer guard.
// TODO: Split in serialization and implementation part so that the namespace fits
class Time_measurement :
	public fast::Serializable
{
public:
	explicit Time_measurement(bool enable_time_measurement = false, std::string format = "", Timer::timepoint_type base_point = Timer::clock::now());
	Time_measurement(const Time_measurement &rhs) = default;
	Time_measurement & operator=(const Time_measurement &rhs) = default;
	~Time_measurement();

	void tick(const std::string &timer_name);
	void tock(const std::string &timer_name);

	bool empty() const;

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;
private:
	bool enabled;
	std::unordered_map<std::string, Timer> timers;
	std::string format;
	Timer::timepoint_type base_point;
};

}
}
}

YAML_CONVERT_IMPL(fast::msg::migfra::Time_measurement)

#endif
