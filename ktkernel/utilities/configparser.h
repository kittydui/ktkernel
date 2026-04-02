#pragma once
#include "mem/string.h"
#include "mem/vector.h"
#include <cstdint>

namespace KtKernel
{

    class TomlParser
    {
    public:
        enum class ValueType : uint8_t
        {
            String,
            Bool,
            Integer
        };

        struct Entry
        {
            char m_section[64];
            char m_key[64];
            char m_value[256];
            ValueType m_type;
            bool m_boolVal;
            int64_t m_intVal;
        };

        bool parse(const char* data, size_t length)
        {
            m_entries.clear();
            char current_section[64] = {};
            size_t pos = 0;

            while (pos < length) {
                size_t line_start = pos;
                while (pos < length && data[pos] != '\n')
                    pos++;
                size_t line_end = pos;
                if (line_end > line_start && data[line_end - 1] == '\r')
                    line_end--;
                if (pos < length)
                    pos++;

                size_t i = skipWhitespace(data, line_start, line_end);
                if (i >= line_end || data[i] == '#')
                    continue;

                if (data[i] == '[') {
                    parseSection(data, i + 1, line_end, current_section, sizeof(current_section));
                    continue;
                }

                size_t eq_pos = i;
                while (eq_pos < line_end && data[eq_pos] != '=')
                    eq_pos++;
                if (eq_pos >= line_end)
                    continue;

                Entry entry = {};
                extractKey(data, i, eq_pos, entry.m_key, sizeof(entry.m_key));
                strncpy(entry.m_section, current_section, sizeof(entry.m_section) - 1);

                size_t val_start = skipWhitespace(data, eq_pos + 1, line_end);
                size_t val_end = trimTrailing(data, val_start, line_end);
                parseValue(data, val_start, val_end, entry);

                m_entries.pushBack(entry);
            }

            return true;
        }

        const char* getString(const char* section, const char* key, const char* defaultVal = "") const
        {
            const Entry* e = find(section, key);
            if (!e)
                return defaultVal;
            return e->m_value;
        }

        bool getBool(const char* section, const char* key, bool defaultVal = false) const
        {
            const Entry* e = find(section, key);
            if (!e || e->m_type != ValueType::Bool)
                return defaultVal;
            return e->m_boolVal;
        }

        int64_t getInt(const char* section, const char* key, int64_t defaultVal = 0) const
        {
            const Entry* e = find(section, key);
            if (!e || e->m_type != ValueType::Integer)
                return defaultVal;
            return e->m_intVal;
        }

        bool hasKey(const char* section, const char* key) const
        {
            return find(section, key) != nullptr;
        }

        bool hasSection(const char* section) const
        {
            for (size_t i = 0; i < m_entries.size(); i++) {
                if (strcmp(m_entries[i].m_section, section) == 0)
                    return true;
            }
            return false;
        }

        size_t entryCount() const
        {
            return m_entries.size();
        }

        const Entry& entry(size_t idx) const
        {
            return m_entries[idx];
        }

    private:
        KtCore::Vector<Entry> m_entries;

        const Entry* find(const char* section, const char* key) const
        {
            for (size_t i = 0; i < m_entries.size(); i++) {
                if (strcmp(m_entries[i].m_section, section) == 0 && strcmp(m_entries[i].m_key, key) == 0)
                    return &m_entries[i];
            }
            return nullptr;
        }

        static size_t skipWhitespace(const char* data, size_t from, size_t to)
        {
            while (from < to && (data[from] == ' ' || data[from] == '\t'))
                from++;
            return from;
        }

        static size_t trimTrailing(const char* data, size_t from, size_t to)
        {
            while (to > from && (data[to - 1] == ' ' || data[to - 1] == '\t'))
                to--;
            return to;
        }

