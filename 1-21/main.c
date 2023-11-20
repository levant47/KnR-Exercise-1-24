/*
Write a program entab that replaces strings of blanks by the minimum number of tabs and blanks to achieve the same
spacing. Use the same tab stops as for detab.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAB_SIZE 4
#define MAX_STRING 1024

char* entab(char* input)
{
    char* result = malloc(MAX_STRING);
    int result_i = 0;
    bool counting_blanks = false;
    int blanks_start;
    for (int i = 0; ; i++)
    {
        if (!counting_blanks)
        {
            if (input[i] == ' ')
            { // don't print anything, count the blanks first
                counting_blanks = true;
                blanks_start = i;
            }
            else { result[result_i++] = input[i]; }
        }
        else if (input[i] != ' ')
        {
            counting_blanks = false;
            int j = blanks_start;
            // print as many tabs as possible first
            while (j < i / TAB_SIZE * TAB_SIZE)
            {
                result[result_i++] = '\t';
                j = (j / TAB_SIZE + 1) * TAB_SIZE;
            }
            // fill the rest with spaces
            while (j < i)
            {
                result[result_i++] = ' ';
                j++;
            }

            result[result_i++] = input[i]; // copy the actual non-blank character that we stopped at
        }

        if (input[i] == '\0') { break; }
    }
    result[result_i] = '\0';
    return result;
}

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

bool all_test_cases_passed = true;

void test_case(int file_i)
{
    char input_file_path[256];
    snprintf(input_file_path, sizeof(input_file_path), "test files/test %d input.txt", file_i);
    char* input_file_contents = read_file(input_file_path);
    char expected_output_file_path[256];
    snprintf(expected_output_file_path, sizeof(expected_output_file_path), "test files/test %d output.txt", file_i);
    char* expected_output_file_contents = read_file(expected_output_file_path);
    char* detab_output = entab(input_file_contents);
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
    int test_case_count = 5;
    for (int i = 1; i <= test_case_count; i++) { test_case(i); }

    if (all_test_cases_passed) { printf("All %d test cases passed!\n", test_case_count); }
}
