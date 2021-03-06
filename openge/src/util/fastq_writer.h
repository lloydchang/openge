/*********************************************************************
 *
 * fastq_writer.h: A simple SAM writer.
 * Open Genomics Engine
 *
 * Author: Lee C. Baker, VBI
 * Last modified: 1 July 2012
 *
 *********************************************************************
 *
 * This file is released under the Virginia Tech Non-Commercial 
 * Purpose License. A copy of this license has been provided in 
 * the openge/ directory.
 *
 *********************************************************************/

#ifndef OGE_FASTQWRITER_H
#define OGE_FASTQWRITER_H

#include "file_io.h"
#include <string>
#include <iostream>
#include <fstream>
#include "bam_header.h"
#include "read_stream_writer.h"

#include <map>
#include <string>

class FastqWriter : public ReadStreamWriter
{
public:
    FastqWriter();
    virtual bool open(const std::string& filename, const BamHeader & samHeader);
    virtual void close();
    virtual bool write( const OGERead & al);
    virtual bool is_open() const { return m_open; }
protected:
    typedef struct fast_record {
        std::string qual, seq;
    } fastq_record_t;

    std::ofstream fwd_file, rev_file, orphan_file;
    std::ostream * fwd_stream, * rev_stream, * orphan_stream;
    BamHeader header;
    std::string filename;
    std::map<std::string, fastq_record_t> potential_pairs;

    bool m_open;
};

#endif
