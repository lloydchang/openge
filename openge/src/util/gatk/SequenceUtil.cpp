/*********************************************************************
 *
 * SequenceUtil.cpp: Port of GATK's SequenceUtil.
 * Open Genomics Engine
 *
 * Author: Lee C. Baker, VBI
 * Last modified: 9 May 2012
 *
 *********************************************************************
 *
 * This file has been ported from GATK's implementation in Java, and
 * is released under the Virginia Tech Non-Commercial Purpose License.
 * A copy of this license has been provided in  the openge/ directory.
 * 
 * The original file, SequenceUtil.java, was released 
 * under the following license:
 *
 * Copyright (c) 2009 The Broad Institute
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "SequenceUtil.h"
#include "AlignmentBlock.h"
#include "../oge_read.h"

#include <iostream>
#include <vector>
using namespace std;

/**
 * Returns blocks of the read sequence that have been aligned directly to the
 * reference sequence. Note that clipped portions of the read and inserted and
 * deleted bases (vs. the reference) are not represented in the alignment blocks.
 */
vector<AlignmentBlock> getAlignmentBlocks(const OGERead * read) {
    assert(read != NULL);

    vector<AlignmentBlock> alignmentBlocks;
    int readBase = 1;
    int refBase  = read->getPosition();

    vector<CigarOp> cigar = read->getCigarData();
    for (vector<CigarOp>::const_iterator e = cigar.begin(); e != cigar.end(); e++) {
        switch (e->type) {
            case 'H' : break; // ignore hard clips
            case 'P' : break; // ignore pads
            case 'S' : readBase += e->length; break; // soft clip read bases
            case 'N' : refBase += e->length; break;  // reference skip
            case 'D' : refBase += e->length; break;
            case 'I' : readBase += e->length; break;
            case 'M' :
            case '=' :
            case 'X' :
                {
                    const int length = e->length;
                    alignmentBlocks.push_back( AlignmentBlock(readBase, refBase, length));
                    readBase += length;
                    refBase  += length;
                    break;
                }
            default : 
                cerr << "Case statement didn't deal with cigar op: " << e->type << endl;
                break;
        }
    }
    
    return alignmentBlocks;
}

/** Attempts to efficiently compare two bases stored as bytes for equality. */
bool SequenceUtil::basesEqual(char lhs, char rhs) {
    if (lhs == rhs) return true;
    else {
        if (lhs > 90) lhs -= 32;
        if (rhs > 90) rhs -= 32;
    }
    
    return lhs == rhs;
}

/** Returns true if the bases are equal OR if the mismatch cannot be accounted for by
 * bisfulite treatment.  C->T on the positive strand and G->A on the negative strand
 * do not count as mismatches */
bool SequenceUtil::bisulfiteBasesEqual(const bool negativeStrand, const char read, const char reference) {
    
    if (basesEqual(read, reference)) {
        return true;
    }
    
    if (negativeStrand) {
        if (basesEqual(reference, 'G') && basesEqual(read, 'A')) {
            return true;
        }
    }
    else {
        if (basesEqual(reference, 'C') && basesEqual(read, 'T')) {
            return true;
        }
    }
    return false;
}

/**
 * Calculates the number of mismatches between the read and the reference sequence provided.
 *
 * @param referenceBases Array of ASCII bytes that covers at least the the portion of the reference sequence
 * to which read is aligned from getReferenceStart to getReferenceEnd.
 * @param referenceOffset 0-based offset of the first element of referenceBases relative to the start
 * of that reference sequence.
 * @param bisulfiteSequence If this is true, it is assumed that the reads were bisulfite treated
 *      and C->T on the positive strand and G->A on the negative strand will not be counted
 *      as mismatches.
 */
