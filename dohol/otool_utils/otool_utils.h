//
//  otool_utils.h
//  dohol
//
//  Created by Alexander Smirnov on 06/02/14.
//  Copyright (c) 2014 smirn0v. All rights reserved.
//

#ifndef dohj_otool_utils_h
#define dohj_otool_utils_h

#include "stuff/bool.h"
#include "stuff/ofile.h"
#include <stdint.h>

#define CPU_TYPE_ARM64 ((cpu_type_t) 16777228)

enum ofile_type;
struct ofile;
struct load_command;
struct segment_command_64;
struct section_64;


#ifdef __cplusplus
extern "C" {
#endif

    extern char* ofile_type_description(enum ofile_type type);
    
    extern char* ou_human_readable_archname(cpu_type_t cputype, cpu_subtype_t cpusubtype);
    
    // number of load commands
    extern uint32_t ou_ncmds(struct ofile* ofile);
    
    extern char* ou_human_readable_load_command_name(uint32_t cmd);
    
    extern char* ou_load_command_short_description(struct ofile* ofile, struct load_command* load_command);
    
    extern struct segment_command_64* ou_segment_64(struct ofile* ofile, const char* name);
    extern struct section_64* ou_section_64(struct ofile* ofile, const char* segment_name, const char* section_name);
    extern enum bool ou_section_64_valid_addr(struct section_64* section, uint64_t addr);
    extern void* ou_section_64_map_addr(struct ofile* ofile, struct section_64* section, uint64_t addr);
    extern void* ou_map_64_addr(struct ofile* ofile, struct section_64** section, uint64_t addr);
    
#ifdef __cplusplus
}
#endif

#endif
