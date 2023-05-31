#pragma once

#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <elf.h>
#include <stdbool.h>

#include "libft.h"

#define MAGIC_NUMBER 0x464C457F

typedef struct MagicNumber
{
    uint32_t magic_number;
    uint8_t support;
    uint8_t endian;
    uint8_t version;
    uint8_t abi;
} MagicNumber;

typedef struct Symbol
{
    char *name;
    char *original_name;
    char *section;
    int section_type;
    int section_flags;
    int shndx;
    char bind;
    char type;
    size_t value;
} Symbol;

typedef struct Options
{
    char all;
    char globals;
    char undefined;
    char reverse;
    char not_sorted;
} Options;

typedef struct File
{
    int file_descriptor;
    size_t file_size;
    Elf64_Ehdr *elf_header;
    Elf64_Shdr *section_header;
    Elf64_Sym *symbol_table;
    Symbol *symbols;
    int symbol_count;
    char *section_string_table;
    char *string_table;
    char file_type;
} File;

/**
 * Sorts an array of symbols using the quicksort algorithm.
 *
 * @param symbols The array of symbols to be sorted.
 * @param len The number of symbols in the array.
 */
void quicksort_symbols(Symbol *symbols, int len)
{
    char *pivot;
    Symbol tempSymbol;
    int currentIndex;
    int pivotIndex;

    if (len < 2)
        return;

    // Select the last symbol as the pivot
    pivot = symbols[len - 1].name;
    currentIndex = 0;
    pivotIndex = -1;

    // Partition the array based on the pivot
    while (++pivotIndex < len)
    {
        if (string_compare(symbols[pivotIndex].name, pivot) <= 0)
        {
            if (currentIndex != pivotIndex)
            {
                // Swap symbols
                tempSymbol = symbols[currentIndex];
                symbols[currentIndex] = symbols[pivotIndex];
                symbols[pivotIndex] = tempSymbol;
            }
            currentIndex++;
        }
    }

    // Recursively sort the subarrays
    quicksort_symbols(symbols, --currentIndex);
    quicksort_symbols(&symbols[currentIndex], len - currentIndex);
}

/**
 * Sorts an array of symbols in reverse order using the quicksort algorithm.
 *
 * @param symbols The array of symbols to be sorted.
 * @param len The number of symbols in the array.
 */
void reverse_quicksort_symbols(Symbol *symbols, int len)
{
    char *pivot;
    Symbol tempSymbol;
    int currentIndex;
    int pivotIndex;

    if (len < 2)
        return;

    // Select the last symbol as the pivot
    pivot = symbols[len - 1].name;
    currentIndex = 0;
    pivotIndex = -1;

    // Partition the array based on the pivot
    while (++pivotIndex < len)
    {
        if (string_compare(symbols[pivotIndex].name, pivot) >= 0)
        {
            if (currentIndex != pivotIndex)
            {
                // Swap symbols
                tempSymbol = symbols[currentIndex];
                symbols[currentIndex] = symbols[pivotIndex];
                symbols[pivotIndex] = tempSymbol;
            }
            currentIndex++;
        }
    }

    // Recursively sort the subarrays
    reverse_quicksort_symbols(symbols, --currentIndex);
    reverse_quicksort_symbols(&symbols[currentIndex], len - currentIndex);
}

/**
 * Prints errors related to the file.
 * 
 * @param preMessage The message to be printed before the file name.
 * @param name The name of the file.
 * @param afterMessage The message to be printed after the file name.
 */
int print_file_errors(char *preMessage, char *name, char *afterMessage)
{
    write(1, TOOL_NAME, sizeof(TOOL_NAME));
    write(1, preMessage, string_length(preMessage));
    write(1, name, string_length(name));
    write(1, afterMessage, string_length(afterMessage));
    return 0;
}

/**
 * Returns the length of a number represented in hexadecimal format.
 *
 * @param number The number to calculate the length of.
 * @return The length of the hexadecimal number.
 */
int hex_number_len(size_t number)
{
    int length = 0;
    while (number)
    {
        number /= 16;
        length++;
    }
    return length;
}

/**
 * Prints a number in hexadecimal format.
 *
 * @param number The number to be printed.
 */
void print_hex_number(size_t number)
{
    static char hex[] = "0123456789abcdef";

    if (number >= 16)
    {
        print_hex_number(number / 16);
        print_hex_number(number % 16);
    }
    else
    {
        write(1, hex + number, 1);
    }
}

