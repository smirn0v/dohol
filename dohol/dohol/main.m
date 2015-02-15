//
//  main.m
//  dohol
//
//  Created by Alexander Smirnov on 15/02/15.
//  Copyright (c) 2015 Alexander Smirnov. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "stuff/ofile.h"
#include "otool_utils.h"

// required by otool
char *progname = "dohl";

void ofile_processor(struct ofile* ofile, char* archname, void* cookie) {
    printf("arch: %s\n", ou_human_readable_archname(ofile->mh_cputype, ofile->mh_cpusubtype));
    
    uint32_t load_commands_count =  ou_ncmds(ofile);
    
    struct load_command* load_command = ofile->load_commands;
    
    for(uint32_t load_cmd_i = 0; load_cmd_i < load_commands_count; load_cmd_i++) {
        printf("%s  %s\n", ou_human_readable_load_command_name(load_command->cmd), ou_load_command_short_description(ofile, load_command));
        
        if(ou_is_segment_load_command(load_command)) {
            
        }
        
        load_command = (struct load_command*)((char*)load_command + load_command->cmdsize);
    }
    printf("\n");
}

int main(int argc, const char * argv[]) {
    
    @autoreleasepool {
        
        ofile_process("/Users/smirnov/crack/A8Cheats.dylib", NULL, 0, true, true, true, true, &ofile_processor, NULL);
        //        ofile_process("/Users/smirnov/projects/a.out", NULL, 0, true, true, true, true, &ofile_processor, NULL);
        return 0;
    }
}