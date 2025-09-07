#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_PAD      7
#define EI_NIDENT   16

#define     ELFMAG0 0x7f
#define     ELFMAG1 'E'
#define     ELFMAG2 'L'
#define     ELFMAG3 'F'

#define     TO_LENDIAN32(x) ((((x) >> 24) & 0x000000ff) | (((x) >> 8) & 0x0000ff00) | (((x) << 8) & 0x00ff0000) | (((x) << 24) & 0xff000000))
#define     TO_LENDIAN16(x) ((((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff00))

struct elf_header_t
{
    unsigned char   e_ident[EI_NIDENT];
    uint16_t        e_type;
    uint16_t        e_machine;
    uint32_t        e_version;
    uint32_t        e_entry;
    uint32_t        e_phoff;
    uint32_t        e_shoff;
    uint32_t        e_flags;
    uint16_t        e_ehsize;
    uint16_t        e_phentsize;
    uint16_t        e_phnum;
    uint16_t        e_shentsize;
    uint16_t        e_shnum;
    uint16_t        e_shstrndx; 
};

#define SHN_UNDEF       0
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff

#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

struct elf_section_header_t
{
    uint32_t    sh_name;
    uint32_t    sh_type;
    uint32_t    sh_flags;
    uint32_t    sh_addr;
    uint32_t    sh_offset;
    uint32_t    sh_size;
    uint32_t    sh_link;
    uint32_t    sh_info;
    uint32_t    sh_addralign;
    uint32_t    sh_entsize;
};

#define ELF32_ST_BIND(i)    ((i)>>4)
#define ELF32_ST_TYPE(i)    ((i)&0xf)
#define ELF32_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOPROC  13
#define STB_HIPROC  15

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_LOPROC  13
#define STT_HIPROC  15

struct elf_symbol_t
{
    uint32_t    st_name;
    uint32_t    st_value;
    uint32_t    st_size;
    uint8_t     st_info;
    uint8_t     st_other;
    uint16_t    st_shndx;
};

#endif
