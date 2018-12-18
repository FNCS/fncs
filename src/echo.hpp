#ifndef _ECHO_HPP_
#define _ECHO_HPP_

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#if (defined WIN32 || defined _WIN32)
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

class FNCS_EXPORT Echo
{
    public:
        explicit Echo();
        explicit Echo(const std::string &filename,
                std::ios_base::openmode mode=std::ios_base::out);
        explicit Echo(const char *filename,
                std::ios_base::openmode mode=std::ios_base::out);
		~Echo();
        void open(const std::string &filename,
                std::ios_base::openmode mode=std::ios_base::out);
        void open(const char *filename,
                std::ios_base::openmode mode=std::ios_base::out);
        void close();
        void enable_stdout();
        void disable_stdout();
        Echo& operator<< (bool val);
        Echo& operator<< (short val);
        Echo& operator<< (unsigned short val);
        Echo& operator<< (int val);
        Echo& operator<< (unsigned int val);
        Echo& operator<< (long val);
        Echo& operator<< (unsigned long val);
        Echo& operator<< (long long val);
        Echo& operator<< (unsigned long long val);
        Echo& operator<< (float val);
        Echo& operator<< (double val);
        Echo& operator<< (long double val);
        Echo& operator<< (void* val);
        Echo& operator<< (std::streambuf* val);
        Echo& operator<< (std::ostream& (*pf)(std::ostream&));
        Echo& operator<< (std::ios& (*pf)(std::ios&));
        Echo& operator<< (std::ios_base& (*pf)(std::ios_base&));
        Echo& operator<< (char val);
        Echo& operator<< (signed char val);
        Echo& operator<< (unsigned char val);
        Echo& operator<< (const char* val);
        Echo& operator<< (const signed char* val);
        Echo& operator<< (const unsigned char* val);

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

