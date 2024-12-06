#ifndef __BSKY_API_H_GUARD__
#define __BSKY_API_H_GUARD__
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define BSKY_ARRAY_LEN(array) (sizeof (array) / sizeof (array)[0])


/*
 * module
 * ============================================================================
 *                                   LOG
 * ============================================================================
*/
    enum bsky_log_level {
        bsky_log_None = 0, // not output
        bsky_log_Error,
        bsky_log_Warning,
        bsky_log_Info,
    };

    #ifndef BSKY_LOG_LEVEL
    #define BSKY_LOG_LEVEL bsky_log_Info
    #endif


    /**
     * Logger function used by `bsky_log'. Logger must sutisfy following
     * usage:
     *          BSKY_SIMPLE_LOGGER(fmt, __VA_ARGS__) or
     *          BSKY_LOGGER(level, fmt, __VA_ARGS__)
     *
     * NOTE: Simple logger do not accept level of logging. The `bsky_log's
     *       fmg will be modified to show log level. If you wanna display
     *       log level in custom way --- use `BSKY_LOGGER'
     *       instead.
     */
    #ifndef BSKY_LOGGER
        #ifndef BSKY_SIMPLE_LOGGER
            #define BSKY_SIMPLE_LOGGER(fmt, ...)                 \
                                       printf(fmt, __VA_ARGS__)
        #endif

        #define BSKY_LOGGER(level, fmt, ...) do {                \
                    switch (level) {                                     \
                    case bsky_log_None: break;                           \
                    case bsky_log_Error:                                 \
                        BSKY_SIMPLE_LOGGER("\x1b[31m[ERR]: `" fmt "'\n" \
                                           "\x1b[0m", __VA_ARGS__);      \
                        break;                                           \
                    case bsky_log_Warning:                               \
                        BSKY_SIMPLE_LOGGER("\x1b[33m[WAR]: `" fmt "'\n" \
                                           "\x1b[0m", __VA_ARGS__);      \
                        break;                                           \
                    case bsky_log_Info:                                  \
                        BSKY_SIMPLE_LOGGER("[INF]: `" fmt "'\n",        \
                                          __VA_ARGS__);                  \
                        break;                                           \
                    }                                                    \
                } while(0)                                               \

    #endif


    /**
     * Log formatted message using BSKY_LOGGER, unless logger lesser than
     * `BSKY_LOG_LEVELE'
     */
    #define bsky_log(level, fmt, ...) if (level <= BSKY_LOG_LEVEL) {\
            BSKY_LOGGER(level, fmt, __VA_ARGS__);                           \
        }                                                                   \

/*
 * module
 * ============================================================================
 *                                ERROR CODES
 * ============================================================================
*/
    enum bsky_error_code {
        bsky_ec_Ok = 0, // no error
        bsky_ec_Tmp_overflow,
    };

    /**
     * Get error code information.
     */
	const char *bsky_str_of_error_code(enum bsky_error_code);

    #define bsky_log_error(ec) bsky_log(bsky_log_Error,        \
                    "%s. %s:%d", bsky_str_of_error_code(ec),   \
                    __FILE__, __LINE__);                       \

    #define bsky_return_error(ec) if (ec != bsky_log_Error) {  \
                            bsky_log_error(ec);                \
                            return (ec);                       \
                        }                                      \


/*
 * module:
 * ============================================================================
 *                               TEMPORARY ARENA
 * ============================================================================
 */
    /**
     * 
     * Default tmp arena allocation. To use your own tmp arena
     * define `bsky_tmp_alloc' before including library.
     *
     * Example:
     *      #define bsky_tmp_alloc my_tmp_alloc
     *      #define BSKY_API_IMPLEMENTATION
     *      #include <bsky-api.h>
     * 
     * You can change default tmp arena size by predifining
     * `BSKY_DEFAULT_TMP_ARENA_CAPACITY'.
     *
     * The default arena return NULL, when it's overflow, so
     * the user must reset arena properly. The library itself
     * donot done this.
     *
     * To reset default tmp arena call:
     * bsky_default_tmp_reset();
     *
     * NOTE: if `bsky_tmp_alloc' returns NULL, it will be interpreted as
     *       temporary arena overflow.
     */
    #ifndef bsky_tmp_alloc
        #define bsky_tmp_alloc __bsky_default_tmp_alloc
    #endif
    #ifndef BSKY_DEFAULT_TMP_ARENA_CAPACITY
        #define BSKY_DEFAULT_TMP_ARENA_CAPACITY (0x8 * 0x400 * 0x400)
    #endif


    /**
     * Default tmp arena allocator.
     */
    void *__bsky_default_tmp_alloc(size_t);
    /**
     * Reset default tmp arena.
     */
    void bsky_default_tmp_reset(void);