int SequenceUtil::countMismatches(const OGERead * read, const string referenceBases, const int referenceOffset, const bool bisulfiteSequence) {

    int mismatches = 0;
    
    const string readBases = read->getQueryBases();//read.getReadBases();
    
    vector<AlignmentBlock> blocks = getAlignmentBlocks(read);
    for (vector<AlignmentBlock>::iterator block = blocks.begin(); block != blocks.end(); block++) {
        const int readBlockStart = block->getReadStart() - 1;
        const int referenceBlockStart = block->getReferenceStart() - 1 - referenceOffset;
        const int length = block->getLength();
        
        for (int i=0; i<length; ++i) {
            if (!bisulfiteSequence) {
                if (!basesEqual(readBases[readBlockStart+i], referenceBases[referenceBlockStart+i])) {
                    ++mismatches;
                }
            }
            else {
                if (!bisulfiteBasesEqual(read->IsReverseStrand() , readBases[readBlockStart+i], referenceBases[referenceBlockStart+i])) {
                    ++mismatches;
                }
            }
        }
    }
    return mismatches;
}

/**
 * Calculates the for the predefined NM tag from the SAM spec. To the result of
 * countMismatches() it adds 1 for each indel.
 
 * @param referenceOffset 0-based offset of the first element of referenceBases relative to the start
 * of that reference sequence.
 * @param bisulfiteSequence If this is true, it is assumed that the reads were bisulfite treated
 *      and C->T on the positive strand and G->A on the negative strand will not be counted
 *      as mismatches.
 */
int SequenceUtil::calculateSamNmTag(const OGERead * read, const std::string referenceBases, const int referenceOffset, const bool bisulfiteSequence) {
    int samNm = countMismatches(read, referenceBases, referenceOffset, bisulfiteSequence);
    
    vector<CigarOp> cigar = read->getCigarData();
    for (vector<CigarOp>::const_iterator el = cigar.begin(); el != cigar.end(); el++) {
        if (el->type == 'I' || el->type == 'D') {
            samNm += el->length;
        }
    }
    return samNm;
}

/**
 * Calculates the sum of qualities for mismatched bases in the read.
 * @param referenceBases Array of ASCII bytes that covers at least the the portion of the reference sequence
 * to which read is aligned from getReferenceStart to getReferenceEnd.
 * @param referenceOffset 0-based offset of the first element of referenceBases relative to the start
 * of that reference sequence. 
 * @param bisulfiteSequence If this is true, it is assumed that the reads were bisulfite treated
 *      and C->T on the positive strand and G->A on the negative strand will not be counted
 *      as mismatches.
 */
int SequenceUtil::sumQualitiesOfMismatches(const OGERead * read, const std::string referenceBases, const int referenceOffset, const bool bisulfiteSequence) {
    int qualities = 0;
    
    const string readBases = read->getQueryBases();
    const string readQualities = read->getQualities();
    
    if (read->getPosition() <= referenceOffset) {
        assert(read->getPosition() > referenceOffset);
        cerr << "read.getAlignmentStart(" << read->getPosition() << ") <= referenceOffset(" << referenceOffset << ")" << endl;
    }
    
    vector<AlignmentBlock> blocks = getAlignmentBlocks(read);
    for (vector<AlignmentBlock>::iterator block = blocks.begin(); block != blocks.end(); block++) {
        const int readBlockStart = block->getReadStart() - 1;
        const int referenceBlockStart = block->getReferenceStart() - 1 - referenceOffset;
        const int length = block->getLength();
        
        for (int i=0; i<length; ++i) {
            if (!bisulfiteSequence) {
                if (!basesEqual(readBases[readBlockStart+i], referenceBases[referenceBlockStart+i])) {
                    qualities += readQualities[readBlockStart+i];
                }
                
            }
            else {
                if (!bisulfiteBasesEqual(read->IsReverseStrand(), readBases[readBlockStart+i],
                                         referenceBases[referenceBlockStart+i])) {
                    qualities += readQualities[readBlockStart+i];
                }
            }
        }
    }
    
    return qualities;
}