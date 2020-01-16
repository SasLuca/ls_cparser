//region interface
#ifndef LS_CPARSER_H
#define LS_CPARSER_H

#define lscp_int signed long long
#define lscp_bool int

#ifdef __cplusplus
    #ifndef LSCP_API
        #define LSCP_API extern "C"
    #endif//LS_CPARSER_API
#else
    #ifndef LSCP_API
        #define LSCP_API extern
    #endif//LS_CPARSER_API
#endif

#ifndef LSCP_INTERNAL
    #define LSCP_INTERNAL static
#endif//LS_CPARSER_INTERNAL

//region structs

typedef enum lscp_error
{
    lscp_no_error = 0,
    lscp_unexpected_end_of_source,
    lscp_newline_in_string_literal,
    lscp_string_literal_not_closed,
    lscp_unexpected_token,
} lscp_error;

typedef struct lscp_string lscp_string;
struct lscp_string
{
    const char* chars;
    lscp_int    len;
};

typedef enum lscp_token_type
{
    lscp_tk_unknown = 0,

    //region keywords

    lscp_tk_auto_keyword,
    lscp_tk_break_keyword,
    lscp_tk_case_keyword,
    lscp_tk_char_keyword,
    lscp_tk_const_keyword,
    lscp_tk_continue_keyword,
    lscp_tk_default_keyword,
    lscp_tk_do_keyword,
    lscp_tk_double_keyword,
    lscp_tk_else_keyword,
    lscp_tk_enum_keyword,
    lscp_tk_extern_keyword,
    lscp_tk_float_keyword,
    lscp_tk_for_keyword,
    lscp_tk_goto_keyword,
    lscp_tk_if_keyword,
    lscp_tk_inline_keyword,
    lscp_tk_int_keyword,
    lscp_tk_long_keyword,
    lscp_tk_register_keyword,
    lscp_tk_restrict_keyword,
    lscp_tk_return_keyword,
    lscp_tk_short_keyword,
    lscp_tk_signed_keyword,
    lscp_tk_sizeof_keyword,
    lscp_tk_static_keyword,
    lscp_tk_struct_keyword,
    lscp_tk_switch_keyword,
    lscp_tk_typedef_keyword,
    lscp_tk_union_keyword,
    lscp_tk_unsigned_keyword,
    lscp_tk_void_keyword,
    lscp_tk_volatile_keyword,
    lscp_tk_while_keyword,
    lscp_tk_alignas_keyword,
    lscp_tk_alignof_keyword,
    lscp_tk_atomic_keyword,
    lscp_tk_bool_keyword,
    lscp_tk_complex_keyword,
    lscp_tk_generic_keyword,
    lscp_tk_imaginary_keyword,
    lscp_tk_noreturn_keyword,
    lscp_tk_static_assert_keyword,
    lscp_tk_thread_local_keyword,

    //endregion

    lscp_tk_identifier,
    lscp_tk_string_literal,
    lscp_tk_int_literal,
    lscp_tk_float_literal,
    lscp_tk_double_literal,
    lscp_tk_single_line_comment,
    lscp_tk_multi_line_comment,

    lscp_tk_backtick,              // `
    lscp_tk_tilde,                 // ~
    lscp_tk_exclamation_mark,      // !
    lscp_tk_percent,               // %
    lscp_tk_hat,                   // ^
    lscp_tk_ampersand,             // &
    lscp_tk_star,                  // *
    lscp_tk_open_paren,            // (
    lscp_tk_close_paren,           // )
    lscp_tk_minus,                 // -
    lscp_tk_plus,                  // +
    lscp_tk_equal,                 // =
    lscp_tk_colon,                 // :
    lscp_tk_left_arrow,            // <
    lscp_tk_right_arrow,           // >
    lscp_tk_comma,                 // ,
    lscp_tk_dot,                   // .
    lscp_tk_open_square_bracket,   // [
    lscp_tk_close_square_bracket,  // ]
    lscp_tk_open_curly,            // {
    lscp_tk_close_curly,           // }
    lscp_tk_bar,                   // |
    lscp_tk_semicolon,             // ;
    lscp_tk_forward_slash,         // /
    lscp_tk_backward_slash,        // \

} lscp_token_type;

typedef struct lscp_token lscp_token;
struct lscp_token
{
    lscp_token_type type;
    lscp_int        text_begin;
    lscp_int        text_length;
    lscp_int        line_of_code;
    lscp_int        number_in_line; //The number of the token in the line its on.
};

