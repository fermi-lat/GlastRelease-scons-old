//$Header$
// Original author T. Burnett 
#ifndef MERITROOTTUPLE_H
#define MERITROOTTUPLE_H

#include <string>
class Tuple;
class TFile;
class TTree;

/* 
 * @brief Creates a ROOT tree from the merit list
 *
 * @author T. Burnett
 *
 * $Header$
 */

class MeritRootTuple  {

public:
    /// define the associated root tuple and a file to write it too
    MeritRootTuple::MeritRootTuple(Tuple* tuple, std::string filename);
    ~MeritRootTuple();

    /// fill it
    void fill();

    /// ask for number of entries
    int entries()const;

private:
    Tuple* m_tuple;

    /// the ROOT stuff: a file and a tree to put into it
    TTree * m_tree;
    TFile *  m_tf;

};

#endif