// $Header$

#ifndef RDBMODEL_BUILDER_H
#define RDBMODEL_BUILDER_H
#include <string>

namespace rdbModel{
  class Rdb;

  /**
   * This class defines the public interface for the generic
   * functionalities of a builder. Each subclass must provide
   * implementation for its methods. 
   * @author J. Bogart,
   *  but basically stolen from detModel Builder
   * Many thanks to R. Giannitrapani and D. Favretto
   */
  class Builder {
  public:
  
    Builder() : m_currentRdb(NULL) {}
    virtual ~Builder() {}
    
    /** This method parses the input into an intermediate in-memeory form.
     *  For now, the only acceptable input source is an xml file and the
     *  intermediate form is its DOM represenation.  Conceivably
     *  could accept other forms someday.
     *  Return is to be interpreted as error code:  0 is ok.
     */
    virtual unsigned int parseInput(const std::string&) = 0;

    /** This method start the parser (implemented in a concrete
     *  subclass of this class) and puts the result in a private pointer
     *  Return is number of tables built.  If negative, indicates
     *  error.
     */
    virtual int buildRdb(rdbModel::Rdb* rdb=0) = 0;


    void setRdb(Rdb* pRdb){m_currentRdb = pRdb;}

    /** This method return a pointer to the internal
        Rdb                  
    */
    Rdb * getRdb() { return m_currentRdb; };

  protected:
    /** This is a private pointer to an Rdb used
        during the building */
    Rdb * m_currentRdb;
  };

}
#endif //BUILDER_H

