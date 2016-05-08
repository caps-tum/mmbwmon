#ifndef FAST_LIB_MESSAGE_MIGFRA_TIME_MEASUREMENT_HPP
#define FAST_LIB_MESSAGE_MIGFRA_TIME_MEASUREMENT_HPP

#include <fast-lib/serializable.hpp>

#include <string>
#include <unordered_map>

namespace fast {
namespace msg {
namespace migfra {

using nanosecond_type = std::int_least64_t;

struct Times
{
	nanosecond_type wall;
	void clear();
};

class Timer
{
public:
	Timer();
	~Timer() noexcept = default;
	Timer(const Timer &rhs) noexcept = default;
	Timer & operator=(const Timer &rhs) noexcept = default;

	bool is_stopped() const noexcept;
	Times elapsed() const noexcept;
	std::string format() const;

	void start() noexcept;
	void stop() noexcept;
	void resume() noexcept;
private:
	Times times;
	bool stopped;
};

// TODO: Add timer guard.
// TODO: Split in serialization and implementation part so that the namespace fits
class Time_measurement :
	public fast::Serializable
{
public:
	explicit Time_measurement(bool enable_time_measurement = false);
	~Time_measurement();

	void tick(const std::string &timer_name);
	void tock(const std::string &timer_name);

	bool empty() const;

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;
private:
	bool enabled;
	std::unordered_map<std::string, Timer> timers;
};

}
}
}

YAML_CONVERT_IMPL(fast::msg::migfra::Time_measurement)

#endif
