#include "ColWidgetFactory.h"
#include "StringWidget.h"
#include "DateWidget.h"
#include "EnumWidget.h"
#include "RealWidget.h"
#include "IntWidget.h"

#include "rdbModel/Tables/Datatype.h"

ColWidget* ColWidgetFactory::createStringWidget(FXComposite* parent,rdbModel::Column *column)
{
  return new StringWidget(parent, column);
}

ColWidget* ColWidgetFactory::createDateWidget(FXComposite* parent, rdbModel::Column *column)
{
  return new DateWidget(parent, column);
}

ColWidget* ColWidgetFactory::createEnumWidget(FXComposite* parent, rdbModel::Column *column)
{
  return new EnumWidget(parent, column);
}

ColWidget* ColWidgetFactory::createRealWidget(FXComposite* parent, rdbModel::Column *column)
{
  return new RealWidget(parent, column);
}

ColWidget* ColWidgetFactory::createIntWidget(FXComposite* parent, rdbModel::Column *column)
{
  return new IntWidget(parent, column);
}

ColWidget* ColWidgetFactory::createColWidget(FXComposite* parent, rdbModel::Column *column)
{
  rdbModel::Datatype* dt = column->getDatatype();
  rdbModel::Column::FROM source = column->getSourceType();

  std::string min;
  std::string max;
    
  ColWidget* colWidget;

  switch(dt->getType()){
  case rdbModel::Datatype::TYPEdatetime :
  {
      colWidget = createDateWidget(parent, column);
      break;
  };
  case rdbModel::Datatype::TYPEtimestamp :
  {
      colWidget = createStringWidget(parent, column);
      break;
  };
  case rdbModel::Datatype::TYPEenum :
  {
      colWidget = createEnumWidget(parent, column);
      break;
  };
  case rdbModel::Datatype::TYPEreal :
  case rdbModel::Datatype::TYPEdouble :
  {
      colWidget = createRealWidget(parent, column);
      break;
  };
  case rdbModel::Datatype::TYPEint :
  case rdbModel::Datatype::TYPEmediumint :
  case rdbModel::Datatype::TYPEsmallint :
  {
      colWidget = createIntWidget(parent, column);
      break;
  };
  case rdbModel::Datatype::TYPEvarchar :
  case rdbModel::Datatype::TYPEchar :
  {
    if (dt->getRestrict() == rdbModel::Datatype::RESTRICTenum)
      colWidget = createEnumWidget(parent, column);
    else
      colWidget = createStringWidget(parent, column);
    break;
  };
  } 

  if ((source == rdbModel::Column::FROMdefault)) 
    colWidget->setValue(column->getDefault());
}
