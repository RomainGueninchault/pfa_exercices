#include <stdio.h>
#include <printf.h>
#include "grader2.h"

#define MAX_STRUCT_RENDERERS    10
struct struct_renderer struct_renderers[MAX_STRUCT_RENDERERS] = {};
#define MAX_ENUM_RENDERERS      10
enum_renderer enum_renderers[MAX_ENUM_RENDERERS] = {};

#ifdef __APPLE__
printf_domain_t domain = NULL;

static int register_printf_specifier(int spec, printf_function *render, printf_arginfo_function *arginfo)
{
    if (!domain)
        domain = new_printf_domain();
    if (domain && register_printf_domain_function(domain, spec, render, arginfo, NULL)) {
        return (domain != NULL) + 1;
    }
    return 0;
}

static int printf_array_info(const struct printf_info *info, size_t n, int argtypes[])
{
    if (n > 1) {
        argtypes[0] = PA_INT /*| PA_FLAG_SIZE*/;
        argtypes[1] = PA_POINTER;
    }
    return 2;
}
#else
static int printf_array_info(const struct printf_info *info, size_t n, int argtypes[], int sizes[])
{
    (void) info;
    if (n > 1) {
        argtypes[0] = PA_INT /*| PA_FLAG_SIZE*/;
        sizes[0] = sizeof(int);
        argtypes[1] = PA_POINTER;
        sizes[1] = sizeof(void*);
    }
    return 2;
}
#endif

static int print_array(FILE *stream, const struct printf_info *info,
        void const* const* args, size_t item_size, printf_renderer render, const void *context)
{
    int len = 0;
    int values_count = *(int*)(args[0]); /* (size_t*) */
    void const* const* value = args[1];

    if (value == NULL) // useles ?
        return fputs("NULL", stream);
    if (!info->left)
        len += fputs("{", stream);
    for (int i = 0; i < values_count; i ++) {
        if (i > 0) {
            if (info->group)
                len += fputs(",\n\t", stream);
            else
                len += fputs(", ", stream);
        }
        if (*value == NULL)
            len += fputs("NULL", stream);
        else
            len += render(stream, *value, context);
        (*(char**)value) += item_size;
    }
    if (!info->left)
        len += fputs("}", stream);
    return len;
}

static int simple_short_printf_renderer(FILE *stream, const void* data, const char *fmt)
{
    return fprintf(stream, fmt, *(const short*)data);
}
static int simple_char_printf_renderer(FILE *stream, const void* data, const char *fmt)
{
    return fprintf(stream, fmt, *(const short*)data);
}
static int simple_int_printf_renderer(FILE *stream, const void* data, const char *fmt)
{
    return fprintf(stream, fmt, *(const int*)data);
}
static int simple_longlong_printf_renderer(FILE *stream, const void* data, const char *fmt)
{
    return fprintf(stream, fmt, *(const long long*)data);
}

#define simple_int_or_longlong_printf(__type)  \
    ((sizeof(int) == sizeof(__type))           \
     ? simple_int_printf_renderer              \
     : simple_longlong_printf_renderer)

enum int_types {
    INT, SHORT, CHAR, LONG, LONGLONG, SIZE_T, INT_TYPES
};
enum int_convertions {
    HEX, UNSIGNED, OCTAL, DECIMAL, INT_CONVERSIONS
};

struct int_formats {
    size_t size;
    int (*printf_renderer)(FILE *stream, const void* data, const char *fmt);
    const char *formats[INT_CONVERSIONS];
};

static const struct int_formats formats[INT_TYPES] = {
    { sizeof(int), simple_int_printf_renderer,
        { "%#x", "%u", "%o", "%d" }},
    { sizeof(short), simple_short_printf_renderer,
        { "%#hx", "%hu", "%ho", "%hd" }},
    { sizeof(char), simple_char_printf_renderer,
        { "%#hhx", "%hhu", "%hho", "%hhd" }},
    { sizeof(long), simple_int_or_longlong_printf(long),
        { "%#lx", "%lu", "%lo", "%ld" }},
    { sizeof(long long), simple_longlong_printf_renderer,
        { "%#llx", "%llu", "%llo", "%lld" }},
    { sizeof(size_t), simple_int_or_longlong_printf(size_t),
        { "%#zx", "%zu", "%zo", "%zd" }},
};

