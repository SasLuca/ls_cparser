#include <catch2/catch.hpp>
#include "../ls_cparser.h"
#include <cstring>

#define MAX_TOKEN_COUNT 1024

TEST_CASE("Tokenizer test", "[tokenizer]")
{
    const char* source_code = "#include <stdio.h>\n"
                              "#define NUM 0\n"
                              "int main() { return NUM; }";

    const int source_code_len = strlen(source_code);

    ls_cparser_tokenizer_context tokenizer_context = { source_code, source_code_len };
    ls_cparser_token tokens[MAX_TOKEN_COUNT];
    int tokens_count = 0;

    while (true)
    {
        const ls_cparser_token tk = ls_cparser_get_next_token(&tokenizer_context);

        if (ls_cparser_tokenizer_is_done(tokenizer_context) || tokens_count >= MAX_TOKEN_COUNT) break;

        tokens[tokens_count++] = tk;
    }
}

