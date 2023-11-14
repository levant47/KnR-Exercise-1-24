/*
Write a program `detab` that replaces tabs in the input with the proper number of blanks to space to the next tab stop.
Assume a fixed set of tab stops, say every `n` columns.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAB_SIZE 4
#define MAX_STRING 1024

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

char* detab(char* input)
{
    char* result = malloc(MAX_STRING);
    int result_i = 0;
    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '\t')
        {
            int spaces_count = TAB_SIZE - (result_i % TAB_SIZE);
            for (int j = 0; j < spaces_count; j++) { result[result_i++] = ' '; }
        }
        else { result[result_i++] = input[i]; }
    }
    result[result_i] = '\0';
    return result;
}

bool all_test_cases_passed = true;

void test_case(int file_i)
{
    char input_file_path[256];
    snprintf(input_file_path, sizeof(input_file_path), "test files/test %d input.txt", file_i);
    char* input_file_contents = read_file(input_file_path);
    char expected_output_file_path[256];
    snprintf(expected_output_file_path, sizeof(expected_output_file_path), "test files/test %d output.txt", file_i);
    char* expected_output_file_contents = read_file(expected_output_file_path);
    char* detab_output = detab(input_file_contents);
    if (strcmp(detab_output, expected_output_file_contents) != 0)
    {
        all_test_cases_passed = false;
        printf(
            "Test %d failed failed: expected `%s`; got `%s`\n",
            file_i,
            expected_output_file_contents,
            detab_output
        );
    }
    free(detab_output);
    free(expected_output_file_contents);
    free(input_file_contents);
}

int main()
{
    for (int i = 1; i <= 4; i++) { test_case(i); }

    if (all_test_cases_passed) { printf("All test cases passed!\n"); }
}
