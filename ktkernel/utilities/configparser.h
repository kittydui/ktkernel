#pragma once
#include "mem/string.h"
#include "mem/vector.h"
#include <cstdint>

namespace kt_kernel
{

    class toml_parser
    {
    public:
        enum class value_type : uint8_t
        {
            string_val,
            bool_val,
            int_val
        };

        struct entry
        {
            char section[64];
            char key[64];
            char value[256];
            value_type type;
            bool bool_val;
            int64_t int_val;
        };

        bool parse(const char* data, size_t length)
        {
            entries.clear();
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

                size_t i = skip_whitespace(data, line_start, line_end);
                if (i >= line_end || data[i] == '#')
                    continue;

                if (data[i] == '[') {
                    parse_section(data, i + 1, line_end, current_section, sizeof(current_section));
                    continue;
                }

                size_t eq_pos = i;
                while (eq_pos < line_end && data[eq_pos] != '=')
                    eq_pos++;
                if (eq_pos >= line_end)
                    continue;

                entry e = {};
                extract_key(data, i, eq_pos, e.key, sizeof(e.key));
                strncpy(e.section, current_section, sizeof(e.section) - 1);

                size_t val_start = skip_whitespace(data, eq_pos + 1, line_end);
                size_t val_end = trim_trailing(data, val_start, line_end);
                parse_value(data, val_start, val_end, e);

                entries.push_back(e);
            }

            return true;
        }

        const char* get_string(const char* section, const char* key, const char* default_val = "") const
        {
            const entry* e = find(section, key);
            if (!e)
                return default_val;
            return e->value;
        }

        bool get_bool(const char* section, const char* key, bool default_val = false) const
        {
            const entry* e = find(section, key);
            if (!e || e->type != value_type::bool_val)
                return default_val;
            return e->bool_val;
        }

        int64_t get_int(const char* section, const char* key, int64_t default_val = 0) const
        {
            const entry* e = find(section, key);
            if (!e || e->type != value_type::int_val)
                return default_val;
            return e->int_val;
        }

        bool has_key(const char* section, const char* key) const
        {
            return find(section, key) != nullptr;
        }

        bool has_section(const char* section) const
        {
            for (size_t i = 0; i < entries.size(); i++) {
                if (strcmp(entries[i].section, section) == 0)
                    return true;
            }
            return false;
        }

        size_t entry_count() const
        {
            return entries.size();
        }

        const entry& get_entry(size_t idx) const
        {
            return entries[idx];
        }

    private:
        vector<entry> entries;

        const entry* find(const char* section, const char* key) const
        {
            for (size_t i = 0; i < entries.size(); i++) {
                if (strcmp(entries[i].section, section) == 0 && strcmp(entries[i].key, key) == 0)
                    return &entries[i];
            }
            return nullptr;
        }

        static size_t skip_whitespace(const char* data, size_t from, size_t to)
        {
            while (from < to && (data[from] == ' ' || data[from] == '\t'))
                from++;
            return from;
        }

        static size_t trim_trailing(const char* data, size_t from, size_t to)
        {
            while (to > from && (data[to - 1] == ' ' || data[to - 1] == '\t'))
                to--;
            return to;
        }

        static void parse_section(const char* data, size_t start, size_t end, char* out, size_t out_size)
        {
            size_t close = start;
            while (close < end && data[close] != ']')
                close++;

            size_t s = skip_whitespace(data, start, close);
            size_t e = trim_trailing(data, s, close);
            size_t len = e - s;
            if (len >= out_size)
                len = out_size - 1;
            strncpy(out, data + s, len);
            out[len] = '\0';
        }

        static void extract_key(const char* data, size_t start, size_t eq_pos, char* out, size_t out_size)
        {
            size_t end = trim_trailing(data, start, eq_pos);
            size_t len = end - start;
            if (len >= out_size)
                len = out_size - 1;
            strncpy(out, data + start, len);
            out[len] = '\0';
        }

        static void parse_value(const char* data, size_t start, size_t end, entry& e)
        {
            size_t len = end - start;

            if (len >= 2 && data[start] == '"' && data[end - 1] == '"') {
                e.type = value_type::string_val;
                size_t s_len = len - 2;
                if (s_len >= sizeof(e.value))
                    s_len = sizeof(e.value) - 1;
                unescape_string(data + start + 1, s_len, e.value, sizeof(e.value));
            } else if (len == 4 && strncmp(data + start, "true", 4) == 0) {
                e.type = value_type::bool_val;
                e.bool_val = true;
                strncpy(e.value, "true", sizeof(e.value));
            } else if (len == 5 && strncmp(data + start, "false", 5) == 0) {
                e.type = value_type::bool_val;
                e.bool_val = false;
                strncpy(e.value, "false", sizeof(e.value));
            } else {
                e.type = value_type::int_val;
                e.int_val = parse_integer(data + start, len);
                size_t copy_len = len < sizeof(e.value) - 1 ? len : sizeof(e.value) - 1;
                strncpy(e.value, data + start, copy_len);
                e.value[copy_len] = '\0';
            }
        }

        static void unescape_string(const char* src, size_t src_len, char* dst, size_t dst_size)
        {
            size_t di = 0;
            for (size_t si = 0; si < src_len && di < dst_size - 1; si++) {
                if (src[si] == '\\' && si + 1 < src_len) {
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
                        if (di < dst_size - 1)
                            dst[di++] = src[si];
                        break;
                    }
                } else {
                    dst[di++] = src[si];
                }
            }
            dst[di] = '\0';
        }

        static int64_t parse_integer(const char* str, size_t len)
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

} // namespace kt_kernel