/**
 * Retrieves the global symbol if the given symbol is of global bind.
 *
 * @param symbol The symbol to check.
 * @param c The character to modify.
 * @return The modified character based on the symbol bind.
 */
char get_global_symbol(Symbol symbol, char c)
{
    return symbol.bind == STB_GLOBAL ? (char)(c - ('a' - 'A')) : c;
}
/**
 * Retrieves the symbol character based on the symbol's bind, type, and other conditions.
 *
 * @param symbol The symbol to retrieve the character for.
 * @return The symbol character.
 */
char get_symbol_char(Symbol symbol)
{
    // Check if the symbol is weak and of type object
    if (symbol.bind == STB_WEAK && symbol.type == STT_OBJECT)
    {
        if (symbol.shndx)
            return 'V'; // Weak object symbol with defined section
        return 'v'; // Weak object symbol without defined section
    }
    // Check if the symbol is weak
    if (symbol.bind == STB_WEAK)
    {
        if (symbol.shndx)
            return 'W'; // Weak symbol with defined section
        return 'w'; // Weak symbol without defined section
    }
    // Check if the symbol is of type GNU indirect function
    if (symbol.type == STT_GNU_IFUNC)
        return 'i';
    // Check if the symbol is of type file or if it has no section and defined index
    if (symbol.type == STT_FILE || (!symbol.section[0] && symbol.shndx))
        return get_global_symbol(symbol, 'a'); // Get the global symbol character
    // Check if the symbol's section type is SHT_NOBITS
    if (symbol.section_type == SHT_NOBITS)
        return get_global_symbol(symbol, 'b'); // Get the global symbol character
    // Check if the symbol's section flags indicate merge and strings, or if it has no section flags and defined index
    if (symbol.section_flags == (SHF_MERGE | SHF_STRINGS) || (!symbol.section_flags && symbol.shndx))
        return get_global_symbol(symbol, 'n'); // Get the global symbol character
    // Check if the symbol's section flags contain executable instructions
    if ((symbol.section_flags & SHF_EXECINSTR))
        return get_global_symbol(symbol, 't'); // Get the global symbol character
    // Check if the symbol's section flags indicate write and allocate
    if (symbol.section_flags == (SHF_WRITE | SHF_ALLOC))
        return get_global_symbol(symbol, 'd'); // Get the global symbol character
    // Check if the symbol's section flags contain allocate
    if ((symbol.section_flags & SHF_ALLOC))
        return get_global_symbol(symbol, 'r'); // Get the global symbol character
    // Check if the symbol is of type common
    if (symbol.type == STT_COMMON)
        return 'C';
    // Check if the symbol has no defined index
    if (!symbol.shndx)
        return get_global_symbol(symbol, 'u'); // Get the global symbol character
    // Return '?' for unknown cases
    return '?';
}

/**
 * Prints symbols based on the provided file and options.
 *
 * @param file The file containing the symbols.
 * @param option The options specifying which symbols to print.
 */
void print_symbols(File *file, Options option)
{
    int n;
    static char type[] = "   ";
    static char zeros[] = "0000000000000000";

    n = 0;
    while (++n < file->symbol_count)
    {
        // Check if the symbol should be printed based on the specified options
        if ((option.undefined && (file->symbols[n].shndx || file->symbols[n].type == STT_FILE || (file->symbols[n].type == STT_NOTYPE && file->symbols[n].bind < STB_WEAK))) ||
            (!option.all && !option.undefined && (!file->symbols[n].original_name[0] || file->symbols[n].type == STT_FILE)) ||
            (option.globals && (file->symbols[n].bind != STB_GLOBAL && file->symbols[n].bind != STB_WEAK)))
        {
            continue; // Skip the symbol if it doesn't meet the printing criteria
        }
        
        if (file->symbols[n].shndx || file->symbols[n].type == STT_FILE)
        {
            // Print the symbol value in hexadecimal format
            write(1, zeros, 8 + (!file->file_type) * 8 - hex_number_len(file->symbols[n].value));
            print_hex_number(file->symbols[n].value);
        }
        else
        {
            // Print spaces if the symbol does not have a value
            write(1, "                ", 8 + (!file->file_type) * 8);
        }

        // Get the symbol character based on the symbol information
        type[1] = get_symbol_char(file->symbols[n]);

        // Write the symbol type and name to the standard output
        write(1, type, 3);
        write(1, file->symbols[n].name, string_length(file->symbols[n].name));
        write(1, "\n", 1);
    }
}