typedef struct lscp_tokenizer_context lscp_tokenizer_context;
struct lscp_tokenizer_context
{
    const char* source_code;
    lscp_int    source_code_len;

    lscp_int    current_pos; //Current position in the source code
    lscp_int    current_line;
    lscp_int    tokens_on_line_count;
    lscp_int    ignored_utf8_bytes_count;

    lscp_error  error;
};

typedef struct lscp_preprocessor_context lscp_preprocessor_context;
struct lscp_preprocessor_context
{

};
//endregion

LSCP_API lscp_bool lscp_tokenizer_is_done(lscp_tokenizer_context tk_ctx);
LSCP_API lscp_token lscp_get_next_token(lscp_tokenizer_context* tk_ctx);

#endif //LS_CPARSER_H
//endregion

//region implementation
#if defined(LS_CPARSER_IMPL) && !defined(LS_CPARSER_IMPL_INCLUDE_GUARD)
#define LS_CPARSER_IMPL_INCLUDE_GUARD

#define LSCP_INDEX_NOT_FOUND (-1)
#define _lscp_str_literal_len(str_lit) (sizeof(str_lit) - 1)

LSCP_INTERNAL inline lscp_bool _lscp_is_ascii_digit(const char it)
{
    return it >= '0' && it <= '9';
}

