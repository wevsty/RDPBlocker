#include "boost_regex_utils.h"

bool regex_find_match(const boost::regex& regex_expr, const std::string& data)
{
    boost::sregex_iterator it(data.begin(), data.end(), regex_expr);
    const boost::sregex_iterator it_end;
    for (; it != it_end; ++it)
    {
        return true;
    }
    return false;
}

bool regex_find_match(const std::string& regex_expr, const std::string& data)
{
    boost::regex expression(regex_expr);
    boost::sregex_iterator it(data.begin(), data.end(), expression);
    const boost::sregex_iterator it_end;
    for (; it != it_end; ++it)
    {
        return true;
    }
    return false;
}