/*
 * module:
 * ============================================================================
 *                             DYNAMIC ARRAY
 * ============================================================================
*/

    /**
     * Generic interface for dynamic arrays.
     * All dynamic arrays must start with fiels:
     *     0. pointer to the data.       (type: pointer)
     *     1. the length of the array.   (type: size_t)
     *     2. the capacity of the array. (type: size_t)
     *
     * To use generic dynamic array with macroses it must have
     * fields names alike `struct bsky_dynamic_arr'.
     *
     * Functions will work even with different fields names.
     */
    struct bsky_dynamic_arr {
        void *data; size_t len, cap;
    };

    /**
     * Push copy of element to the end of the dynamic array.
     */
    enum bsky_error_code
    __bsky_da_push(void *self_gen, const void *elem, size_t elem_size);

    /**
     * Append array of elements to the dynamic array;
     */
    enum bsky_error_code
    __bsky_da_append(void *self_gen, const void *elems,
                     size_t elem_size, size_t len);

    /**
     * Free dynamic array.
     */
    void   bsky_da_free(void *da);

    /**
     * Push copy of element to the end of the dynamic array.
     */
    #define bsky_da_push(da, elem) __bsky_da_push(da, &(elem), sizeof (elem))

    /**
     * Push copy of element to the end of the dynamic array.
     */
    #define bsky_da_append(da, elem, len) __bsky_da_push(da, &(elem),\
                                                         sizeof (elem), len)


/*
 * module:
 * ============================================================================
 *                                   VIEW
 * ============================================================================
*/
    /**
     * Generic interface for views. (fat pointers with start and end);
     *     0. pointer to the start of the data.              (type: pointer)
     *     1. pointer to the end of the data [not included]. (type: pointer)
     * 
     * To use generic view with macroses it must have
     * fields names alike `struct bsky_view'.
     *
     * Functions will work even with different fields names.
     */
    struct bsky_view { void *start, *end; };

	/**
     * Create view from gneric dynamic array and element size.
     */
    struct bsky_view __bsky_view_of_da(void *da, size_t elem_size);

    /**
     * Copy view to tmp storage.
     */
    struct bsky_view   bsky_view_to_tmp(struct bsky_view);

	/**
     * Copy generic dynamic array to tmp areana, free it, and return view to
     * the data.
     */
    struct bsky_view __bsky_tmp_view_of_da(void *da, size_t elem_size); 

    /**
     * Construct view of dynamic array pointer.
     */
    #define bsky_view_of_da(da)\
          __bsky_view_of_da(da, sizeof da->data[0])

    /**
     * Copy dynamic array pointer to tmp view and free it.
     */
    #define bsky_tmp_view_of_da(da)\
          __bsky_tmp_view_of_da(da,\
              sizeof ((struct bsky_dynamic_arr*)da)->data[0])




/*
 * module:
 * ============================================================================
 *                                   STRING
 * ===========================================================================
*/
    /**
     * Builder for the string.
     * 
     * NOTE: string builder ensures that string always null terminated.
     */
    struct bsky_str_builder { char *data; size_t len, cap; };

    /**
     * Null terminated string. The pointer points to '\0' char.
     */
    struct bsky_str         { char *start; char *end; };

    /**
     * Push char to `string builder'.
     */
    void bsky_sb_push(struct bsky_str_builder *sb, char c);

    /**
     * Push string to string builder.
     *
     * NOTE: even if string is not null terminated, string builder will
     * add null character at the end.
     */
    void bsky_sb_push_str(struct bsky_str_builder *, struct bsky_str);

    /**
     * Push formated string to string builder.
     */
    void bsky_sb_push_fmt(struct bsky_str_builder *, char *, ...);

    /**
     * Build string from string builder.
     *
     * NOTE: string owning data of builder.
     */
    struct bsky_str bsky_sb_build(struct bsky_str_builder *sb);

    /**
     * Build string in tmp arena and free builder.
     */
    struct bsky_str bsky_sb_build_tmp(struct bsky_str_builder *sb);

    /**
     * Construct view of string. String is view by itself, but
     * string not include last character (null character), however
     * returned view will contain null character.
     */
    struct bsky_view bsky_view_of_str(struct bsky_str);

    /**
     * Left trim string. This function donot copy
     * string it's just create subview of the original string.
     */
    struct bsky_str bsky_trim_left(struct bsky_str);

    /**
     * Check if string starts with other string.
     * Retrun 1 if string starts with prefix and 0 otherwise.
     */
    int bsky_str_starts_with(struct bsky_str, struct bsky_str);

    /**
     * Check if string ends with other string.
     * Retrun 1 if string ends with prefix and 0 otherwise.
     */
    int bsky_str_ends_with(struct bsky_str, struct bsky_str);

    /**
     * Check if two strings equal to each other.
     * Return 1 if strings are equal and 0 otherwise.
     */
    int bsky_str_eq(struct bsky_str, struct bsky_str);

    /**
     * Compare two strings.
     * 
     * Return 0 if strings are equal.
     * Return 1+idx if first string bigger than second.
     * Return -1-idx if secodn string bigger than first.
     *
     * idx -- idx of first different character of strings.
     */
    int bsky_str_cmp(struct bsky_str, struct bsky_str);

    /**
     * Return length of the string.
     */
    size_t bsky_str_len(struct bsky_str);