static printf_renderer number_renderer(const struct printf_info *info, size_t *item_size, const char **fmt)
{
    const struct int_formats *format = formats + INT;
    if (info->is_char) {
        format = formats + CHAR;
    } else if (info->is_short) {
        format = formats + SHORT;
    } else if (info->is_long) {
        format = formats + LONG;
    } else if (info->is_long_double) {
        format = formats + LONGLONG;
#ifdef __APPLE__
    } else if (info->is_size) {
        format = formats + SIZE_T;
#endif
    }

    *item_size = format->size;
    if (info->alt)
        *fmt = format->formats[HEX];
    else if (info->showsign)
        *fmt = format->formats[UNSIGNED];
    else if (info->pad == '0') // FIXME this is a bad symbol
        *fmt = format->formats[OCTAL];
    else
        *fmt = format->formats[DECIMAL];

    return (printf_renderer)format->printf_renderer;
}

static int printf_number(FILE *stream, const struct printf_info *info, void const* const* data)
{
    size_t item_size;
    const char *fmt;
    printf_renderer renderer = number_renderer(info, &item_size, &fmt);
    return print_array(stream, info, data, item_size, renderer, fmt);
}

static int index_of(char element, const char *str)
{
    int i = 0;
    while (str[i]) {
        if (str[i] == element)
            return i;
        i ++;
    }
    return -1;
}

static char special_chars[] = "\"'\n\t";
static char special_replacements[] = "\"'nt";

int print_escaped_string(FILE *stream, const char** data, const void *unused)
{
    (void)unused;
    const char *str = *data;
    int printed = 2;
    fputc('\"', stream);
    while (*str) {
        int idx = index_of(*str, special_chars);
        if (idx != -1) {
            fputc('\\', stream);
            fputc(special_replacements[idx], stream);
            printed += 2;
        } else if (*str < 32) {
            fputc('\\', stream);
            fputc('0', stream);
            fputc('0'+ *str/10, stream);
            fputc('0'+ *str%10, stream);
            printed += 4;
        } else {
            fputc(*str, stream);
            printed++;
        }
        str ++;
    }
    fputc('\"', stream);
    return printed;
}

static printf_renderer ptr_renderer(const struct printf_info *info, const char **fmt)
{
    if (info->spec == 'P') {
        *fmt = "%p";
    } else {
        if (info->alt)
            return (printf_renderer)print_escaped_string;
        *fmt = "%s";
    }
    return (printf_renderer)simple_int_or_longlong_printf(void*);
}

static int printf_string(FILE *stream, const struct printf_info *info, void const* const* data)
{
    const char *fmt;
    printf_renderer renderer = ptr_renderer(info, &fmt);
    return print_array(stream, info, data, sizeof(void*), renderer, fmt);
}

static int printf_struct(FILE *stream, const struct printf_info *info, void const* const* data)
{
    int idx = info->width;
    if (idx < 0 || idx >= MAX_STRUCT_RENDERERS)
        return fprintf(stream, "NoRendererSpecified<%d, %p>", *(int*)data[0], data[1]);
    if (struct_renderers[idx].item_size == 0)
        return fprintf(stream, "UnknownRenderer:%d<%d, %p>", idx, *(int*)data[0], data[1]);
    struct struct_renderer renderer = struct_renderers[idx];
    return print_array(stream, info, data, renderer.item_size, renderer.render, NULL);
}

PROVIDED_FUNCTION(void, initialize_struct_renderers)
{}

static int puts_enum(FILE* stream, const void* item, const void *name_of)
{
    return fputs(((enum_renderer)name_of)(*(int*)item), stream);
}

static int printf_enum(FILE *stream, const struct printf_info *info, void const* const* data)
{
    int idx = info->width;
    if (idx < 0 || idx >= MAX_ENUM_RENDERERS)
        return fprintf(stream, "NoRendererSpecified<%d, %p>", *(int*)data[0], data[1]);
    if (!enum_renderers[idx])
        return fprintf(stream, "UnknownRenderer:%d<%d, %p>", idx, *(int*)data[0], data[1]);
    return print_array(stream, info, data, sizeof(int), puts_enum, enum_renderers[idx]);
}

PROVIDED_FUNCTION(void, initialize_enum_renderers)
{}

int register_printf_handlers()
{
    if (
            register_printf_specifier('I', printf_number, printf_array_info) ||
            register_printf_specifier('D', printf_number, printf_array_info) ||
            register_printf_specifier('P', printf_string, printf_array_info) ||
            register_printf_specifier('S', printf_string, printf_array_info)  ||
            // register_printf_specifier('F', printf_print_array_double, printf_array_info) ||
            register_printf_specifier('@', printf_struct, printf_array_info) ||
            register_printf_specifier('[', printf_enum, printf_array_info) ||
            0
       ) {
        perror("Registering printf conversions");
        return 1;
    }
    initialize_struct_renderers();
    initialize_enum_renderers();
    return 0;
}
