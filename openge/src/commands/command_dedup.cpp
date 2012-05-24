//
//  command_dedup.cpp
//  OpenGE
//
//  Created by Lee Baker on 4/14/12.
//  Copyright (c) 2012 LCB. All rights reserved.
//

#include "commands.h"
#include "../util/picard_structures.h"
#include "../algorithms/mark_duplicates.h"
#include "../algorithms/file_writer.h"
#include "../algorithms/file_reader.h"
#include <iostream>

#include <api/BamMultiReader.h>
#include <api/BamWriter.h>
using namespace BamTools;
namespace po = boost::program_options;
using namespace std;

void DedupCommand::getOptions()
{
    options.add_options()
    ("out,o", po::value<string>()->default_value("stdout"), "Output filename. Omit for stdout.")
    ("remove,r", "Remove duplicates")
    ;
}

int DedupCommand::runCommand()
{
    FileReader reader;
    MarkDuplicates mark_duplicates;
    FileWriter writer;

    reader.addSink(&mark_duplicates);
    mark_duplicates.addSink(&writer);

    reader.addFiles(input_filenames);

    writer.setFilename(vm["out"].as<string>());

    mark_duplicates.removeDuplicates = vm.count("remove") > 0;
    mark_duplicates.verbose = verbose;
    
    
    char filename[48];
    sprintf(filename, "/dedup_%8x.bam",  (uint32_t)(0xffffffff & (uint64_t)this));
    mark_duplicates.setBufferFileName(tmpdir + string(filename));

    if(input_filenames.size() != 1)
    {
        cerr << "One input file is required. You supplied " << input_filenames.size() << endl;
        return -1;
    }

    reader.runChain();
    
    return 0;
}