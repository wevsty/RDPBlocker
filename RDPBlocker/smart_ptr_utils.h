#ifndef __STD_SMART_PTR_UTILS__
#define __STD_SMART_PTR_UTILS__

#include <memory>

namespace std 
{
	template<typename TYPE>
	std::shared_ptr<TYPE> make_shared_array(std::size_t array_size)
	{
		return std::shared_ptr<TYPE>(new TYPE[array_size], std::default_delete <TYPE[]>());
	}
}


#endif //__STD_SMART_PTR_UTILS__
