#include "includes/nm.h"
#include "includes/nm64.h"
#include "includes/nm32.h"

/**
 * Parses the command line flags and updates the options accordingly.
 *
 * @param option The options structure to update.
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 */
void parse_flags(Options *option, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j] != '\0'; j++)
            {
                char flag = argv[i][j];
                switch (flag)
                {
                case 'a':
                    option->all = 1;
                    break;
                case 'g':
                    option->globals = 1;
                    break;
                case 'p':
                    option->not_sorted = 1;
                    break;
                case 'r':
                    option->reverse = 1;
                    break;
                case 'u':
                    option->undefined = 1;
                    break;
                default:
                    write(2, "ft_nm: invalid option", 21);
                    break;
                }
            }
        }
    }
}

/**
 * Retrieves file data for a given file.
 *
 * @param file The File structure to store the file data.
 * @param name The name of the file.
 * @return 1 if the file data retrieval is successful, 0 otherwise.
 */
int get_file_data(File *file, char *name)
{
    struct stat file_stats;
    MagicNumber *magic;

    // Open the file
    if ((file->file_descriptor = open(name, O_RDONLY)) <= 0)
        return file_errors(": '", name, ":' No such file\n");

    // Get file stats
    if (fstat(file->file_descriptor, &file_stats))
        return file_errors(": '", name, ":' No such file\n");

    // Check if the file is a regular file
    if (!S_ISREG(file_stats.st_mode))
        return file_errors(": Warning: '", name, "' is not an ordinary file\n");

    // Set file size and mmap the file
    file->file_size = file_stats.st_size;
    if (!(file->elf_header = mmap(0, file->file_size, PROT_READ, MAP_PRIVATE, file->file_descriptor, 0)))
        return 0;

    // Close the file descriptor
    close(file->file_descriptor);

    // Check the magic number to determine the file type
    magic = (MagicNumber *)file->elf_header;
    if (magic->magic_number != MAGIC_NUMBER)
        return file_errors(": ", name, ": File format not recognized\n");
    file->file_type = (magic->support == ELF32) ? ELF32 : ELF64;

    return 1;
}

/**
 * Processes a file, including retrieving the file data, checking file data, getting symbols,
 * sorting symbols, and printing symbols.
 *
 * @param filename The name of the file to process.
 * @param options The options to apply during processing.
 * @param multiple_programs Indicates if there are multiple programs being processed.
 * @return 1 if the file is processed successfully, 0 otherwise.
 */
int process_file(char *filename, Options options, bool multiple_programs)
{
    File file = {0};

    // Get file data
    if (!get_file_data(&file, filename))
        return (file_errors(": ", filename, ": No such file or directory\n"));

    // Check file data and retrieve symbols based on file type
    if (file.file_type)
    {
        if (!check_file_data_32(&file, filename))
            return (0);
        get_symbols_32(&file);
    }
    else
    {
        if (!check_file_data_64(&file, filename))
            return (0);
        get_symbols_64(&file);
    }

    // Print file name if there are multiple programs
    if (multiple_programs)
    {
        write(1, "\n", 1);
        write(1, filename, string_length(filename));
        write(1, ":\n", 2);
    }

    // Sort symbols if necessary and print symbols
    if (options.not_sorted == false)
    {
        options.reverse ? reverse_quicksort_symbols(file.symbols + 1, file.symbol_count - 1) : quicksort_symbols(file.symbols + 1, file.symbol_count - 1);
    }
    print_symbols(&file, options);

    // Free memory and cleanup
    free(file.symbols);
    munmap(file.elf_header, file.file_size);

    return (1);
}

/**
 * The main entry point of the program.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 * @return The exit status of the program.
 */
int main(int argc, char **argv)
{
    int arg_index;
    int file_count = 0;
    Options options = {0};

    // Parse command line flags
    parse_flags(&options, argc, argv);

    // Get the number of files
    file_count = get_file_count(argc, argv);

    if (file_count == 0)
    {
        // Process default file "a.out"
        if (!process_file("a.out", options, 0))
            return (EXIT_FAILURE);
    }
    else
    {
        // Process each file
        arg_index = 0;
        while (++arg_index < argc)
        {
            if (argv[arg_index][0] != '-')
            {
                if (!process_file(argv[arg_index], options, file_count > 1))
                    return (EXIT_FAILURE);
            }
        }
    }

    return (EXIT_SUCCESS);
}
