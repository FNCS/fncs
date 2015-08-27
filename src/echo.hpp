/**
 * @file echo.hpp
 *
 * @author jeff.daily@pnnl.gov
 */
#ifndef _ECHO_HPP_
#define _ECHO_HPP_

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#if defined (__WINDOWS__)
#   if defined LIBFNCS_STATIC
#       define FNCS_EXPORT
#   elif defined LIBFNCS_EXPORTS
#       define FNCS_EXPORT __declspec(dllexport)
#   else
#       define FNCS_EXPORT __declspec(dllimport)
#   endif
#else
#   define FNCS_EXPORT
#endif

namespace fncs {

class Echo
{
    public:
        FNCS_EXPORT explicit Echo();
        FNCS_EXPORT explicit Echo(const std::string &filename,
                std::ios_base::openmode mode=std::ios_base::out);
        FNCS_EXPORT explicit Echo(const char *filename,
                std::ios_base::openmode mode=std::ios_base::out);
        FNCS_EXPORT void open(const std::string &filename,
                std::ios_base::openmode mode=std::ios_base::out);
        FNCS_EXPORT void open(const char *filename,
                std::ios_base::openmode mode=std::ios_base::out);
        FNCS_EXPORT void close();
        FNCS_EXPORT void enable_stdout();
        FNCS_EXPORT void disable_stdout();
        FNCS_EXPORT Echo& operator<< (bool val);
        FNCS_EXPORT Echo& operator<< (short val);
        FNCS_EXPORT Echo& operator<< (unsigned short val);
        FNCS_EXPORT Echo& operator<< (int val);
        FNCS_EXPORT Echo& operator<< (unsigned int val);
        FNCS_EXPORT Echo& operator<< (long val);
        FNCS_EXPORT Echo& operator<< (unsigned long val);
        FNCS_EXPORT Echo& operator<< (long long val);
        FNCS_EXPORT Echo& operator<< (unsigned long long val);
        FNCS_EXPORT Echo& operator<< (float val);
        FNCS_EXPORT Echo& operator<< (double val);
        FNCS_EXPORT Echo& operator<< (long double val);
        FNCS_EXPORT Echo& operator<< (void* val);
        FNCS_EXPORT Echo& operator<< (std::streambuf* val);
        FNCS_EXPORT Echo& operator<< (std::ostream& (*pf)(std::ostream&));
        FNCS_EXPORT Echo& operator<< (std::ios& (*pf)(std::ios&));
        FNCS_EXPORT Echo& operator<< (std::ios_base& (*pf)(std::ios_base&));
        FNCS_EXPORT Echo& operator<< (char val);
        FNCS_EXPORT Echo& operator<< (signed char val);
        FNCS_EXPORT Echo& operator<< (unsigned char val);
        FNCS_EXPORT Echo& operator<< (const char* val);
        FNCS_EXPORT Echo& operator<< (const signed char* val);
        FNCS_EXPORT Echo& operator<< (const unsigned char* val);

        template <typename T>
        Echo& operator<< (const T &val);

    private:
        std::ofstream os;
        bool cout_enabled;
};

template <typename T>
Echo& Echo::operator<< (const T &val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        std::cout << val;
    }
    return *this;
}

template<class charT, class traits, class T>
Echo& operator<< (Echo& os, const T& val)
{
    return os << val;
}

}

#endif /* _ECHO_HPP_ */

