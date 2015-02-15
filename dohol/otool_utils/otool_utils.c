//
//  otool_utils.c
//  dohol
//
//  Created by Alexander Smirnov on 06/02/14.
//  Copyright (c) 2014 smirn0v. All rights reserved.
//

#include "otool_utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct cmd_description {
    uint32_t cmd;
    char* name;
    char* desc;
};

static const struct cmd_description cmds_description[] = {
    {.cmd = LC_SEGMENT,              .name = "lc_segment",              .desc = "segment of this file to be mapped" },
    {.cmd = LC_SYMTAB,               .name = "lc_symtab",               .desc = "link-edit stab symbol table info" },
    {.cmd = LC_SYMSEG,               .name = "lc_symseg",               .desc = "link-edit gdb symbol table info (obsolete)"},
    {.cmd = LC_THREAD,               .name = "lc_thread",               .desc = "thread"},
    {.cmd = LC_UNIXTHREAD,           .name = "lc_unixthread",           .desc = "unix thread (includes a stack)"},
    {.cmd = LC_LOADFVMLIB,           .name = "lc_loadfvmlib",           .desc = "load a specified fixed VM shared library"},
    {.cmd = LC_IDFVMLIB,             .name = "lc_idfvmlib",             .desc = "fixed VM shared library identification"},
    {.cmd = LC_IDENT,                .name = "lc_ident",                .desc = "object identification info (obsolete)"},
    {.cmd = LC_FVMFILE,              .name = "lc_fvmfile",              .desc = "fixed VM file inclusion (internal use)"},
    {.cmd = LC_PREPAGE,              .name = "lc_prepage",              .desc = "prepage command (internal use)"},
    {.cmd = LC_DYSYMTAB,             .name = "lc_dysymtab",             .desc = "dynamic link-edit symbol table info"},
    {.cmd = LC_LOAD_DYLIB,           .name = "lc_load_dylib",           .desc = "load a dynamically linked shared library"},
    {.cmd = LC_ID_DYLIB,             .name = "lc_id_dylib",             .desc = "dynamically linked shared lib ident"},
    {.cmd = LC_LOAD_DYLINKER,        .name = "lc_load_dylinker",        .desc = "load a dynamic linker"},
    {.cmd = LC_ID_DYLINKER,          .name = "lc_id_dylinker",          .desc = "dynamic linker identification"},
    {.cmd = LC_PREBOUND_DYLIB,       .name = "lc_prebound_dylib",       .desc = "modules prebound for a dynamically linked shared library"},
    {.cmd = LC_ROUTINES,             .name = "lc_routines",             .desc = "image routines"},
    {.cmd = LC_SUB_FRAMEWORK,        .name = "lc_sub_framework",        .desc = "sub framework"},
    {.cmd = LC_SUB_UMBRELLA,         .name = "lc_sub_umbrella",         .desc = "sub umbrella"},
    {.cmd = LC_SUB_CLIENT,           .name = "lc_sub_client",           .desc = "sub client"},
    {.cmd = LC_SUB_LIBRARY,          .name = "lc_sub_library",          .desc = "sub library"},
    {.cmd = LC_TWOLEVEL_HINTS,       .name = "lc_twolevel_hints",       .desc = "two-level namespace lookup hints"},
    {.cmd = LC_PREBIND_CKSUM,        .name = "lc_prebind_cksum",        .desc = "prebind checksum"},
    {.cmd = LC_LOAD_WEAK_DYLIB,      .name = "lc_load_weak_dylib",      .desc = "load a dynamically linked shared library that is allowed to be missing (all symbols are weak imported)"} ,
    {.cmd = LC_SEGMENT_64,           .name = "lc_segment_64",           .desc = "64-bit segment of this file to be mapped"},
    {.cmd = LC_ROUTINES_64,          .name = "lc_routines_64",          .desc = "64-bit image routines"},
    {.cmd = LC_UUID,                 .name = "lc_uuid",                 .desc = "the uuid"},
    {.cmd = LC_RPATH,                .name = "lc_rpath",                .desc = "runpath additions"},
    {.cmd = LC_CODE_SIGNATURE,       .name = "lc_code_signature",       .desc = "local of code signature"},
    {.cmd = LC_SEGMENT_SPLIT_INFO,   .name = "lc_segment_split_info",   .desc = "local of info to split segments"},
    {.cmd = LC_REEXPORT_DYLIB,       .name = "lc_reexport_dylib",       .desc = "load and re-export dylib"},
    {.cmd = LC_LAZY_LOAD_DYLIB,      .name = "lc_lazy_load_dylib",      .desc = "delay load of dylib until first use"},
    {.cmd = LC_ENCRYPTION_INFO,      .name = "lc_encryption_info",      .desc = "encrypted segment information"},
    {.cmd = LC_DYLD_INFO,            .name = "lc_dyld_info",            .desc = "compressed dyld information"},
    {.cmd = LC_DYLD_INFO_ONLY,       .name = "lc_dyld_info_only",       .desc = "compressed dyld information only"},
    {.cmd = LC_LOAD_UPWARD_DYLIB,    .name = "lc_load_upward_dylib",    .desc = "load upward dylib"},
    {.cmd = LC_VERSION_MIN_MACOSX,   .name = "lc_version_min_macosx",   .desc = "build for MacOSX min OS version"},
    {.cmd = LC_VERSION_MIN_IPHONEOS, .name = "lc_version_min_iphoneos", .desc = "build for iPhoneOS min OS version"},
    {.cmd = LC_FUNCTION_STARTS,      .name = "lc_function_starts",      .desc = "compressed table of function start addresses"},
    {.cmd = LC_DYLD_ENVIRONMENT,     .name = "lc_dyld_environment",     .desc = "string for dyld to treat like environment variable"},
    {.cmd = LC_MAIN,                 .name = "lc_main",                 .desc = "replacement for LC_UNIXTHREAD"},
    {.cmd = LC_DATA_IN_CODE,         .name = "lc_data_in_code",         .desc = "table of non-instructions in __text"},
    {.cmd = LC_SOURCE_VERSION,       .name = "lc_source_version",       .desc = "source version used to build binary"},
    {.cmd = LC_DYLIB_CODE_SIGN_DRS,  .name = "lc_dylib_code_sign_drs",  .desc = "Code signing DRs copied from linked dylibs"},
    {.cmd = LC_ENCRYPTION_INFO_64,   .name = "lc_encryption_info_64",   .desc = "64-bit encrypted segment information"},
    {.cmd = LC_LINKER_OPTION,        .name = "lc_linker_option",        .desc = "linker options in MH_OBJECT files"}
};

