#ifndef FAST_LOG_HPP
#define FAST_LOG_HPP

namespace fast {
	namespace log {
		class Dev_null
		{
		};
	       	extern Dev_null dev_null;
		template<typename T> Dev_null & operator<<(Dev_null &dest, T)
		{
			return dest;
		}
	}
}

#ifdef FASTLIB_ENABLE_LOGGING
	#include <spdlog/spdlog.h>

	#define FASTLIB_LOG_INIT(var, name) namespace fast { namespace log {auto var = spdlog::stdout_logger_mt(name); } }

	#define FASTLIB_LOG_SET_LEVEL(var, lvl) (fast::log::var->set_level(spdlog::level::lvl))

	#define FASTLIB_LOG_SET_LEVEL_GLOBAL(var, lvl) \
		namespace fast { namespace log { \
			struct Level_setter_var { \
				Level_setter_var (){ \
					fast::log::var->set_level(spdlog::level::lvl); \
				} \
			}; \
			Level_setter_var level_setter_var;\
		}}

	#define FASTLIB_LOG(var, lvl) (fast::log::var->lvl())
#else
	#define FASTLIB_LOG_INIT(var, name)
	#define FASTLIB_LOG_SET_LEVEL(var, lvl)
	#define FASTLIB_LOG_SET_LEVEL_GLOBAL(var, lvl)
	#define FASTLIB_LOG(var, lvl) fast::log::dev_null
#endif

#endif
