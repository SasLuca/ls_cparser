//region interface
#ifndef LS_CPARSER_H
#define LS_CPARSER_H

#define ls_int signed long long
#define ls_bool int

#ifdef __cplusplus
    #ifndef LS_API
        #define LS_API extern "C"
    #endif//LS_CPARSER_API
#else
    #ifndef LS_API
        #define LS_API extern
    #endif//LS_CPARSER_API
#endif

#ifndef LS_INTERNAL
#define LS_INTERNAL static
#endif//LS_CPARSER_INTERNAL

//region structs
typedef enum ls_cparser_error
{
    ls_cparser_no_error = 0,
    ls_cparser_unexpected_end_of_source,
    ls_cparser_newline_in_string_literal,
    ls_cparser_string_literal_not_closed,
    ls_cparser_unexpected_token,
} ls_cparser_error;

typedef enum ls_cparser_token_type
{
    //region keywords

    alignof_keyword,
    and_keyword,
    asm_keyword,
    bool_keyword,
    break_keyword,
    case_keyword,
    catch_keyword,
    char_keyword,
    continue_keyword,
    default_keyword,
    do_keyword,
    else_keyword,
    false_keyword,
    for_keyword,
    goto_keyword,
    if_keyword,
    import_keyword,
    inline_keyword,
    int_keyword,
    mut_keyword,
    not_keyword,
    null_keyword,
    operator_keyword,
    or_keyword,
    private_keyword,
    public_keyword,
    return_keyword,
    sizeof_keyword,
    struct_keyword,
    switch_keyword,
    this_keyword,
    true_keyword,
    try_keyword,
    union_keyword,
    typedef_keyword,
    typename_keyword,
    using_keyword,
    void_keyword,
    with_keyword,
    while_keyword,
    enum_keyword,

    //endregion

    ls_token_type_identifier,
    ls_token_type_string_literal,
    ls_token_type_int_literal,
    ls_token_type_float_literal,
    ls_token_type_double_literal,
    ls_token_type_single_line_comment,
    ls_token_type_multi_line_comment,

    ls_token_type_backtick,              // `
    ls_token_type_tilde,                 // ~
    ls_token_type_exclamation_mark,      // !
    ls_token_type_percent,               // %
    ls_token_type_hat,                   // ^
    ls_token_type_ampersand,             // &
    ls_token_type_star,                  // *
    ls_token_type_open_paren,            // (
    ls_token_type_close_paren,           // )
    ls_token_type_minus,                 // -
    ls_token_type_plus,                  // +
    ls_token_type_equal,                 // =
    ls_token_type_colon,                 // :
    ls_token_type_left_arrow,            // <
    ls_token_type_right_arrow,           // >
    ls_token_type_comma,                 // ,
    ls_token_type_dot,                   // .
    ls_token_type_open_square_bracket,   // [
    ls_token_type_close_square_bracket,  // ]
    ls_token_type_open_curly,            // {
    ls_token_type_close_curly,           // }
    ls_token_type_bar,                   // |
    ls_token_type_semicolon,             // ;
    ls_token_type_forward_slash,         // /
    ls_token_type_backward_slash,        // \

} ls_cparser_token_type;

typedef struct ls_cparser_token ls_cparser_token;
struct ls_cparser_token
{
    ls_cparser_token_type type;
    ls_int                text_begin;
    ls_int                text_length;
    ls_int                line_of_code;
    ls_int                number_in_line; //The number of the token in the line its on.
};

typedef struct ls_cparser_tokenizer_context ls_cparser_tokenizer_context;
struct ls_cparser_tokenizer_context
{
    const char* source_code;
    ls_int      source_code_len;

    ls_int      current_pos; //Current position in the source code
    ls_int      current_line;
    ls_int      tokens_on_line_count;
    ls_int      ignored_utf8_bytes_count;

    ls_cparser_error error;
};
//endregion

LS_API ls_bool ls_cparser_tokenizer_is_done(ls_cparser_tokenizer_context tk_ctx);
LS_API ls_cparser_token ls_cparser_get_next_token(ls_cparser_tokenizer_context* tk_ctx);

#endif //LS_CPARSER_H
//endregion