static const struct arch_flag supported_arch_flags[] = {
    {.name = "x86_64", .cputype = CPU_TYPE_X86_64, .cpusubtype = CPU_SUBTYPE_X86_64_ALL},
    {.name = "i386",   .cputype = CPU_TYPE_I386,   .cpusubtype = CPU_SUBTYPE_I386_ALL},
    {.name = "armv7",  .cputype = CPU_TYPE_ARM,    .cpusubtype = CPU_SUBTYPE_ARM_V7},
    {.name = "armv7s", .cputype = CPU_TYPE_ARM,    .cpusubtype = CPU_SUBTYPE_ARM_V7S},
    {.name = "arm64",  .cputype = CPU_TYPE_ARM64,  .cpusubtype = CPU_SUBTYPE_ARM_ALL},
};

char* ofile_type_description(enum ofile_type type) {
    switch(type) {
        case OFILE_FAT: return "FAT";
        case OFILE_ARCHIVE: return "Archive";
        case OFILE_Mach_O: return "Mach-O";
#ifdef LTO_SUPPORT
        case OFILE_LLVM_BITCODE: return "LLVM Bitcode";
#endif
        default:
            return "Unknown";
    }
}

char* ou_human_readable_archname(cpu_type_t cputype, cpu_subtype_t cpusubtype) {
    for(uint8_t i = 0; i < sizeof(supported_arch_flags)/sizeof(struct arch_flag); i++) {
        enum bool type_match = supported_arch_flags[i].cputype == cputype;
        enum bool subtype_match = (cpusubtype & ~CPU_SUBTYPE_MASK) == supported_arch_flags[i].cpusubtype;
        if(type_match && subtype_match) {
            return supported_arch_flags[i].name;
        }
    }
    
    static char archname[256];
    memset(archname, 0, sizeof(archname));
    snprintf(archname, sizeof(archname)-1, "cpu_type: %d, cpu_subtype: %d", cputype, cpusubtype);
    return archname;
}

uint32_t ou_ncmds(struct ofile* ofile) {
    if(ofile->mh64) {
        return ofile->mh64->ncmds;
    }
    if(ofile->mh) {
        return ofile->mh->ncmds;
    }
    return 0;
}

char* ou_human_readable_load_command_name(uint32_t cmd) {
    static char load_command_name[256];
    memset(load_command_name, 0, sizeof(load_command_name));
    
    for(uint8_t i = 0; i < sizeof(cmds_description)/sizeof(struct cmd_description); i++) {
        if(cmds_description[i].cmd == cmd) {
            snprintf(load_command_name, sizeof(load_command_name)-1, "%32s", cmds_description[i].name);
            return load_command_name;
        }
    }
    
    snprintf(load_command_name, sizeof(load_command_name)-1, "unknown_load_command(0x%02x)", cmd);
    return load_command_name;
}

