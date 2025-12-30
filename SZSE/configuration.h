// this file derived from inih project
// to read ini file
//
// Created by dzg on 2025/12/30.
//
#ifndef SZSE_CONFIGURATION_H
#define SZSE_CONFIGURATION_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <iostream>

//std::string
#include <string>

//uint64_t
#include <cstdint>

//std::transform
#include <algorithm>

typedef struct {
    char szServerIP[50];
    int iPort = 0;
    char szLocalName[20];
    char szTargetName[20];
    int iHeartBeat = 0;;
    char szPassword[16];
    char szVersion[32];
} Configuration;

class INIReader {
public:
    // Construct INIReader and parse given filename. See ini.h for more info
    // about the parsing.
    explicit INIReader(const std::string &filename);

    // Return the result of ini_parse(), i.e., 0 on success, line number of
    // first error on parse error, -1 on file open error, or -2 if there was a
    // memory allocation error.
    int ParseError() const;

    // Return a message that describes the type of error that occurred.
    // It will return "" (empty string) if there was no error.
    std::string ParseErrorMessage() const;

    // Get a string value from INI file, returning default_value if not found.
    std::string Get(const std::string &section, const std::string &name,
                    const std::string &default_value) const;

    // Get a string value from INI file, returning default_value if not found,
    // empty, or contains only whitespace.
    std::string GetString(const std::string &section, const std::string &name,
                          const std::string &default_value) const;

    // Get an integer (long) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    long GetInteger(const std::string &section, const std::string &name, long default_value) const;

    // Get a 64-bit integer (int64_t) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    int64_t GetInteger64(const std::string &section, const std::string &name, int64_t default_value) const;

    // Get an unsigned integer (unsigned long) value from INI file, returning default_value if
    // not found or not a valid unsigned integer (decimal "1234", or hex "0x4d2").
    unsigned long GetUnsigned(const std::string &section, const std::string &name, unsigned long default_value) const;

    // Get an unsigned 64-bit integer (uint64_t) value from INI file, returning default_value if
    // not found or not a valid unsigned integer (decimal "1234", or hex "0x4d2").
    uint64_t GetUnsigned64(const std::string &section, const std::string &name, uint64_t default_value) const;

protected:
    int _error;
    std::map<std::string, std::string> _values;

    static std::string MakeKey(const std::string &section, const std::string &name);

    static int ValueHandler(void *user, const char *section, const char *name,
                            const char *value);
};
#endif //SZSE_CONFIGURATION_H