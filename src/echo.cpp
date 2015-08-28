/**
 * @file echo.hpp
 *
 * @author jeff.daily@pnnl.gov
 */
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include "echo.hpp"

using namespace std;

fncs::Echo::Echo()
    : os(), cout_enabled(false)
{
}

fncs::Echo::Echo(const string &filename, ios_base::openmode mode)
    : os(filename.c_str(), mode), cout_enabled(false)
{
}

fncs::Echo::Echo(const char *filename, ios_base::openmode mode)
    : os(filename, mode), cout_enabled(false)
{
}

fncs::Echo::~Echo()
{
}

void fncs::Echo::open(const string &filename, ios_base::openmode mode) {
    this->os.open(filename.c_str(), mode);
}

void fncs::Echo::open(const char *filename, ios_base::openmode mode) {
    this->os.open(filename, mode);
}

void fncs::Echo::close() {
    this->os.close();
}

void fncs::Echo::enable_stdout() {
    this->cout_enabled = true;
}

void fncs::Echo::disable_stdout() {
    this->cout_enabled = false;
}

fncs::Echo& fncs::Echo::operator<< (bool val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (short val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (unsigned short val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (int val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (unsigned int val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (long val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (unsigned long val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (long long val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (unsigned long long val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (float val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (double val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (long double val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (void* val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (streambuf* val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (ostream& (*pf)(ostream&)) {
    if (this->os.is_open()) {
        pf(this->os);
    }
    if (this->cout_enabled) {
        pf(cout);
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (ios& (*pf)(ios&)) {
    if (this->os.is_open()) {
        pf(this->os);
    }
    if (this->cout_enabled) {
        pf(cout);
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (ios_base& (*pf)(ios_base&)) {
    if (this->os.is_open()) {
        pf(this->os);
    }
    if (this->cout_enabled) {
        pf(cout);
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (char val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (signed char val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (unsigned char val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (const char* val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (const signed char* val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

fncs::Echo& fncs::Echo::operator<< (const unsigned char* val) {
    if (this->os.is_open()) {
        this->os << val;
    }
    if (this->cout_enabled) {
        cout << val;
    }
    return *this;
}