char* ou_load_command_short_description(struct ofile* ofile, struct load_command* load_command) {
    static char description[256];
    memset(description, 0, sizeof(description));
    
    uint32_t file_offset = 0;
    if(ofile->file_type == OFILE_FAT) {
        struct fat_arch *fat_archs = ofile->fat_archs + ofile->narch;
        file_offset = fat_archs->offset;
    }
    
    switch(load_command->cmd) {
        case LC_SEGMENT: {
            struct segment_command* segment_command = (struct segment_command*)load_command;
            snprintf(description, sizeof(description)-1, "segname: %16.16s  |  abs_file_offset: %10u  |  file_size: %10u  |  vmaddr: 0x%08x  |  vmsize: 0x%08x  |  vmsize: %10u", segment_command->segname, segment_command->fileoff+file_offset, segment_command->filesize, segment_command->vmaddr, segment_command->vmsize, segment_command->vmsize);
        }
            break;
        case LC_SEGMENT_64: {
            struct segment_command_64* segment_command = (struct segment_command_64*)load_command;
            snprintf(description, sizeof(description)-1, "segname: %16.16s  |  abs_file_offset: %10llu  |  file_size: %10llu  |  vmaddr: 0x%.16llx  |  vmsize: 0x%08llx  |  vmsize: %10llu", segment_command->segname, segment_command->fileoff+file_offset, segment_command->filesize, segment_command->vmaddr, segment_command->vmsize, segment_command->vmsize);
        }
            break;
        case LC_LOAD_DYLIB: {
            struct dylib_command* dylib_command = (struct dylib_command*)load_command;
            snprintf(description, sizeof(description)-1, "%s", (char*)dylib_command+dylib_command->dylib.name.offset);
        }
            break;
        case LC_DATA_IN_CODE: {
            struct data_in_code_entry* data_in_code = (struct data_in_code_entry*)load_command;
            snprintf(description, sizeof(description)-1, "offset: %10u  |  length: %10u  |  kind: %10u", data_in_code->offset+file_offset, data_in_code->length, data_in_code->kind);
        }
            break;
    }
    
    return description;
}

enum bool ou_is_segment_load_command(struct load_command* load_command) {
    return load_command->cmd == LC_SEGMENT || load_command->cmd == LC_SEGMENT_64;
}

struct segment_command_64* ou_segment_64(struct ofile* ofile, const char* name) {

    
    struct load_command* lc = ofile->load_commands;
    for(uint32_t seg_i = 0; seg_i < ofile->mh64->ncmds; seg_i++) {
        struct segment_command_64* sg= (struct segment_command_64*)lc;

        if(sg->cmd == LC_SEGMENT_64 && strcmp(sg->segname, name) == 0)
            return sg;
        
        lc = (struct load_command*)((char*)lc + lc->cmdsize);
    }
           
    return NULL;
}

struct section_64* ou_section_64(struct ofile* ofile, const char* segment_name, const char* section_name) {
    
    struct segment_command_64* sg = ou_segment_64(ofile, segment_name);
    if(sg != NULL) {
        struct section_64* sc = (struct section_64*)((char*)sg+sizeof(struct segment_command_64));
        
        for(uint32_t sec_i = 0; sec_i < sg->nsects; sec_i++) {
            // strncmp is crucial here. sect name can be as long as 16 bytes and there is no space
            // left for '\0'
            if(strncmp(sc->sectname, section_name, sizeof(sc->sectname)) == 0)
                return sc;
            
            sc = (struct section_64*)((char*)sc + sizeof(struct section_64));
        }
    }
    return NULL;
}

enum bool ou_section_64_valid_addr(struct section_64* section, uint64_t addr) {
    return addr >= section->addr && addr < (section->addr + section->size);
}

void* ou_section_64_map_addr(struct ofile* ofile, struct section_64* section, uint64_t addr) {
    if(!ou_section_64_valid_addr(section, addr))
        return NULL;
    return ofile->file_addr + section->offset + addr - section->addr;
}

void* ou_map_64_addr(struct ofile* ofile, struct section_64** section, uint64_t addr) {
    struct section_64* dumb;
    if(!section)
        section = &dumb;
   
    struct load_command* lc = ofile->load_commands;
    for(uint32_t seg_i = 0; seg_i < ofile->mh64->ncmds; seg_i++) {
        struct segment_command_64* sg= (struct segment_command_64*)lc;
        
        if(sg->cmd == LC_SEGMENT_64) {
            
            struct section_64* sc = (struct section_64*)((char*)sg+sizeof(struct segment_command_64));
            
            for(uint32_t sec_i = 0; sec_i < sg->nsects; sec_i++) {
                
                if(ou_section_64_valid_addr(sc, addr)) {
                    *section = sc;
                    return ou_section_64_map_addr(ofile, sc, addr);
                }
                
                sc = (struct section_64*)((char*)sc + sizeof(struct section_64));
            }
            
        }
        
        lc = (struct load_command*)((char*)lc + lc->cmdsize);
    }
    return NULL;
}