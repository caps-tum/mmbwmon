#ifndef mmbwmon_helper_hpp
#define mmbwmon_helper_hpp

#include <string>

#include <cassert>
#include <unistd.h>

/*** config vars **/
extern std::string server;
extern size_t port;

inline std::string get_hostname() {
	char hostname[60];
	assert(gethostname(hostname, 60) == 0);
	return std::string(hostname);
}

/*** MQTT constants ***/
const std::string baseTopic = "fast/agent/" + get_hostname() + "/mmbwmon";

#endif /* end of include guard: mmbwmon_helper_hpp */
