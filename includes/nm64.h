#pragma once

#include "nm.h"

/**
 * Checks the file data for 64-bit ELF files.
 *
 * @param file The file to check.
 * @param name The name of the file.
 * @return 1 if the file data is valid, 0 otherwise.
 */
int check_file_data_64(File *file, char *name)
{
    // Check if the section header table exceeds the file size
    if (file->elf_header->e_shoff + file->elf_header->e_shnum * file->elf_header->e_shentsize > file->file_size)
        return file_errors(": ", name, ": File format not recognized\n");

    // Set the section header, section string table, string table, and symbol table pointers
    file->section_header = (void *)file->elf_header + file->elf_header->e_shoff;
    file->section_string_table = (void *)file->elf_header + file->section_header[file->elf_header->e_shstrndx].sh_offset;
    file->string_table = (void *)file->elf_header + file->section_header[file->elf_header->e_shstrndx - 1].sh_offset;
    file->symbol_table = (void *)file->elf_header + file->section_header[file->elf_header->e_shstrndx - 2].sh_offset;

    // Check if the symbol table exists
    if (string_compare(&file->section_string_table[file->section_header[file->elf_header->e_shstrndx - 2].sh_name], ".symtab"))
        return file_errors(": ", name, ": no symbols\n");

    // Calculate the number of symbols
    file->symbol_count = file->section_header[file->elf_header->e_shstrndx - 2].sh_size / sizeof(Elf64_Sym);

    return 1;
}

/**
 * Retrieves symbols from a 64-bit ELF file.
 *
 * @param file The file containing the symbols.
 * @return 1 if the symbol retrieval is successful, 0 otherwise.
 */
int get_symbols_64(File *file)
{
    // Allocate memory for the symbols
    if (!(file->symbols = malloc(sizeof(Symbol) * file->symbol_count)))
        return 0;
    be_zero(file->symbols, sizeof(Symbol) * file->symbol_count);

    // Process each symbol
    for (int n = 0; n < file->symbol_count; n++)
    {
        // Set symbol properties
        file->symbols[n].name = &file->string_table[file->symbol_table[n].st_name];
        file->symbols[n].original_name = file->symbols[n].name;
        file->symbols[n].bind = ELF64_ST_BIND(file->symbol_table[n].st_info);
        file->symbols[n].type = ELF64_ST_TYPE(file->symbol_table[n].st_info);
        file->symbols[n].value = file->symbol_table[n].st_value;
        file->symbols[n].shndx = file->symbol_table[n].st_shndx;

        // Set symbol section properties
        if (file->symbol_table[n].st_shndx < file->elf_header->e_shnum)
        {
            file->symbols[n].section = &file->section_string_table[file->section_header[file->symbol_table[n].st_shndx].sh_name];
            file->symbols[n].section_type = file->section_header[file->symbol_table[n].st_shndx].sh_type;
            file->symbols[n].section_flags = file->section_header[file->symbol_table[n].st_shndx].sh_flags;
        }
        else
        {
            // If the symbol's section index is out of range, set the section name to the first section
            file->symbols[n].section = &file->section_string_table[file->section_header[0].sh_name];
        }

        // If the symbol name is empty, use the section name as the symbol name
        if (!file->symbols[n].name[0])
            file->symbols[n].name = file->symbols[n].section;
    }

    return 1;
}
