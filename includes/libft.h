#pragma once

#define TOOL_NAME "ft_nm"
#define ELF64 0
#define ELF32 1

#include <unistd.h>

/**
 * Retrieves the count of files based on the command-line arguments.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The count of files.
 */
int get_file_count(int argc, char **argv)
{
    int argIndex;
    int fileCount;

    argIndex = fileCount = 0;
    while (++argIndex < argc)
    {
        fileCount += (argv[argIndex][0] != '-');
    }
    return fileCount;
}

/**
 * Calculates the length of a null-terminated string.
 *
 * @param string The string to calculate the length of.
 * @return The length of the string.
 */
int string_length(char *string)
{
    int length = 0;
    while (string[length] != '\0')
        length++;
    return length;
}

/**
 * Compares two strings lexicographically.
 *
 * @param string1 The first string to compare.
 * @param string2 The second string to compare.
 * @return A negative value if string1 is less than string2, a positive value if string1 is greater than string2,
 *         or 0 if both strings are equal.
 */
int string_compare(char *string1, char *string2)
{
    int index = 0;
    while (string1[index] != '\0' && string2[index] != '\0')
    {
        if (string1[index] != string2[index])
            return string1[index] - string2[index];
        index++;
    }
    return string1[index] - string2[index];
}

/**
 * Sets all bytes of a memory region to zero.
 *
 * @param ptr The pointer to the memory region.
 * @param size The size of the memory region.
 */
void be_zero(void *ptr, size_t size)
{
    char *ptrChar = (char *)ptr;
    for (size_t i = 0; i < size; i++)
        ptrChar[i] = 0;
}

/**
 * Prints file errors with the provided pre-message, name, and after-message.
 *
 * @param pre_message The pre-message to be printed.
 * @param name The name to be printed.
 * @param after_message The after-message to be printed.
 * @return 0 to indicate successful execution.
 */
int file_errors(char *pre_message, char *name, char *after_message)
{
    write(1, TOOL_NAME, sizeof(TOOL_NAME));
    write(1, pre_message, string_length(pre_message));
    write(1, name, string_length(name));
    write(1, after_message, string_length(after_message));
    return 0;
}
