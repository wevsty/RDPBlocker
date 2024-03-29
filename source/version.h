#ifndef __APPLICATION_VERSION__
#define __APPLICATION_VERSION__

#define XSTRINGIZER(source) #source
#define STRINGIZER(source) XSTRINGIZER(source)

#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_REVISION 6
#define VERSION_BUILD 5

#define APPLICATION_FILE_VERSION \
    VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define APPLICATION_FILE_VERSION_STRING           \
    STRINGIZER(VERSION_MAJOR)                     \
    "." STRINGIZER(VERSION_MINOR) "." STRINGIZER( \
        VERSION_REVISION) "." STRINGIZER(VERSION_BUILD)

#define APPLICATION_PRODUCT_VERSION APPLICATION_FILE_VERSION
#define APPLICATION_PRODUCT_VERSION_STRING APPLICATION_FILE_VERSION_STRING

#endif  //__APPLICATION_VERSION__
