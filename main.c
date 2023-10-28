/*
Write a program to check a C program for rudimentary syntax errors like
unmatched parentheses, brackets and braces. Don't forget about quotes, both single and
double, escape sequences, and comments. (This program is hard if you do it in full
generality.)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

char* read_file(char* file_path)
{
    FILE* file_handle = fopen(file_path, "rb");
    if (file_handle == NULL) { printf("Failed to open file '%s'\n", file_path); exit(1); }
    fseek(file_handle, 0, SEEK_END);
    int file_size = ftell(file_handle);
    rewind(file_handle);
    char* file_contents = malloc(file_size + 1);
    int bytes_read = (int)fread(file_contents, 1, file_size, file_handle);
    if (bytes_read != file_size)
    {
        printf("Was only able to read %d bytes out of %d expected for file '%s'\n", bytes_read, file_size, file_path);
        exit(1);
    }
    file_contents[file_size] = '\0';
    fclose(file_handle);
    return file_contents;
}

char* copy_string(char* source)
{
    int source_size = strlen(source);
    char* result = malloc(source_size + 1);
    memcpy(result, source, source_size + 1);
    return result;
}

typedef enum
{
    DelimiterParenthesis,
    DelimiterBracket,
    DelimiterBrace,
} Delimiter;

char closing_delimiter_to_character(Delimiter source)
{
    switch (source)
    {
        case DelimiterParenthesis: return ')';
        case DelimiterBracket: return ']';
        case DelimiterBrace: return '}';
        default:
            printf("`closing_delimiter_to_character` received an invalid argument: %d\n", source);
            exit(1);
    }
}

typedef enum
{
    ValidationResultTypeSuccess,
    ValidationResultTypeExtraClosingDelimiter,
    ValidationResultTypeWrongDelimiter,
    ValidationResultTypeUnmatchedDelimiters,
    ValidationResultTypeUnterminatedQuote,
    ValidationResultTypeUnterminatedBlockComment,
} ValidationResultType;

typedef struct
{
    ValidationResultType type;
    int error_line;
    int error_character;
    union
    {
        Delimiter extra_closing_delimiter;
        struct
        {
            Delimiter wrong_delimiter_expected;
            Delimiter wrong_delimiter_actual;
        };
        int unmatched_delimiters_count;
        bool unterminated_quote_is_single_quote;
    };
} ValidationResult;

ValidationResult make_successful_validation_result()
{
    ValidationResult result;
    result.type = ValidationResultTypeSuccess;
    return result;
}

bool are_validation_results_equal(ValidationResult left, ValidationResult right)
{
    if (left.type != right.type) { return false; }
    switch (left.type)
    {
        case ValidationResultTypeSuccess: return true;
        case ValidationResultTypeExtraClosingDelimiter:
            return left.error_line == right.error_line
                && left.error_character == right.error_character
                && left.extra_closing_delimiter == right.extra_closing_delimiter;
        case ValidationResultTypeWrongDelimiter:
            return left.error_line == right.error_line
                && left.error_character == right.error_character
                && left.wrong_delimiter_actual == right.wrong_delimiter_actual
                && left.wrong_delimiter_expected == right.wrong_delimiter_expected;
        case ValidationResultTypeUnmatchedDelimiters:
            return left.unmatched_delimiters_count == right.unmatched_delimiters_count;
        case ValidationResultTypeUnterminatedQuote:
            return left.unterminated_quote_is_single_quote == right.unterminated_quote_is_single_quote;
        case ValidationResultTypeUnterminatedBlockComment: return true;
    }
}

char* validation_result_to_string(ValidationResult validation_result)
{
    switch (validation_result.type)
    {
        case ValidationResultTypeSuccess:
            return copy_string("successful validation");
        case ValidationResultTypeExtraClosingDelimiter:
        {
            int error_message_capacity = 1024;
            char* error_message = malloc(error_message_capacity);
            snprintf(
                error_message,
                error_message_capacity,
                "failed validation: extra '%c' at line %d, character %d",
                closing_delimiter_to_character(validation_result.extra_closing_delimiter),
                validation_result.error_line,
                validation_result.error_character
            );
            return error_message;
        }
        case ValidationResultTypeWrongDelimiter:
        {
            int error_message_capacity = 1024;
            char* error_message = malloc(error_message_capacity);
            snprintf(
                error_message,
                error_message_capacity,
                "failed validation: expected '%c' at line %d, character %d, but got '%c'",
                closing_delimiter_to_character(validation_result.wrong_delimiter_expected),
                validation_result.error_line,
                validation_result.error_character,
                closing_delimiter_to_character(validation_result.wrong_delimiter_actual)
            );
            return error_message;
        }
        case ValidationResultTypeUnmatchedDelimiters:
        {
            int error_message_capacity = 1024;
            char* error_message = malloc(error_message_capacity);
            snprintf(
                error_message,
                error_message_capacity,
                "failed validation: left %d unmatched delimiters",
                validation_result.unmatched_delimiters_count
            );
            return error_message;
        }
        case ValidationResultTypeUnterminatedQuote:
        {
            int error_message_capacity = 1024;
            char* error_message = malloc(error_message_capacity);
            snprintf(
                error_message,
                error_message_capacity,
                "failed validation: unterminated %s quote",
                validation_result.unterminated_quote_is_single_quote ? "single" : "double"
            );
            return error_message;
        }
        case ValidationResultTypeUnterminatedBlockComment:
            return copy_string("failed validation: unterminated block comment");
        default:
            printf(
                "`validation_result_to_string` received a `ValidationResult` with an invalid `type` value: %d\n",
                validation_result.type
            );
            exit(1);
    }
}

typedef struct
{
    Delimiter* delimiter_stack_data;
    int delimiter_stack_size;
    int delimiter_stack_capacity;
    int line; // 1-based
    int character; // 1-based
    bool is_inside_quotes;
    bool is_inside_single_quotes;
    bool is_escaped;
    bool is_inside_comment;
    bool is_inside_line_comment;
    int line_comment_state_machine;
    int block_comment_state_machine;
} ValidationState;

ValidationState make_validation_state()
{
    ValidationState result;
    result.delimiter_stack_size = 0;
    result.delimiter_stack_capacity = 16;
    result.delimiter_stack_data = malloc(sizeof(Delimiter) * result.delimiter_stack_capacity);
    result.line = 1;
    result.character = 1;
    result.is_inside_quotes = false;
    result.is_escaped = false;
    result.is_inside_comment = false;
    result.is_inside_line_comment = false;
    result.line_comment_state_machine = 0;
    result.block_comment_state_machine = 0;
    return result;
}

void deallocate_validation_state(ValidationState state) { free(state.delimiter_stack_data); }

void push_delimiter(Delimiter delimiter, ValidationState* state)
{
    if (state->delimiter_stack_size == state->delimiter_stack_capacity)
    {
        state->delimiter_stack_capacity *= 2;
        state->delimiter_stack_data = realloc(state->delimiter_stack_data, state->delimiter_stack_capacity);
    }
    state->delimiter_stack_data[state->delimiter_stack_size] = delimiter;
    state->delimiter_stack_size++;
}

int get_delimiter_stack_size(ValidationState state) { return state.delimiter_stack_size; }

bool is_delimiter_stack_empty(ValidationState state) { return state.delimiter_stack_size == 0; }

Delimiter get_last_delimiter(ValidationState state)
{ 
    if (state.delimiter_stack_size == 0)
    {
        printf("`get_last_delimiter` was called on an empty delimiter stack\n");
        exit(1);
    }
    return state.delimiter_stack_data[state.delimiter_stack_size - 1];
}

void pop_delimiter(ValidationState* state) { state->delimiter_stack_size--; }

void update_tracking_information(char source, ValidationState* state)
{
    if (source == '\n')
    {
        state->line++;
        state->character = 1;
    }
    else { state->character++; }

    if (!state->is_inside_comment)
    {
        if (
            (source == '"' || source == '\'')
                && !state->is_escaped
                && (!state->is_inside_quotes || (source == '\'') == state->is_inside_single_quotes)
        )
        {
            state->is_inside_quotes = !state->is_inside_quotes;
            if (state->is_inside_quotes) { state->is_inside_single_quotes = source == '\''; }
        }

        state->is_escaped = source == '\\' && !state->is_escaped;
    }

    if (!state->is_inside_quotes)
    {
        if (!state->is_inside_comment)
        {
            if (source == '/')
            {
                state->line_comment_state_machine++;
                state->block_comment_state_machine = 1;
            }
            else if (source == '*')
            {
                if (state->block_comment_state_machine == 1) { state->block_comment_state_machine = 2; }
            }
            else
            {
                state->line_comment_state_machine = 0;
                state->block_comment_state_machine = 0;
            }

            if (state->line_comment_state_machine == 2)
            {
                state->is_inside_comment = true;
                state->is_inside_line_comment = true;

                state->line_comment_state_machine = 0;
                state->block_comment_state_machine = 0;
            }
            else if (state->block_comment_state_machine == 2)
            {
                state->is_inside_comment = true;
                state->is_inside_line_comment = false;

                state->line_comment_state_machine = 0;
                state->block_comment_state_machine = 0;
            }
        }
        else
        {
            if (state->is_inside_line_comment)
            {
                if (source == '\n') { state->is_inside_comment = false; }
            }
            else
            {
                if (source == '*') { state->block_comment_state_machine = 1; }
                else if (source == '/' && state->block_comment_state_machine == 1)
                { state->block_comment_state_machine = 2; }
                else { state->block_comment_state_machine = 0; }

                if (state->block_comment_state_machine == 2)
                {
                    state->is_inside_comment = false;
                    state->block_comment_state_machine = 0;
                }
            }
        }
    }
}

typedef struct
{
    bool success;
    Delimiter delimiter;
    bool is_opening;
} ParsedDelimiter;

ParsedDelimiter parse_delimiter(char source)
{
    ParsedDelimiter result;
    switch (source)
    {
        case '(':
            result.success = true;
            result.delimiter = DelimiterParenthesis;
            result.is_opening = true;
            break;
        case '[':
            result.success = true;
            result.delimiter = DelimiterBracket;
            result.is_opening = true;
            break;
        case '{':
            result.success = true;
            result.delimiter = DelimiterBrace;
            result.is_opening = true;
            break;
        case ')':
            result.success = true;
            result.delimiter = DelimiterParenthesis;
            result.is_opening = false;
            break;
        case ']':
            result.success = true;
            result.delimiter = DelimiterBracket;
            result.is_opening = false;
            break;
        case '}':
            result.success = true;
            result.delimiter = DelimiterBrace;
            result.is_opening = false;
            break;
        default:
            result.success = false;
            break;
    }
    return result;
}

ValidationResult validate(char* source)
{
    ValidationState state = make_validation_state();
    for (int i = 0; source[i] != '\0'; i++)
    {
        if (!state.is_inside_quotes && !state.is_inside_comment)
        {
            ParsedDelimiter parsed_delimiter = parse_delimiter(source[i]);
            if (!parsed_delimiter.success) { }
            else if (parsed_delimiter.is_opening) { push_delimiter(parsed_delimiter.delimiter, &state); }
            else
            {
                if (is_delimiter_stack_empty(state))
                {
                    ValidationResult result;
                    result.type = ValidationResultTypeExtraClosingDelimiter;
                    result.error_line = state.line;
                    result.error_character = state.character;
                    result.extra_closing_delimiter = parsed_delimiter.delimiter;
                    return result;
                }
                if (get_last_delimiter(state) != parsed_delimiter.delimiter)
                {
                    ValidationResult result;
                    result.type = ValidationResultTypeWrongDelimiter;
                    result.error_line = state.line;
                    result.error_character = state.character;
                    result.wrong_delimiter_actual = parsed_delimiter.delimiter;
                    result.wrong_delimiter_expected = get_last_delimiter(state);
                    return result;
                }
                pop_delimiter(&state);
            }
        }
        update_tracking_information(source[i], &state);
    }
    if (!is_delimiter_stack_empty(state))
    {
        ValidationResult result;
        result.type = ValidationResultTypeUnmatchedDelimiters;
        result.unmatched_delimiters_count = get_delimiter_stack_size(state);
        return result;
    }
    if (state.is_inside_quotes)
    {
        ValidationResult result;
        result.type = ValidationResultTypeUnterminatedQuote;
        result.unterminated_quote_is_single_quote = state.is_inside_single_quotes;
        return result;
    }
    if (state.is_inside_comment && !state.is_inside_line_comment)
    {
        ValidationResult result;
        result.type = ValidationResultTypeUnterminatedBlockComment;
        return result;
    }
    deallocate_validation_state(state);
    return make_successful_validation_result();
}

bool all_test_cases_passed = true;

void test_case(char* test_file_path, ValidationResult expected_validation_result)
{
    char* test_file_contents = read_file(test_file_path);
    ValidationResult actual_validation_result = validate(test_file_contents);
    free(test_file_contents);
    if (!are_validation_results_equal(actual_validation_result, expected_validation_result))
    {
        all_test_cases_passed = false;
        char* expected_validation_result_string = validation_result_to_string(expected_validation_result);
        char* actual_validation_result_string = validation_result_to_string(actual_validation_result);
        printf(
            "Validation test for file '%s' failed: expected %s; got %s\n",
            test_file_path,
            expected_validation_result_string,
            actual_validation_result_string
        );
        free(actual_validation_result_string);
        free(expected_validation_result_string);
    }
}

int main()
{
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnmatchedDelimiters;
        expected_validation_result.unmatched_delimiters_count = 1;
        test_case("test files/test1.txt", expected_validation_result);
    }
    test_case("test files/test2.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnmatchedDelimiters;
        expected_validation_result.unmatched_delimiters_count = 1;
        test_case("test files/test3.txt", expected_validation_result);
    }
    test_case("test files/test4.txt", make_successful_validation_result());
    test_case("test files/test5.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeWrongDelimiter;
        expected_validation_result.error_line = 1;
        expected_validation_result.error_character = 3;
        expected_validation_result.wrong_delimiter_actual = DelimiterParenthesis;
        expected_validation_result.wrong_delimiter_expected = DelimiterBracket;
        test_case("test files/test6.txt", expected_validation_result);
    }
    test_case("test files/test7.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnterminatedQuote;
        expected_validation_result.unterminated_quote_is_single_quote = true;
        test_case("test files/test8.txt", expected_validation_result);
    }
    test_case("test files/test9.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnterminatedQuote;
        expected_validation_result.unterminated_quote_is_single_quote = false;
        test_case("test files/test10.txt", expected_validation_result);
    }
    test_case("test files/test11.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnterminatedQuote;
        expected_validation_result.unterminated_quote_is_single_quote = true;
        test_case("test files/test12.txt", expected_validation_result);
    }
    test_case("test files/test13.txt", make_successful_validation_result());
    test_case("test files/test14.txt", make_successful_validation_result());
    test_case("test files/test15.txt", make_successful_validation_result());
    test_case("test files/test16.txt", make_successful_validation_result());
    test_case("test files/test17.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnterminatedQuote;
        expected_validation_result.unterminated_quote_is_single_quote = true;
        test_case("test files/test18.txt", expected_validation_result);
    }
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnterminatedQuote;
        expected_validation_result.unterminated_quote_is_single_quote = false;
        test_case("test files/test19.txt", expected_validation_result);
    }
    test_case("test files/test20.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeExtraClosingDelimiter;
        expected_validation_result.error_line = 2;
        expected_validation_result.error_character = 1;
        expected_validation_result.extra_closing_delimiter = DelimiterBrace;
        test_case("test files/test21.txt", expected_validation_result);
    }
    test_case("test files/test22.txt", make_successful_validation_result());
    test_case("test files/test23.txt", make_successful_validation_result());
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnmatchedDelimiters;
        expected_validation_result.unmatched_delimiters_count = 1;
        test_case("test files/test24.txt", expected_validation_result);
    }
    {
        ValidationResult expected_validation_result;
        expected_validation_result.type = ValidationResultTypeUnterminatedBlockComment;
        test_case("test files/test25.txt", expected_validation_result);
    }
    test_case("test files/test26.txt", make_successful_validation_result());

    if (all_test_cases_passed)
    {
        printf("All test cases passed!\n");
    }
}
