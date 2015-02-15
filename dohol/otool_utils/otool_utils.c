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
#include "stuff/ofile.h"


static void validate_ofile(struct ofile* ofile) {
    assert(ofile);
    assert(ofile->load_commands);
}

static void validate_64(struct ofile* ofile) {
    validate_ofile(ofile);
    assert(ofile->mh64);
    assert(ofile->mh64->cputype == CPU_TYPE_X86_64);
    assert((ofile->mh64->cpusubtype & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_X86_64_ALL);
    
}

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

struct load_command* ou_load_command(struct ofile* ofile, uint32_t cmd) {
    validate_ofile(ofile);
    
    struct load_command* lc = ofile->load_commands;
    for(uint32_t lc_i = 0; lc_i < ofile->mh64->ncmds; lc_i++) {
        if(lc->cmd == cmd)
            return lc;
        lc = (struct load_command*)((char*)lc + lc->cmdsize);
    }
    return NULL;
}

struct segment_command_64* ou_segment_64(struct ofile* ofile, const char* name) {
    validate_64(ofile);
    
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
    validate_64(ofile);
    
    struct segment_command_64* sg = ou_segment_64(ofile, segment_name);
    if(sg != NULL) {
        struct section_64* sc = (struct section_64*)((char*)sg+sizeof(struct segment_command_64));
        
        for(uint32_t sec_i = 0; sec_i < sg->nsects; sec_i++) {
            // strncmp is crucial here. sect name can be as long as 16 bytes and there is not space
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
    validate_64(ofile);
    if(!ou_section_64_valid_addr(section, addr))
        return NULL;
    return ofile->file_addr + section->offset + addr - section->addr;
}

void* ou_map_64_addr(struct ofile* ofile, struct section_64** section, uint64_t addr) {
    validate_64(ofile);
    
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