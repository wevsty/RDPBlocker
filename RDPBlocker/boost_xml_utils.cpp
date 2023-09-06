#include "boost_xml_utils.h"

boost::property_tree::ptree read_xml_from_string(const std::string& data)
{
    std::stringstream stream_buffer;
    stream_buffer << data;
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(stream_buffer, pt);

    return pt;
}
