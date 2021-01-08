#ifndef __STD_EXPIRES_MAP_UTILS__
#define __STD_EXPIRES_MAP_UTILS__

#include <map>
#include <iostream>

template <typename T_MAP>
void delete_expire_keys(T_MAP& map)
{
	typename T_MAP::iterator it;
	for (it = map.begin(); it != map.end();)
	{
		typename T_MAP::iterator current_it = it;
		it++;
		if (current_it->second.is_expired() == true)
		{
			map.erase(current_it);
			// std::cout << "delete key" << std::endl;
		}
	}
}


#endif //__STD_EXPIRES_MAP_UTILS__