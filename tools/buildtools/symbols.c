#include "symbols.h"

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        FILE *symbols_file = fopen("src/boot/symbols.c", "w");

        fprintf(symbols_file, "#include \"fault.h\"\n");
        // fprintf(symbols_file, "struct func_symbol_t gFuncSymbols[] = {\n");

        for(uint32_t index = 1; index < argc; index++)
        {
            FILE *object_file = fopen(argv[index], "rb");
            fseek(object_file, 0, SEEK_END);
            size_t file_size = ftell(object_file);
            rewind(object_file);
            char *object_data = calloc(1, file_size);
            fread(object_data, file_size, 1, object_file);
            fclose(object_file);

            // char *in = object_data;
            struct elf_header_t *header = (struct elf_header_t *)object_data;
            if(header->e_ident[EI_MAG0] == ELFMAG0 && header->e_ident[EI_MAG1] == ELFMAG1 &&
                header->e_ident[EI_MAG2] == ELFMAG2 && header->e_ident[EI_MAG3] == ELFMAG3)
            {
                if(header->e_shentsize == 0 || header->e_shnum == 0)
                {
                    printf("Input file [%s] doesn't have a section table\n", argv[index]);
                }
                else
                {
                    if(TO_LENDIAN16(header->e_shstrndx) == SHN_UNDEF)
                    {
                        printf("Input file [%s] doesn't have a section string table\n", argv[index]);
                    }
                    else
                    {
                        char *section_header_table = object_data + TO_LENDIAN32(header->e_shoff);
                        uint32_t section_header_count = TO_LENDIAN16(header->e_shnum);
                        struct elf_section_header_t *section_string_table_header = section_header_table + TO_LENDIAN16(header->e_shstrndx) * TO_LENDIAN16(header->e_shentsize);
                        char *section_string_table = object_data + TO_LENDIAN32(section_string_table_header->sh_offset);
                        uint32_t symbol_string_table_entry_index = section_header_count;

                        for(uint32_t section_table_entry_index = 0; section_table_entry_index < section_header_count; section_table_entry_index++)
                        {
                            struct elf_section_header_t *section_header = (struct elf_section_header_t *)(section_header_table + TO_LENDIAN16(header->e_shentsize) * section_table_entry_index);    
                            if(section_header->sh_name > 0)
                            {
                                if(!strcmp(".strtab", section_string_table + TO_LENDIAN32(section_header->sh_name)))
                                {
                                    symbol_string_table_entry_index = section_table_entry_index;
                                    break;
                                }
                            }
                        }

                        if(symbol_string_table_entry_index >= section_header_count)
                        {
                            printf("Input file [%s] doesn't have a string table\n", argv[index]);
                        }
                        else
                        {
                            struct elf_section_header_t *string_table_header = section_header_table + symbol_string_table_entry_index * TO_LENDIAN16(header->e_shentsize);
                            char *string_table = object_data + TO_LENDIAN32(string_table_header->sh_offset);

                            for(uint32_t section_table_entry_index = 0; section_table_entry_index < section_header_count; section_table_entry_index++)
                            {
                                struct elf_section_header_t *section_header = (struct elf_section_header_t *)(section_header_table + TO_LENDIAN16(header->e_shentsize) * section_table_entry_index);
                                if(TO_LENDIAN32(section_header->sh_type) == SHT_SYMTAB)
                                {
                                    char *symbol_table = object_data + TO_LENDIAN32(section_header->sh_offset);
                                    uint32_t symbol_count = TO_LENDIAN32(section_header->sh_size) / TO_LENDIAN32(section_header->sh_entsize);
                                    for(uint32_t symbol_index = 0; symbol_index < symbol_count; symbol_index++)
                                    {
                                        struct elf_symbol_t *symbol = (struct elf_symbol_t *)(symbol_table + TO_LENDIAN32(section_header->sh_entsize) * symbol_index);
                                        if(ELF32_ST_TYPE(symbol->st_info) == STT_FUNC)
                                        {
                                            if(symbol->st_name > 0 && TO_LENDIAN32(symbol->st_value) != SHN_UNDEF)
                                            {
                                                char *symbol_name = string_table + TO_LENDIAN32(symbol->st_name);
                                                fprintf(symbols_file, "extern void * %s;\n", symbol_name);
                                                // fprintf(symbols_file, "{(uintptr_t)%s, {\"%s\"}},\n", symbol_name, symbol_name);
                                                // printf("Function: %s\n", string_table + TO_LENDIAN32(symbol->st_name));
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            }
            else
            {
                printf("Input file [%s] is not a valid elf file\n", argv[index]);
            }

            free(object_data);
        }

        // fprintf(symbols_file, "};\n");
    }

    return 0;
}
