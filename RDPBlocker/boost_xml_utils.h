#ifndef __BOOST_XML_PARSE__
#define __BOOST_XML_PARSE__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>
#include <string>

boost::property_tree::ptree read_xml_from_string(const std::string& data);

#endif  //__BOOST_XML_PARSE__