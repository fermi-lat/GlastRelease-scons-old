/** \file Keyword.h

    \brief Helper classes to allow convenient and idiomatic access to ITabularData objects.

    \author James Peachey, HEASARC
*/
#ifndef tip_Keyword_h
#define tip_Keyword_h

#include <string>

#include "tip/IHeaderData.h"
#include "tip/TipException.h"

namespace tip {

  /** \class Keyword

      \brief Encapsulation of a single keyword. The keyword may be get/set in any data type,
  */
  class Keyword {
    public:
      /** \brief Construct a Keyword object associated with a particular Header.
          \param header_data Pointer to the referent IHeaderData object.
          \param name The name of this Keyword.
      */
      Keyword(IHeaderData * header_data, const std::string & name): m_header_data(header_data),
        m_name(name) {
        if (!m_header_data) throw TipException("Keyword::Keyword(IHeaderData *, const std::string &): "
          "Cannot create Keyword with a NULL IHeaderData pointer");
      }

      /** \brief Get the current value of this Keyword.
          The type of the converted value is given by the template parameter.
          \param value The current value.
      */
      template <typename T>
      void get(T & value) const;

      /** \brief Set the value of this Keyword.
          \param value The value to set.
      */
      template <typename T>
      void set(const T & value);

    private:
      IHeaderData * m_header_data;
      std::string m_name;
  };

  template <typename T>
  inline void Keyword::get(T & value) const { m_header_data->getKeyword(m_name, value); }

  template <typename T>
  inline void Keyword::set(const T & value) { m_header_data->setKeyword(m_name, value); }

}

#endif
