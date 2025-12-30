// this file derived from inih project
// to read ini file
//
// Created by dzg on 2025/12/30.
//
#include "configuration.h"

#define INI_START_COMMENT_PREFIXES ";#"
#define INI_MAX_LINE 200
#define MAX_SECTION 50
#define MAX_NAME 50

static char *ini_rstrip(char *s, char *end) {
    while (end > s && isspace((unsigned char) (*--end)))
        *end = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char *ini_lskip(const char *s) {
    while (*s && isspace((unsigned char) (*s)))
        s++;
    return (char *) s;
}

static char *ini_find_chars_or_comment(const char *s, const char *chars) {
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
    return (char *) s;
}

/* Similar to strncpy, but ensures dest (size bytes) is
   NUL-terminated, and doesn't pad with NULs. */
static char *ini_strncpy0(char *dest, const char *src, size_t size) {
    /* Could use strncpy internally, but it causes gcc warnings (see issue #91) */
    size_t i;
    for (i = 0; i < size - 1 && src[i]; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

/* Typedef for prototype of fgets-style reader function. */
typedef char * (*ini_reader)(char *str, int num, void *stream);

typedef int (*ini_handler)(void *user, const char *section,
                           const char *name, const char *value);

/* See documentation in header file. */
int ini_parse_stream(ini_reader reader, void *stream, ini_handler handler,
                     void *user) {
    /* Uses a fair bit of stack (use heap instead if you need to) */

    char line[INI_MAX_LINE];
    size_t max_line = INI_MAX_LINE;

    char section[MAX_SECTION] = "";

    size_t offset;
    char *start;
    char *end;
    char *name;
    char *value;
    int lineno = 0;
    int error = 0;

#define HANDLER(u, s, n, v) handler(u, s, n, v)

    /* Scan through stream line by line */
    while (reader(line, (int) max_line, stream) != NULL) {
        offset = strlen(line);

        lineno++;

        start = line;
        start = ini_rstrip(ini_lskip(start), line + offset);

        if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
            /* Start-of-line comment */
        } else if (*start == '[') {
            /* A "[section]" line */
            end = ini_find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                ini_strncpy0(section, start + 1, sizeof(section));
            } else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        } else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = ini_find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = ini_rstrip(start, end);
                value = end + 1;

                value = ini_lskip(value);
                ini_rstrip(value, end);

                /* Valid name[=:]value pair found, call handler */
                if (!HANDLER(user, section, name, value) && !error)
                    error = lineno;
            } else {
                /* No '=' or ':' found on name[=:]value line */
                if (!error)
                    error = lineno;
            }
        }
#if 0
        if (error)
            break;
#endif
    }
    return error;
}

/* See documentation in header file. */
int ini_parse_file(FILE *file, ini_handler handler, void *user) {
    return ini_parse_stream((ini_reader) fgets, file, handler, user);
}

/* See documentation in header file. */
int ini_parse(const char *filename, ini_handler handler, void *user) {
    FILE *file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}



INIReader::INIReader(const std::string &filename) {
    _error = ini_parse(filename.c_str(), ValueHandler, this);
}

int INIReader::ParseError() const {
    return _error;
}

std::string INIReader::ParseErrorMessage() const {
    // If _error is positive it means it is the line number on which a parse
    // error occurred. This could be an overlong line, that ValueHandler
    // indicated a user defined error, an unterminated section name, or a name
    // without a value.
    if (_error > 0) {
        return "parse error on line " + std::to_string(_error) + "; missing ']' or '='?";
    }

    // If _error is negative it is a system type error, and 0 means success.
    switch (_error) {
        case -2:
            return "unable to allocate memory";

        case -1:
            return "unable to open file";

        case 0:
            return "";
    }

    // This should never be reached. It probably means a new error code was
    // added to the C API without updating this method.
    return "unknown error " + std::to_string(_error);
}

std::string INIReader::Get(const std::string &section, const std::string &name,
                           const std::string &default_value) const {
    std::string key = MakeKey(section, name);
    // Use _values.find() here instead of _values.at() to support pre C++11 compilers
    return _values.count(key) ? _values.find(key)->second : default_value;
}

std::string INIReader::GetString(const std::string &section, const std::string &name,
                                 const std::string &default_value) const {
    const std::string str = Get(section, name, "");
    return str.empty() ? default_value : str;
}

long INIReader::GetInteger(const std::string &section, const std::string &name, long default_value) const {
    std::string valstr = Get(section, name, "");
    const char *value = valstr.c_str();
    char *end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : default_value;
}

int64_t INIReader::GetInteger64(const std::string &section, const std::string &name, int64_t default_value) const {
    std::string valstr = Get(section, name, "");
    const char *value = valstr.c_str();
    char *end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    int64_t n = strtoll(value, &end, 0);
    return end > value ? n : default_value;
}

unsigned long INIReader::GetUnsigned(const std::string &section, const std::string &name,
                                     unsigned long default_value) const {
    std::string valstr = Get(section, name, "");
    const char *value = valstr.c_str();
    char *end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    unsigned long n = strtoul(value, &end, 0);
    return end > value ? n : default_value;
}

uint64_t INIReader::GetUnsigned64(const std::string &section, const std::string &name, uint64_t default_value) const {
    std::string valstr = Get(section, name, "");
    const char *value = valstr.c_str();
    char *end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    uint64_t n = strtoull(value, &end, 0);
    return end > value ? n : default_value;
}


std::string INIReader::MakeKey(const std::string &section, const std::string &name) {
    std::string key = section + "=" + name;
    // Convert to lower case to make section/name lookups case-insensitive
    std::transform(key.begin(), key.end(), key.begin(),
                   [](const unsigned char &ch) { return static_cast<unsigned char>(::tolower(ch)); });
    return key;
}

int INIReader::ValueHandler(void *user, const char *section, const char *name,
                            const char *value) {
    if (!name) // Happens when INI_CALL_HANDLER_ON_NEW_SECTION enabled
        return 1;
    INIReader *reader = static_cast<INIReader *>(user);
    std::string key = MakeKey(section, name);
    if (reader->_values[key].size() > 0)
        reader->_values[key] += "\n";
    reader->_values[key] += value ? value : "";
    return 1;
}