/*
 * module:
 * ===========================================================================
 *                                    JSON
 * ===========================================================================
*/

	// Just forward declaration :)
    struct bsky_json_pair;

    /**
     * `bsky_json' is data structure to represent json format, parse, and
     * serialize it. It can be static created.
     *
     * By default `bsky_json' is not dynamic data structure, cause in primary
     * use cases it should just serve as help format to create JSON strings or
     * read data from JSON format.
     */
    struct bsky_json {
        enum {
            bsky_json_Arr,  // JSON Array
            bsky_json_Dct,  // JSON Dictionary

            bsky_json_Num,  // JSON Number
            bsky_json_Str,  // JSON String
            bsky_json_Bool, // JSON Boolean
            bsky_json_Null, // JSON NULL
        } var;

        union {
            struct { struct bsky_json      *data; size_t len; } arr;
            struct { struct bsky_json_pair *data; size_t len; } dct;
            float num; char *str; int _bool;
        };
    };

	// pair to implement dictionaries.
    struct bsky_json_pair {
        char *name; struct bsky_json value;
    };

    /**
     * Create temporary string of json. The resulting string would be in
     * compressed format.
     */
    struct bsky_str bsky_tmp_str_of_json(struct bsky_json);

    /**
     * Push JSON string to string builder.
     */
    void bsky_sb_push_json(struct bsky_str_builder *, struct bsky_json);

	/**
     * Parse JSON data.
     */
    enum bsky_error_code bsky_json_of_str(struct bsky_str, struct bsky_json *);

    typedef struct bsky_json      bsky_Json;
    typedef struct bsky_json_pair bsky_Json_Pair;




