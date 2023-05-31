#pragma once

#include "nm.h"

#pragma once

#include "nm.h"

/**
 * Checks the file data for 32-bit ELF files.
 *
 * @param file The file to check.
 * @param name The name of the file.
 * @return 1 if the file data is valid, 0 otherwise.
 */
int check_file_data_32(File *file, char *name)
{
    Elf32_Ehdr *elfHeader;
    Elf32_Shdr *sectionHeader;

    // Retrieve the ELF header and section header
    elfHeader = (Elf32_Ehdr *)file->elf_header;
    if (elfHeader->e_shoff + elfHeader->e_shnum * elfHeader->e_shentsize > file->file_size)
        return file_errors(": ", name, ": File format not recognized\n");

    // Set the section header and string table pointers
    file->section_header = (void *)elfHeader + elfHeader->e_shoff;
    sectionHeader = (Elf32_Shdr *)file->section_header;
    file->section_string_table = (void *)file->elf_header + sectionHeader[elfHeader->e_shstrndx].sh_offset;
    file->string_table = (void *)file->elf_header + sectionHeader[elfHeader->e_shstrndx - 1].sh_offset;
    file->symbol_table = (void *)file->elf_header + sectionHeader[elfHeader->e_shstrndx - 2].sh_offset;

    // Check if the symbol table exists
    if (string_compare(&file->section_string_table[sectionHeader[elfHeader->e_shstrndx - 2].sh_name], ".symtab"))
        return file_errors(": ", name, ": no symbols\n");

    // Calculate the number of symbols
    file->symbol_count = sectionHeader[elfHeader->e_shstrndx - 2].sh_size / sizeof(Elf32_Sym);

    return 1;
}

/**
 * Retrieves symbols from a 32-bit ELF file.
 *
 * @param file The file containing the symbols.
 * @return 1 if the symbol retrieval is successful, 0 otherwise.
 */
int get_symbols_32(File *file)
{
    Elf32_Ehdr *elfHeader;
    Elf32_Shdr *sectionHeader;
    Elf32_Sym *symbolTable;
    int n;

    // Retrieve the symbol table, ELF header, and section header
    symbolTable = (Elf32_Sym *)file->symbol_table;
    elfHeader = (Elf32_Ehdr *)file->elf_header;
    sectionHeader = (Elf32_Shdr *)file->section_header;

    // Allocate memory for the symbols
    if (!(file->symbols = malloc(sizeof(Symbol) * file->symbol_count)))
        return 0;
    be_zero(file->symbols, sizeof(Symbol) * file->symbol_count);

    // Process each symbol
    for (n = 0; n < file->symbol_count; n++)
    {
        // Set symbol properties
        file->symbols[n].name = &file->string_table[symbolTable[n].st_name];
        file->symbols[n].original_name = file->symbols[n].name;
        file->symbols[n].bind = ELF32_ST_BIND(symbolTable[n].st_info);
        file->symbols[n].type = ELF32_ST_TYPE(symbolTable[n].st_info);
        file->symbols[n].value = symbolTable[n].st_value;
        file->symbols[n].shndx = symbolTable[n].st_shndx;

        // Set symbol section properties
        if (symbolTable[n].st_shndx < elfHeader->e_shnum)
        {
            file->symbols[n].section = &file->section_string_table[sectionHeader[symbolTable[n].st_shndx].sh_name];
            file->symbols[n].section_type = sectionHeader[symbolTable[n].st_shndx].sh_type;
            file->symbols[n].section_flags = sectionHeader[symbolTable[n].st_shndx].sh_flags;
        }
        else
        {
            // If the symbol's section index is out of range, set the section name to the first section
            file->symbols[n].section = &file->section_string_table[sectionHeader[0].sh_name];
        }

        // If the symbol name is empty, use the section name as the symbol name
        if (!file->symbols[n].name[0])
            file->symbols[n].name = file->symbols[n].section;
    }

    return 1;
}
