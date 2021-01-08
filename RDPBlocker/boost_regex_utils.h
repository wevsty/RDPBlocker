#ifndef __BOOST_REGEX_UTILS__
#define __BOOST_REGEX_UTILS__

#include <string>

#include <boost/regex.hpp>

bool regex_find_match(const boost::regex& regex_expr, const std::string& data);
bool regex_find_match(const std::string& regex_expr, const std::string& data);

#endif // __BOOST_REGEX_UTILS__