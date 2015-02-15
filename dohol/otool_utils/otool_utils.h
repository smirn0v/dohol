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
#include <stdint.h>

enum ofile_type;
struct ofile;
struct load_command;
struct segment_command_64;
struct section_64;

#ifdef __cplusplus
extern "C" {
#endif

    extern char* ofile_type_description(enum ofile_type type);
    
    extern struct load_command* ou_load_command(struct ofile* ofile, uint32_t cmd);
    extern struct segment_command_64* ou_segment_64(struct ofile* ofile, const char* name);
    extern struct section_64* ou_section_64(struct ofile* ofile, const char* segment_name, const char* section_name);
    extern enum bool ou_section_64_valid_addr(struct section_64* section, uint64_t addr);
    extern void* ou_section_64_map_addr(struct ofile* ofile, struct section_64* section, uint64_t addr);
    extern void* ou_map_64_addr(struct ofile* ofile, struct section_64** section, uint64_t addr);
    
#ifdef __cplusplus
}
#endif

#endif