LSCP_INTERNAL inline lscp_bool _lscp_is_ascii_letter(const char it)
{
    return (it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z');
}

LSCP_INTERNAL inline lscp_int _lscp_zstring_len(const char* zstr)
{
    const char* iter = zstr;
    while (*iter++) {}
    return (lscp_int) (iter - zstr);
}

LSCP_INTERNAL lscp_bool _lscp_strings_match(const char* a, const lscp_int a_len, const char* b, const lscp_int b_len)
{
    if (a_len != b_len) return 0;

    const char* const a_end  = a + a_len;
    const char* const b_end  = b + b_len;

    while (a != a_end && b != b_end)
    {
        if (*a != *b) return 0;
        a++;
        b++;
    }

    return 1;
}

LSCP_INTERNAL lscp_token_type _lscp_get_token_type_if_string_is_keyword(const char* str, const lscp_int str_len)
{
    if (_lscp_strings_match(str, str_len, "auto", _lscp_str_literal_len("auto"))) return lscp_tk_auto_keyword;
    if (_lscp_strings_match(str, str_len, "break", _lscp_str_literal_len("break"))) return lscp_tk_break_keyword;
    if (_lscp_strings_match(str, str_len, "case", _lscp_str_literal_len("case"))) return lscp_tk_case_keyword;
    if (_lscp_strings_match(str, str_len, "char", _lscp_str_literal_len("char"))) return lscp_tk_char_keyword;
    if (_lscp_strings_match(str, str_len, "const", _lscp_str_literal_len("const"))) return lscp_tk_const_keyword;
    if (_lscp_strings_match(str, str_len, "continue", _lscp_str_literal_len("continue"))) return lscp_tk_continue_keyword;
    if (_lscp_strings_match(str, str_len, "default", _lscp_str_literal_len("default"))) return lscp_tk_default_keyword;
    if (_lscp_strings_match(str, str_len, "do", _lscp_str_literal_len("do"))) return lscp_tk_do_keyword;
    if (_lscp_strings_match(str, str_len, "double", _lscp_str_literal_len("double"))) return lscp_tk_double_keyword;
    if (_lscp_strings_match(str, str_len, "else", _lscp_str_literal_len("else"))) return lscp_tk_else_keyword;
    if (_lscp_strings_match(str, str_len, "enum", _lscp_str_literal_len("enum"))) return lscp_tk_enum_keyword;
    if (_lscp_strings_match(str, str_len, "extern", _lscp_str_literal_len("extern"))) return lscp_tk_extern_keyword;
    if (_lscp_strings_match(str, str_len, "float", _lscp_str_literal_len("float"))) return lscp_tk_float_keyword;
    if (_lscp_strings_match(str, str_len, "for", _lscp_str_literal_len("for"))) return lscp_tk_for_keyword;
    if (_lscp_strings_match(str, str_len, "goto", _lscp_str_literal_len("goto"))) return lscp_tk_goto_keyword;
    if (_lscp_strings_match(str, str_len, "if", _lscp_str_literal_len("if"))) return lscp_tk_if_keyword;
    if (_lscp_strings_match(str, str_len, "inline", _lscp_str_literal_len("inline"))) return lscp_tk_inline_keyword;
    if (_lscp_strings_match(str, str_len, "int", _lscp_str_literal_len("int"))) return lscp_tk_int_keyword;
    if (_lscp_strings_match(str, str_len, "long", _lscp_str_literal_len("long"))) return lscp_tk_long_keyword;
    if (_lscp_strings_match(str, str_len, "register", _lscp_str_literal_len("register"))) return lscp_tk_register_keyword;
    if (_lscp_strings_match(str, str_len, "restrict", _lscp_str_literal_len("restrict"))) return lscp_tk_restrict_keyword;
    if (_lscp_strings_match(str, str_len, "return", _lscp_str_literal_len("return"))) return lscp_tk_return_keyword;
    if (_lscp_strings_match(str, str_len, "short", _lscp_str_literal_len("short"))) return lscp_tk_short_keyword;
    if (_lscp_strings_match(str, str_len, "signed", _lscp_str_literal_len("signed"))) return lscp_tk_signed_keyword;
    if (_lscp_strings_match(str, str_len, "sizeof", _lscp_str_literal_len("sizeof"))) return lscp_tk_sizeof_keyword;
    if (_lscp_strings_match(str, str_len, "static", _lscp_str_literal_len("static"))) return lscp_tk_static_keyword;
    if (_lscp_strings_match(str, str_len, "struct", _lscp_str_literal_len("struct"))) return lscp_tk_struct_keyword;
    if (_lscp_strings_match(str, str_len, "switch", _lscp_str_literal_len("switch"))) return lscp_tk_switch_keyword;
    if (_lscp_strings_match(str, str_len, "typedef", _lscp_str_literal_len("typedef"))) return lscp_tk_typedef_keyword;
    if (_lscp_strings_match(str, str_len, "union", _lscp_str_literal_len("union"))) return lscp_tk_union_keyword;
    if (_lscp_strings_match(str, str_len, "unsigned", _lscp_str_literal_len("unsigned"))) return lscp_tk_unsigned_keyword;
    if (_lscp_strings_match(str, str_len, "void", _lscp_str_literal_len("void"))) return lscp_tk_void_keyword;
    if (_lscp_strings_match(str, str_len, "volatile", _lscp_str_literal_len("volatile"))) return lscp_tk_volatile_keyword;
    if (_lscp_strings_match(str, str_len, "while", _lscp_str_literal_len("while"))) return lscp_tk_while_keyword;
    if (_lscp_strings_match(str, str_len, "_Alignas", _lscp_str_literal_len("_Alignas"))) return lscp_tk_alignas_keyword;
    if (_lscp_strings_match(str, str_len, "_Alignof", _lscp_str_literal_len("_Alignof"))) return lscp_tk_alignof_keyword;
    if (_lscp_strings_match(str, str_len, "_Atomic", _lscp_str_literal_len("_Atomic"))) return lscp_tk_atomic_keyword;
    if (_lscp_strings_match(str, str_len, "_Bool", _lscp_str_literal_len("_Bool"))) return lscp_tk_bool_keyword;
    if (_lscp_strings_match(str, str_len, "_Complex", _lscp_str_literal_len("_Complex"))) return lscp_tk_complex_keyword;
    if (_lscp_strings_match(str, str_len, "_Generic", _lscp_str_literal_len("_Generic"))) return lscp_tk_generic_keyword;
    if (_lscp_strings_match(str, str_len, "_Imaginary", _lscp_str_literal_len("_Imaginary"))) return lscp_tk_imaginary_keyword;
    if (_lscp_strings_match(str, str_len, "_Noreturn", _lscp_str_literal_len("_Noreturn"))) return lscp_tk_noreturn_keyword;
    if (_lscp_strings_match(str, str_len, "_Static_assert", _lscp_str_literal_len("_Static_assert"))) return lscp_tk_static_assert_keyword;
    if (_lscp_strings_match(str, str_len, "_Thread_local", _lscp_str_literal_len("_Thread_local"))) return lscp_tk_thread_local_keyword;

    return lscp_tk_unknown;
}

//Returns the position of the char
LSCP_INTERNAL lscp_int tokenizer_find_char(const lscp_tokenizer_context* tk_ctx, const char until_c)
{
    for (lscp_int i = tk_ctx->current_pos; i < tk_ctx->source_code_len; i++)
    {
        const char current_char = tk_ctx->source_code[i];
        if (current_char == until_c)
        {
            return i;
        }
    }

    return tk_ctx->source_code_len;
}

LSCP_API lscp_bool lscp_tokenizer_is_done(const lscp_tokenizer_context tk_ctx)
{
    return tk_ctx.current_pos >= tk_ctx.source_code_len || tk_ctx.error;
}

LSCP_API lscp_token lscp_get_next_token(lscp_tokenizer_context* tk_ctx)
{
    #define current_char tk_ctx->source_code[tk_ctx->current_pos]
    #define reached_end_of_code (tk_ctx->current_pos >= tk_ctx->source_code_len)
    #define can_peek ((tk_ctx->current_pos + 1) >= tk_ctx->source_code_len)
    #define peek_next_char tk_ctx->source_code[tk_ctx->current_pos + 1]
    #define one_char_token_case(c, t) case c: return (lscp_token) { .type = t, .text_begin = tk_ctx->current_pos++, .text_length = 1, .line_of_code = tk_ctx->current_line, .number_in_line = tk_ctx->tokens_on_line_count++, }
    #define report_error_and_return(err) tk_ctx->error = err; return (lscp_token) {}

    if (tk_ctx->error)
    {
        return (lscp_token) {};
    }

    while (!reached_end_of_code)
    {
        switch (current_char)
        {
            //One character tokens
            //case '/': We dont handle / here because it is used to produce comments
            one_char_token_case('`',  lscp_tk_backtick);
            one_char_token_case('~',  lscp_tk_tilde);
            one_char_token_case('!',  lscp_tk_exclamation_mark);
            one_char_token_case('%',  lscp_tk_percent);
            one_char_token_case('^',  lscp_tk_hat);
            one_char_token_case('&',  lscp_tk_ampersand);
            one_char_token_case('*',  lscp_tk_star);
            one_char_token_case('(',  lscp_tk_open_paren);
            one_char_token_case(')',  lscp_tk_close_paren);
            one_char_token_case('-',  lscp_tk_minus);
            one_char_token_case('+',  lscp_tk_plus);
            one_char_token_case('=',  lscp_tk_equal);
            one_char_token_case(':',  lscp_tk_colon);
            one_char_token_case('<',  lscp_tk_left_arrow);
            one_char_token_case('>',  lscp_tk_right_arrow);
            one_char_token_case(',',  lscp_tk_comma);
            one_char_token_case('.',  lscp_tk_dot);
            one_char_token_case('[',  lscp_tk_open_square_bracket);
            one_char_token_case(']',  lscp_tk_close_square_bracket);
            one_char_token_case('{',  lscp_tk_open_curly);
            one_char_token_case('}',  lscp_tk_close_curly);
            one_char_token_case('|',  lscp_tk_bar);
            one_char_token_case(';',  lscp_tk_semicolon);
            one_char_token_case('\\', lscp_tk_backward_slash);

            case '\n':
            {
                tk_ctx->current_line++;
                tk_ctx->tokens_on_line_count = 0;

                tk_ctx->current_pos++;
                continue;
            }

            case '#':
            {
                if (tk_ctx->tokens_on_line_count != 0)
                {
                    report_error_and_return(lscp_unexpected_token);
                }

                tk_ctx->current_pos++;

                tk_ctx->current_pos = tokenizer_find_char(tk_ctx, '\n');

                continue;

                //@Todo: check if keyword
                //@Todo: check if lscp_flag_ignore_preprocessor_commands
                //const char* pp_keyword = "if ";
                //const ls_int pp_keyword_len = sizeof("if ") - 1;
                //const ls_int code_chars_left = tk_ctx->source_code_len - tk_ctx->current_pos;
                //
                //if (code_chars_left >= pp_keyword_len && string_match(&current_char, pp_keyword_len, pp_keyword, pp_keyword_len))
                //{
                //    tk_ctx->current_pos += pp_keyword_len + 1;
                //    const ls_int maybe_newline_pos = tokenizer_find_char(tk_ctx, '\n');
                //
                //    //If we havent reached the end of the source code it means we found a newline
                //    if (maybe_newline_pos != INDEX_NOT_FOUND)
                //    {
                //        tk_ctx->current_line = maybe_newline_pos + 1;
                //        tk_ctx->tokens_on_line_count = 0;
                //
                //        tk_ctx->current_pos++;
                //        continue;
                //    }
                //}
            }

            //Whitespace
            case '\t':
            case '\r':
            case ' ' :
            {
                tk_ctx->current_pos++;
                continue;
            }

            case '/':
            {
                const lscp_int begin = tk_ctx->current_pos;

                if (can_peek && peek_next_char == '/') //Single line comment
                {
                    while (!reached_end_of_code && current_char != '\n')
                    {
                        tk_ctx->current_pos++;
                    }

                    return (lscp_token)
                    {
                        .type           = lscp_tk_single_line_comment,
                        .text_begin     = begin,
                        .text_length    = tk_ctx->current_pos - begin,
                        .line_of_code   = reached_end_of_code ? tk_ctx->current_line : tk_ctx->current_line++, //Only increase the current line counter if we didnt reach the end of code
                        .number_in_line = tk_ctx->tokens_on_line_count++,
                    };
                }
                else if (can_peek && peek_next_char == '*') //Multi line comment
                {
                    const lscp_int line = tk_ctx->current_line;
                    const lscp_int number_in_line = tk_ctx->tokens_on_line_count;

                    while (!reached_end_of_code)
                    {
                        if (current_char == '\n')
                        {
                            tk_ctx->current_line++;
                            tk_ctx->tokens_on_line_count = 0;
                        }
                        //If found comment_end
                        else if (current_char == '*' && can_peek && peek_next_char == '/')
                        {
                            tk_ctx->current_pos += 2; //We should go 1 past the closing forward slash

                            //if no characters left then return single char token
                            return (lscp_token)
                            {
                                .type           = lscp_tk_multi_line_comment,
                                .text_begin     = begin,
                                .text_length    = (lscp_int) (tk_ctx->current_pos - begin),
                                .line_of_code   = line,
                                .number_in_line = number_in_line,
                            };
                        }
                    }
                }
                else //handle operator/
                {
                    tk_ctx->current_pos++;

                    return (lscp_token)
                    {
                        .type           = lscp_tk_forward_slash,
                        .text_begin     = begin,
                        .text_length    = 1,
                        .line_of_code   = tk_ctx->current_line,
                        .number_in_line = tk_ctx->tokens_on_line_count++,
                    };
                }
            }

            case '"':
            {
                const lscp_int begin = tk_ctx->current_pos; //Position of the first quotation mark
                lscp_int ending_quote_pos = LSCP_INDEX_NOT_FOUND; //Position of the ending quotation mark

                //Look until the end of the string for an ending quotation mark
                for (lscp_int i = begin, escape_next_char = 0; i < tk_ctx->source_code_len && ending_quote_pos == -1; i++)
                {
                    const char curr_char = tk_ctx->source_code[i];

                    if (curr_char == '\n')
                    {
                        report_error_and_return(lscp_string_literal_not_closed);
                    }

                    if (curr_char == '"' && !escape_next_char)
                    {
                        ending_quote_pos = i;
                    }

                    escape_next_char = curr_char == '\\' && !escape_next_char;
                }

                if (ending_quote_pos == LSCP_INDEX_NOT_FOUND)
                {
                    report_error_and_return(lscp_string_literal_not_closed);
                }

                const lscp_int end = ending_quote_pos + 1;
                const lscp_int len = end - begin;

                tk_ctx->current_pos += len;

                return (lscp_token)
                {
                    .type           = lscp_tk_string_literal,
                    .text_begin     = begin,
                    .text_length    = len,
                    .line_of_code   = tk_ctx->current_line,
                    .number_in_line = tk_ctx->tokens_on_line_count++,
                };
            }

            default:
            {
                if (_lscp_is_ascii_digit(current_char))
                {
                    const lscp_int numeric_literal_begin = tk_ctx->current_pos;
                    //@Unimplemented
                }
                else if (_lscp_is_ascii_letter(current_char) || current_char == '_')
                {
                    const lscp_int begin = tk_ctx->current_pos;

                    tk_ctx->current_pos++;

                    //@Todo: Add utf8 parsing to check for valid identifiers in C
                    //Eat characters until its no longer a valid word (http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf check Annex D)
                    while (!reached_end_of_code && (_lscp_is_ascii_letter(current_char) || current_char == '_' ||
                                                    _lscp_is_ascii_digit(current_char)))
                    {
                        tk_ctx->current_pos++;
                    }

                    const lscp_int end = tk_ctx->current_pos;
                    const lscp_int len = (lscp_int) (end - begin);
                    const lscp_token_type token_type = _lscp_get_token_type_if_string_is_keyword(&tk_ctx->source_code[begin], len);

                    return (lscp_token)
                    {
                        .type           = token_type == lscp_tk_unknown ? lscp_tk_identifier : token_type,
                        .text_begin     = begin,
                        .text_length    = len,
                        .line_of_code   = tk_ctx->current_line,
                        .number_in_line = tk_ctx->tokens_on_line_count++,
                    };
                }
            }
        }
    }

    return (lscp_token) {};

    #undef current_char
    #undef reached_end_of_code
    #undef can_peek
    #undef peek_next_char
    #undef one_char_token_case
}

#undef _lscp_str_literal_len

#endif
//endregion