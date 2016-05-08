#
# This file is part of fast-lib.
# Copyright (C) 2015 RWTH Aachen University - ACS
#
# This file is licensed under the GNU Lesser General Public License Version 3
# Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
#

#get_property(BOOST_LOG_LIBRARY TARGET ${BoostLogLibs} PROPERTY LOCATION)
#get_property(BOOST_CHRONO_LIBRARY TARGET ${BoostChronoLibs} PROPERTY LOCATION) # dependency of boost.log
#get_property(BOOST_DATE_TIME_LIBRARY TARGET ${BoostDateTimeLibs} PROPERTY LOCATION) # dependency of boost.log
#get_property(BOOST_FILESYSTEM_LIBRARY TARGET ${BoostFilesystemLibs} PROPERTY LOCATION) # dependency of boost.log
#get_property(BOOST_THREAD_LIBRARY TARGET ${BoostThreadLibs} PROPERTY LOCATION) # dependency of boost.log
#get_property(BOOST_SYSTEM_LIBRARY TARGET ${BoostSystemLibs} PROPERTY LOCATION) # dependency of boost.log
#set(EXAMPLE_LIBS ${BOOST_LOG_LIBRARY} ${BOOST_CHRONO_LIBRARY} ${BOOST_DATE_TIME_LIBRARY} ${BOOST_FILESYSTEM_LIBRARY} ${BOOST_THREAD_LIBRARY} ${BOOST_SYSTEM_LIBRARY})

add_subdirectory(communication)
add_subdirectory(comm_serial)
