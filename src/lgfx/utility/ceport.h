#ifndef _CE_PORT_H
#define _CE_PORT_H

#define MINIZ_HEADER_FILE_ONLY

#ifdef _MSC_VER
#  define PACKED_STRUCT(name) \
    __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif defined(__GNUC__)
#  define PACKED_STRUCT(name) struct __attribute__((packed)) name
#endif

#endif // !_CE_PORT_H

