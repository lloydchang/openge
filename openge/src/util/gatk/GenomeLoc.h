/*********************************************************************
 *
 * GenomeLoc.h: Port of GATK's GenomeLoc.
 * Open GenomeLocParser Engine
 *
 * Author: Lee C. Baker, VBI
 * Last modified: 6 May 2012
 *
 *********************************************************************
 *
 * This file has been ported from GATK's implementation in Java, and
 * is released under the Virginia Tech Non-Commercial Purpose License.
 * A copy of this license has been provided in  the openge/ directory.
 * 
 * The original file, GenomeLoc.java, was released 
 * under the following license:
 *
 * Copyright (c) 2010 The Broad Institute
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


/**
 * Created by IntelliJ IDEA.
 * User: mdepristo
 * Date: Mar 2, 2009
 * Time: 8:50:11 AM
 *
 * Genome location representation.  It is *** 1 *** based closed.  Note that GenomeLocs start and stop values
 * can be any positive or negative number, by design.  Bound validation is a feature of the GenomeLocParser,
 * and not a fundamental constraint of the GenomeLoc
 */

#ifndef GENOMELOC_H
#define GENOMELOC_H

#include <string>
#include <vector>
#include <limits.h>
class GenomeLoc {
    /**
     * the basic components of a genome loc, its contig index,
     * start and stop position, and (optionally) the contig name
     */
protected:
    std::string contigName;
    int contigIndex;
    int start;
    int stop;
    
    /**
     * A static constant to use when referring to the unmapped section of a datafile
     * file.  The unmapped region cannot be subdivided.  Only this instance of
     * the object may be used to refer to the region, as '==' comparisons are used
     * in comparators, etc.
     */
    // TODO - WARNING WARNING WARNING code somehow depends on the name of the contig being null!
    public:
    static GenomeLoc UNMAPPED;
    static GenomeLoc WHOLE_GENOME;
    
    static bool isUnmapped(GenomeLoc loc) {
        return loc == UNMAPPED;
    }
    
    // --------------------------------------------------------------------------------------------------------------
    //
    // constructors
    //
    // --------------------------------------------------------------------------------------------------------------
    
    //@Requires({
    //    "contig != null",
    //    "contigIndex >= 0", // I believe we aren't allowed to create GenomeLocs without a valid contigIndex
    //    "start <= stop"})
public:
    GenomeLoc( const std::string contig, int contigIndex, int start, int stop );
private:
    GenomeLoc( const std::string contig );  
public:

    std::string getContig() const { return contigName; }
    int getContigIndex() const { return contigIndex; }
    int getStart() const    { return start; }
    int getStop() const     { return stop; }

    std::string toString() const;
    
    GenomeLoc getLocation() { return *this; }
    GenomeLoc getStartLocation() const { return GenomeLoc(getContig(),getContigIndex(),getStart(),getStart()); }
    GenomeLoc getStopLocation() const { return GenomeLoc(getContig(),getContigIndex(),getStop(),getStop()); }
    
private:
    bool throughEndOfContigP() const { return stop == INT_MAX; }
    bool atBeginningOfContigP() const { return start == 1; }
    
    //@Requires("that != null")
public:
    bool disjointP(const GenomeLoc & that) const {
        return contigIndex != that.contigIndex || start > that.stop || that.start > stop;
    }
    
    //@Requires("that != null")
    bool discontinuousP(GenomeLoc that) const {
        return contigIndex != that.contigIndex || (start - 1) > that.stop || (that.start - 1) > stop;
    }
    
    //@Requires("that != null")
    bool overlapsP(const GenomeLoc & that) const {
        return ! disjointP( that );
    }
    
    //@Requires("that != null")
    bool contiguousP(const GenomeLoc & that) const {
        return ! discontinuousP( that );
    }

    GenomeLoc merge( const GenomeLoc & that );    
    GenomeLoc endpointSpan(const GenomeLoc & that);    
    std::pair<GenomeLoc, GenomeLoc> split( int splitPoint) const;
    GenomeLoc Union( const GenomeLoc & that ) { return merge(that); }
    GenomeLoc intersect( const GenomeLoc & that ) const;
    
    std::vector<GenomeLoc> subtract( const GenomeLoc & that );

public:
    bool containsP(const GenomeLoc & that) const;
    bool onSameContig(const GenomeLoc & that) const;
    int distance( const GenomeLoc & that ) const;
    bool isBetween( const GenomeLoc & left, const GenomeLoc & right );
    bool isBefore( GenomeLoc that ) const;    
    bool startsBefore(const GenomeLoc & that);    

public:
    bool isPast( const GenomeLoc & that );
    int minDistance( const GenomeLoc & that );    
private:
    static int distanceFirstStopToSecondStart(GenomeLoc locFirst, GenomeLoc locSecond);
public:
    int hashCode();
    int compareContigs( GenomeLoc that ) const;

    int compareTo( const GenomeLoc & that )const;

    bool endsAt(GenomeLoc that) {
        return (compareContigs(that) == 0) && ( getStop() == that.getStop() );
    }

    int size() const {
        return stop - start + 1;
    }

    double reciprocialOverlapFraction(const GenomeLoc & o);
    
private:
    static double overlapPercent(const GenomeLoc & gl1, const GenomeLoc & gl2) {
        return (1.0 * gl1.intersect(gl2).size()) / gl1.size();
    }
    
public:
    long sizeOfOverlap( const GenomeLoc & that ) const;
    
public:
    bool operator<(const GenomeLoc & o) const {
        if(start != o.start)
            return start < o.start;
        if(stop != o.stop)
            return stop < o.stop;
        if(contigIndex != o.contigIndex)
            return contigIndex < o.contigIndex;
        return contigName < o.contigName;
    }
    
    bool operator==(const GenomeLoc & o) const {
        return start == o.start && stop == o.stop && contigIndex == o.contigIndex && contigName == o.contigName;
    }
    
    bool operator!=(const GenomeLoc & o) const { return !(*this == o); }
};

#endif
