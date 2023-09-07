#include "boost_regex_utils.h"

bool regex_find_match(const boost::regex& regex_expr, const std::string& data)
{
    static const boost::sregex_iterator empty_it;
    boost::sregex_iterator it(data.begin(), data.end(), regex_expr);
    if (it != empty_it)
    {
        return true;
    }
    return false;
}

bool regex_find_match(const std::string& regex_expr, const std::string& data)
{
    static const boost::sregex_iterator empty_it;
    boost::regex expression(regex_expr);
    boost::sregex_iterator it(data.begin(), data.end(), expression);
    if (it != empty_it)
    {
        return true;
    }
    return false;
}
