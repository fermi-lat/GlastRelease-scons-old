
#ifndef TABLELIST_H
#define TABLELIST_H

#include "fx.h"

#include "FXCheckList.h"

#include "rdbModel/Management/Visitor.h"
#include "rdbModel/Rdb.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Column.h"


class TableColumnList: public FXVerticalFrame, public rdbModel::Visitor
{
  FXDECLARE(TableColumnList)
 public:
  enum{
    ID_TBLLIST=FXVerticalFrame::ID_LAST,
    ID_COLLIST
  }; 

  TableColumnList(FXComposite *);

  long onSelectTable(FXObject*, FXSelector, void*);
  long onSelectColumn(FXObject*, FXSelector, void*);
  
 protected:
  TableColumnList(){}
  TableColumnList(TableColumnList&){} 
  
 private:
 
  FXList *m_tblList;                                // List of database tables
  FXCheckList *m_colList;                           // List of comlumns for the selected table
  bool m_tableSelected;                             // True if a table has been selected 
  
  
  /// Visitors to build the table and column lists  
  rdbModel::Visitor::VisitorState visitRdb(rdbModel::Rdb *rdb);
  rdbModel::Visitor::VisitorState visitTable(rdbModel::Table *table);
  rdbModel::Visitor::VisitorState visitColumn(rdbModel::Column *column);
  rdbModel::Visitor::VisitorState visitIndex(rdbModel::Index *index);
  rdbModel::Visitor::VisitorState visitAssertion(rdbModel::Assertion *assertion);
  
};


#endif
