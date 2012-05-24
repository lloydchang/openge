#include "commands.h"

#include <vector>
#include <string>
using namespace std;

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((size_t) -1)
#endif

#include "../algorithms/file_reader.h"
#include "../algorithms/file_writer.h"
#include "../algorithms/filter.h"
using namespace BamTools;
namespace po = boost::program_options;

void ViewCommand::getOptions()
{
    options.add_options()
    ("out,o", po::value<string>()->default_value("stdout"), "Output filename. Omit for stdout.")
    ("count,n", po::value<size_t>(), "Number of alignments to copy")
    ("region,r", po::value<string>(), "Genomic region to use.");
}

int ViewCommand::runCommand()
{
    string filename_in = input_filenames[0];
    string filename_out = vm["out"].as<string>();
    
    if(input_filenames.size() > 1)
        cerr << "More than one input filename provided - only using " << filename_in << "." << endl;

    FileReader reader;
    Filter filter;
    FileWriter writer;
    reader.addSink(&filter);
    filter.addSink(&writer);
    
    if(vm.count("count")) {
        size_t count_limit = vm["count"].as<size_t>();
        filter.setCountLimit(count_limit);
    }

    bool hasregion = vm.count("region") != 0;

    if(hasregion) {
        std::string region_string = vm["region"].as<string>();
        filter.setRegion(region_string);
    }
    
    reader.addFiles(input_filenames);
    writer.setFilename(filename_out);
    
    reader.runChain();
    
    return 0;
}