        static void parseSection(const char* data, size_t start, size_t end, char* out, size_t outSize)
        {
            size_t close = start;
            while (close < end && data[close] != ']')
                close++;

            size_t s = skipWhitespace(data, start, close);
            size_t e = trimTrailing(data, s, close);
            size_t len = e - s;
            if (len >= outSize)
                len = outSize - 1;
            strncpy(out, data + s, len);
            out[len] = '\0';
        }

        static void extractKey(const char* data, size_t start, size_t eqPos, char* out, size_t outSize)
        {
            size_t end = trimTrailing(data, start, eqPos);
            size_t len = end - start;
            if (len >= outSize)
                len = outSize - 1;
            strncpy(out, data + start, len);
            out[len] = '\0';
        }

        static void parseValue(const char* data, size_t start, size_t end, Entry& entry)
        {
            size_t len = end - start;

            if (len >= 2 && data[start] == '"' && data[end - 1] == '"') {
                entry.m_type = ValueType::String;
                size_t s_len = len - 2;
                if (s_len >= sizeof(entry.m_value))
                    s_len = sizeof(entry.m_value) - 1;
                unescapeString(data + start + 1, s_len, entry.m_value, sizeof(entry.m_value));
            } else if (len == 4 && strncmp(data + start, "true", 4) == 0) {
                entry.m_type = ValueType::Bool;
                entry.m_boolVal = true;
                strncpy(entry.m_value, "true", sizeof(entry.m_value));
            } else if (len == 5 && strncmp(data + start, "false", 5) == 0) {
                entry.m_type = ValueType::Bool;
                entry.m_boolVal = false;
                strncpy(entry.m_value, "false", sizeof(entry.m_value));
            } else {
                entry.m_type = ValueType::Integer;
                entry.m_intVal = parseInteger(data + start, len);
                size_t copy_len = len < sizeof(entry.m_value) - 1 ? len : sizeof(entry.m_value) - 1;
                strncpy(entry.m_value, data + start, copy_len);
                entry.m_value[copy_len] = '\0';
            }
        }

        static void unescapeString(const char* src, size_t srcLen, char* dst, size_t dstSize)
        {
            size_t di = 0;
            for (size_t si = 0; si < srcLen && di < dstSize - 1; si++) {
                if (src[si] == '\\' && si + 1 < srcLen) {
                    si++;
                    switch (src[si]) {
                    case 'n':
                        dst[di++] = '\n';
                        break;
                    case 't':
                        dst[di++] = '\t';
                        break;
                    case 'r':
                        dst[di++] = '\r';
                        break;
                    case '\\':
                        dst[di++] = '\\';
                        break;
                    case '"':
                        dst[di++] = '"';
                        break;
                    default:
                        dst[di++] = '\\';
                        if (di < dstSize - 1)
                            dst[di++] = src[si];
                        break;
                    }
                } else {
                    dst[di++] = src[si];
                }
            }
            dst[di] = '\0';
        }

        static int64_t parseInteger(const char* str, size_t len)
        {
            if (len == 0)
                return 0;

            int64_t result = 0;
            bool negative = false;
            size_t i = 0;

            if (str[0] == '-') {
                negative = true;
                i++;
            } else if (str[0] == '+') {
                i++;
            }

            if (i + 1 < len && str[i] == '0' && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
                i += 2;
                while (i < len) {
                    char c = str[i++];
                    if (c >= '0' && c <= '9')
                        result = result * 16 + (c - '0');
                    else if (c >= 'a' && c <= 'f')
                        result = result * 16 + (c - 'a' + 10);
                    else if (c >= 'A' && c <= 'F')
                        result = result * 16 + (c - 'A' + 10);
                    else if (c != '_')
                        break;
                }
            } else {
                while (i < len) {
                    char c = str[i++];
                    if (c >= '0' && c <= '9')
                        result = result * 10 + (c - '0');
                    else if (c != '_')
                        break;
                }
            }

            return negative ? -result : result;
        }
    };

} // namespace KtKernel