#ifdef BSKY_API_IMPLEMENTATION

	/*
     * BSKY ERROR CODE
     */
    const char *bsky_str_of_error_code(enum bsky_error_code code)
    {
        switch (code) {
        case bsky_ec_Ok:           return "Ok";
        case bsky_ec_Tmp_overflow: return "overflow of temporary arena!";
        }
    }

	/*
     * DEFAULT TMP ARENA
     */
    struct __bsky_default_tmp_arena {
        void *allocator; size_t len;
    } __bsky_default_tmp_arena = { 0 };

    void    bsky_default_tmp_reset(void) {
        __bsky_default_tmp_arena.len = 0;
    }

    void *__bsky_default_tmp_alloc(size_t size_to_alloc)
    {
        if (__bsky_default_tmp_arena.allocator == 0) {
            __bsky_default_tmp_arena.allocator =
                calloc(BSKY_DEFAULT_TMP_ARENA_CAPACITY, sizeof(char));
        }

        if (__bsky_default_tmp_arena.len + size_to_alloc >
            BSKY_DEFAULT_TMP_ARENA_CAPACITY) 
            return NULL;

        void *ret = __bsky_default_tmp_arena.allocator
                    + __bsky_default_tmp_arena.len;

        __bsky_default_tmp_arena.len += size_to_alloc;

        return ret;
    }

    /*
     * BKSY DYNAMIC ARRAY
     */
	#include <string.h>
    void bsky_da_free(void *da) {
        free(((struct bsky_dynamic_arr*)(da))->data);
    }

    enum bsky_error_code 
    __bsky_da_push(void *self_gen, const void *elem, size_t elem_size)
    {
        struct bsky_dynamic_arr *self = (struct bsky_dynamic_arr*) self_gen;

        if (self->len >= self->cap) {
            self->cap  = self->cap ?  self->cap * 2 : 16;
            self->data = realloc(self->data, self->cap * elem_size);
            if (self->data == NULL) bsky_return_error(bsky_ec_Tmp_overflow);
        }

        memcpy(self->data + self->len++, elem, elem_size);

        return bsky_ec_Ok;
    }

    enum bsky_error_code
    __bsky_da_append(void *self_gen, const void *elems,
                     size_t elem_size, size_t len)
    {
        struct bsky_dynamic_arr *self = (struct bsky_dynamic_arr*) self_gen;

        if (self->len + len > self->cap) {
            self->cap  = (self->cap ? self->cap * 2 : 16) + len;
            self->data = realloc(self->data, self->cap * elem_size);

            if (self->data == NULL) bsky_return_error(bsky_ec_Tmp_overflow);
        }

        memcpy(self->data + self->len, elems, elem_size * len);
        self->len += len;

        return bsky_ec_Ok;
    }

    /*
     * BSKY VIEW
     */
    struct bsky_view __bsky_view_of_da(void *_da, size_t elem_size)
    {
        struct bsky_dynamic_arr *da = (struct bsky_dynamic_arr *) _da;

        return (struct bsky_view) { da->data, da->data + da->len * elem_size };
    }

    struct bsky_view bsky_view_to_tmp(struct bsky_view view)
    {
        void *data = bsky_tmp_alloc(view.end - view.start);
        memcpy(data, view.start, view.end - view.start);

        return (struct bsky_view) { data, (char*)data + 
                        (view.end - view.start) / sizeof(char)
        };
    }

    struct bsky_view __bsky_tmp_view_of_da(void *da, size_t elem_size)
    {
        struct bsky_view ret = bsky_view_to_tmp(
                                 __bsky_view_of_da(da, elem_size));

        bsky_da_free(da);

        return ret;
    }

    /*
     * BSKY STRING
     */
    void bsky_sb_push(struct bsky_str_builder *sb, char c)
    {
        const char nullc = '\0';

        if (sb->len == 0) {
            char buf[] = { c, nullc };
            __bsky_da_append(sb, buf, sizeof(buf), 2);
        }
        else {
            sb->data[sb->len-1] = c;
            bsky_da_push(sb, nullc);
        }
    }

    void bsky_sb_push_str(struct bsky_str_builder *sb, struct bsky_str str)
    {
        const char nullc = '\0';

        if (sb->len != 0) sb->len--; // remove null character.

        __bsky_da_append(sb, str.start, sizeof(char), str.end - str.start);

        if (sb->data[sb->len-1] != '\0') bsky_da_push(sb, nullc);
    }

    #include <stdarg.h>

    void bsky_sb_push_fmt(struct bsky_str_builder *sb, char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(NULL, 0, fmt, args);
        va_end(args);

        char *result = bsky_tmp_alloc(len + 1);
        va_start(args, fmt);
        vsnprintf(result, len + 1, fmt, args);
        va_end(args);

        bsky_sb_push_str(sb, (struct bsky_str) {result, result + len + 1});
    }

    struct bsky_str bsky_sb_build(struct bsky_str_builder *sb)
    {
        return (struct bsky_str) { sb->data, sb->data + sb->len-1 };
    }

    struct bsky_str bsky_sb_build_tmp(struct bsky_str_builder *sb)
    {
        struct bsky_view str = bsky_view_of_str(bsky_sb_build(sb));
        struct bsky_view tmp = bsky_view_to_tmp(str);

        bsky_da_free(sb);
        *sb = (struct bsky_str_builder) { 0 };

        return (struct bsky_str) { tmp.start, tmp.end-1 };
    }

    struct bsky_view bsky_view_of_str(struct bsky_str str)
    {
        return (struct bsky_view) { str.start, str.end + 1 };
    }

    static int __s_is_whitespace(char c) {
        return c == ' ' || c == '\n' || c == '\t';
    }

    struct bsky_str bsky_trim_left(struct bsky_str str)
    {
        while (str.start != str.end && __s_is_whitespace(*str.start)) {
            str.start++;
        }

        return str;
    }

    int bsky_str_starts_with(struct bsky_str fst, struct bsky_str pref)
    {
        size_t fst_len = bsky_str_len(fst), pref_len = bsky_str_len(pref);

        if (fst_len < pref_len) return 0;

        return memcmp(fst.start, pref.start, pref_len) == 0;
    }

    int bsky_str_ends_with(struct bsky_str fst, struct bsky_str post)
    {
        size_t fst_len = bsky_str_len(fst), post_len = bsky_str_len(post);

        if (fst_len < post_len) return 0;

        return memcmp(fst.start + fst_len - post_len,
                      post.start, post_len) == 0;
    }

    size_t bsky_str_len(struct bsky_str str) {
        return (str.end - str.start) / sizeof (char);
    }

    int bsky_str_eq(struct bsky_str fst, struct bsky_str snd) {
        return strcmp(fst.start, snd.start) == 0;
    }

    int bsky_str_cmp(struct bsky_str fst, struct bsky_str snd) {
        size_t fst_len = bsky_str_len(fst), snd_len = bsky_str_len(snd);

        for (int i = 0; i < (fst_len > snd_len ? fst_len : snd_len); ++i) {
            if (fst.start[i] > snd.start[i]) return  1 + i;
            if (fst.start[i] < snd.start[i]) return -1 - i;
        }

        return fst_len > snd_len ?   1 + snd_len :
               snd_len > fst_len ? - 1 - fst_len :
               0;
    }


    /*
     * BSKY JSON
     */
    void bsky_sb_push_json(struct bsky_str_builder *sb, struct bsky_json json)
    {
        switch (json.var) {
        case bsky_json_Arr: {
            bsky_sb_push_fmt(sb, "[");

            for (size_t i = 0; i < json.arr.len; ++i) {
                bsky_sb_push_json(sb, json.arr.data[i]);

                if (i != json.arr.len-1) bsky_sb_push_fmt(sb, ",");
                else                     bsky_sb_push_fmt(sb, "]");
            }
        } break;
        case bsky_json_Dct: {
            bsky_sb_push_fmt(sb, "{");

            for (size_t i = 0; i < json.dct.len; ++i) {
                bsky_sb_push_fmt(sb, "\"%s\":", json.dct.data[i].name);
                bsky_sb_push_json(sb, json.dct.data[i].value);

                if (i != json.dct.len-1) bsky_sb_push_fmt(sb, ",");
                else                     bsky_sb_push_fmt(sb, "}");
            }

        } break;
        case bsky_json_Num: {
            if (fabsf(json.num - (float)(int)json.num) < 0.0001) {
                bsky_sb_push_fmt(sb, "%d", (int)json.num);
            } else {
                bsky_sb_push_fmt(sb, "%f.3",    json.num);
            }
        }break;
        case bsky_json_Str:  bsky_sb_push_fmt(sb, "\"%s\"", json.str); break;
        case bsky_json_Null: bsky_sb_push_fmt(sb, "null"); break;
        case bsky_json_Bool: {
            bsky_sb_push_fmt(sb, "%s", json._bool ? "true" : "false"); 
        }break;
        }
    }

    struct bsky_str bsky_tmp_str_of_json(struct bsky_json json)
    {
        struct bsky_str_builder sb = { 0 };

        bsky_sb_push_json(&sb, json);

        return bsky_sb_build_tmp(&sb);
    }

    enum bsky_error_code
    bsky_json_of_str(struct bsky_str str, struct bsky_json *result)
    {
		return bsky_ec_Ok;
    }


#endif

/**
 * If defined `BSKY_API_STRIP_PREFIX' will strip `bsky_' prefix from functions.
 */
#ifdef BSKY_API_STRIP_PREFIX
    /*
     * BSKY TMP ARENA
     */
    #define tmp_alloc(size) bsky_tmp_alloc(size)
    #define default_tmp_reset() bsky_default_tmp_reset()

    /*
     * BSKY DYNAMIC ARRAY
     */
    #define da_free(da) bsky_da_free(da)
    #define __da_push(da, elem_size) __bsky_da_push(da, elem_size)
    #define da_push(da) bsky_da_push(da)

    /*
     * BSKY 
     */
    #define __view_of_da(da, elem_size) __bsky_view_of_da(da, elem_size)
    #define view_of_da(da) bsky_view_of_da(da)
    #define view_to_tmp(view) bsky_view_to_tmp(view)
    #define __tmp_view_of_da(da, elem_size) __bsky_tmp_view_of_da(da,elem_size)
    #define tmp_view_of_da(da) bsky_tmp_view_of_da(da)
#endif



#endif //GUARD
