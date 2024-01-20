#ifndef __UTF_CONVERT__
#define __UTF_CONVERT__

// boost locale bug on 1.83
#if !defined(NOMINMAX)
#define NOMINMAX 1
#endif

#include <boost/locale.hpp>

template <typename CharOut, typename CharIn>
std::basic_string<CharOut> utf_to_utf(const CharIn* source)
{
    return boost::locale::conv::utf_to_utf<CharOut>(source);
}

template <typename CharOut, typename CharIn>
std::basic_string<CharOut> utf_to_utf(const std::basic_string<CharIn>& source)
{
    return boost::locale::conv::utf_to_utf<CharOut>(source);
}

#endif  //__UTF_CONVERT__
