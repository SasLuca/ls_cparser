#include "ls_cparser.h"

#ifndef __cplusplus
    #define true (1)
    #define false (0)
#endif

#define INDEX_NOT_FOUND (-1)

LS_INTERNAL inline ls_bool is_ascii_digit(const char it)
{
    return it >= '0' && it <= '9';
}

LS_INTERNAL inline ls_bool is_ascii_letter(const char it)
{
    return (it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z');
}

LS_INTERNAL inline ls_int zstring_len(const char* zstr)
{
    const char* iter = zstr;
    while (*iter++) {}
    return (ls_int) (iter - zstr);
}

LS_INTERNAL ls_bool string_match(const char* a, const ls_int a_len, const char* b, const ls_int b_len)
{
    if (a_len != b_len) return false;

    const char* a_iter = a;
    const char* b_iter = b;

    while ((ls_int) (a_iter - a) < a_len &&
          ((ls_int) (b_iter - b) < b_len))
    {
        if (*a_iter != *b_iter)
        {
            return false;
        }
    }

    return true;
}

//Returns the position of the char
LS_INTERNAL ls_int tokenizer_find_char(const ls_cparser_tokenizer_context* tk_ctx, const char until_c)
{
    for (ls_int i = tk_ctx->current_pos; i < tk_ctx->source_code_len; i++)
    {
        const char current_char = tk_ctx->source_code[i];
        if (current_char == until_c)
        {
            return i;
        }
    }

    return tk_ctx->source_code_len;
}

LS_API ls_bool ls_cparser_tokenizer_is_done(const ls_cparser_tokenizer_context tk_ctx)
{
    return tk_ctx.current_pos >= tk_ctx.source_code_len || tk_ctx.error;
}

LS_API ls_cparser_token ls_cparser_get_next_token(ls_cparser_tokenizer_context* tk_ctx)
{
    #define current_char tk_ctx->source_code[tk_ctx->current_pos]
    #define reached_end_of_code (tk_ctx->current_pos >= tk_ctx->source_code_len)
    #define can_peek ((tk_ctx->current_pos + 1) >= tk_ctx->source_code_len)
    #define peek_next_char tk_ctx->source_code[tk_ctx->current_pos + 1]
    #define one_char_token_case(c, t) case c: return (ls_cparser_token) { .type = t, .text_begin = tk_ctx->current_pos++, .text_length = 1, .line_of_code = tk_ctx->current_line, .number_in_line = tk_ctx->tokens_on_line_count++, }
    #define report_error_and_return(err) tk_ctx->error = err; return (ls_cparser_token) {}

    if (tk_ctx->error)
    {
        return (ls_cparser_token) {};
    }

    while (!reached_end_of_code)
    {
        switch (current_char)
        {
            //One character tokens
            //case '/': We dont handle / here because it is used to produce comments
            one_char_token_case('`', ls_token_type_backtick);
            one_char_token_case('~', ls_token_type_tilde);
            one_char_token_case('!', ls_token_type_exclamation_mark);
            one_char_token_case('%', ls_token_type_percent);
            one_char_token_case('^', ls_token_type_hat);
            one_char_token_case('&', ls_token_type_ampersand);
            one_char_token_case('*', ls_token_type_star);
            one_char_token_case('(', ls_token_type_open_paren);
            one_char_token_case(')', ls_token_type_close_paren);
            one_char_token_case('-', ls_token_type_minus);
            one_char_token_case('+', ls_token_type_plus);
            one_char_token_case('=', ls_token_type_equal);
            one_char_token_case(':', ls_token_type_colon);
            one_char_token_case('<', ls_token_type_left_arrow);
            one_char_token_case('>', ls_token_type_right_arrow);
            one_char_token_case(',', ls_token_type_comma);
            one_char_token_case('.', ls_token_type_dot);
            one_char_token_case('[', ls_token_type_open_square_bracket);
            one_char_token_case(']', ls_token_type_close_square_bracket);
            one_char_token_case('{', ls_token_type_open_curly);
            one_char_token_case('}', ls_token_type_close_curly);
            one_char_token_case('|', ls_token_type_bar);
            one_char_token_case(';', ls_token_type_semicolon);
            one_char_token_case('\\', ls_token_type_backward_slash);

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
                    report_error_and_return(ls_cparser_unexpected_token);
                }

                tk_ctx->current_pos++;

                tk_ctx->current_pos = tokenizer_find_char(tk_ctx, '\n');

                continue;

                //@Todo: check if keyword
                //@Todo: check if ls_cparser_flag_ignore_preprocessor_commands
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
                const ls_int begin = tk_ctx->current_pos;

                if (can_peek && peek_next_char == '/') //Single line comment
                {
                    while (!reached_end_of_code && current_char != '\n')
                    {
                        tk_ctx->current_pos++;
                    }

                    return (ls_cparser_token)
                    {
                        .type           = ls_token_type_single_line_comment,
                        .text_begin     = begin,
                        .text_length    = tk_ctx->current_pos - begin,
                        .line_of_code   = reached_end_of_code ? tk_ctx->current_line : tk_ctx->current_line++, //Only increase the current line counter if we didnt reach the end of code
                        .number_in_line = tk_ctx->tokens_on_line_count++,
                    };
                }
                else if (can_peek && peek_next_char == '*') //Multi line comment
                {
                    const ls_int line = tk_ctx->current_line;
                    const ls_int number_in_line = tk_ctx->tokens_on_line_count;

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
                            return (ls_cparser_token)
                            {
                                .type           = ls_token_type_multi_line_comment,
                                .text_begin     = begin,
                                .text_length    = (ls_int) (tk_ctx->current_pos - begin),
                                .line_of_code   = line,
                                .number_in_line = number_in_line,
                            };
                        }
                    }
                }
                else //handle operator/
                {
                    tk_ctx->current_pos++;

                    return (ls_cparser_token)
                    {
                        .type           = ls_token_type_forward_slash,
                        .text_begin     = begin,
                        .text_length    = 1,
                        .line_of_code   = tk_ctx->current_line,
                        .number_in_line = tk_ctx->tokens_on_line_count++,
                    };
                }
            }

            case '"':
            {
                const ls_int begin = tk_ctx->current_pos; //Position of the first quotation mark
                ls_int ending_quote_pos = INDEX_NOT_FOUND; //Position of the ending quotation mark

                //Look until the end of the string for an ending quotation mark
                for (ls_int i = begin, escape_next_char = false; i < tk_ctx->source_code_len && ending_quote_pos == -1; i++)
                {
                    const char curr_char = tk_ctx->source_code[i];

                    if (curr_char == '\n')
                    {
                        report_error_and_return(ls_cparser_string_literal_not_closed);
                    }

                    if (curr_char == '"' && !escape_next_char)
                    {
                        ending_quote_pos = i;
                    }

                    escape_next_char = curr_char == '\\' && !escape_next_char;
                }

                if (ending_quote_pos == INDEX_NOT_FOUND)
                {
                    report_error_and_return(ls_cparser_string_literal_not_closed);
                }

                const ls_int end = ending_quote_pos + 1;
                const ls_int len = end - begin;

                tk_ctx->current_pos += len;

                return (ls_cparser_token)
                {
                    .type           = ls_token_type_string_literal,
                    .text_begin     = begin,
                    .text_length    = len,
                    .line_of_code   = tk_ctx->current_line,
                    .number_in_line = tk_ctx->tokens_on_line_count++,
                };
            }

            default:
            {
                if (is_ascii_digit(current_char))
                {
                    const ls_int numeric_literal_begin = tk_ctx->current_pos;
                    //@Unimplemented
                }
                else if (is_ascii_letter(current_char) || current_char == '_')
                {
                    const ls_int identifier_begin = tk_ctx->current_pos;

                    tk_ctx->current_pos++;

                    //@Todo: Add utf8 parsing to check for valid identifiers in C
                    //Eat characters until its no longer a valid word (http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf check Annex D)
                    while (!reached_end_of_code && (is_ascii_letter(current_char) || current_char == '_' || is_ascii_digit(current_char)))
                    {
                        tk_ctx->current_pos++;
                    }

                    const ls_int identifier_end = tk_ctx->current_pos;
                    const ls_int identifier_size = (ls_int) (identifier_end - identifier_begin);

                    return (ls_cparser_token)
                    {
                        .type           = ls_token_type_identifier,
                        .text_begin     = identifier_begin,
                        .text_length    = identifier_size,
                        .line_of_code   = tk_ctx->current_line,
                        .number_in_line = tk_ctx->tokens_on_line_count++,
                    };
                }
            }
        }
    }

    return (ls_cparser_token) {};

    #undef current_char
    #undef reached_end_of_code
    #undef can_peek
    #undef peek_next_char
    #undef one_char_token_case
}