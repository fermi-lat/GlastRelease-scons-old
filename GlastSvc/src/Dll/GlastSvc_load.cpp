/** 
* @file GlastSvc_load.cpp
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header$
*/

#include "GaudiKernel/DeclareFactoryEntries.h"
// #include "GaudiKernel/IToolFactory.h"

// #define DLL_DECL_TOOL(x)       extern const IToolFactory& x##Factory; x##Factory.addRef();

DECLARE_FACTORY_ENTRIES(GlastSvc) {
    DECLARE_SERVICE( GlastDetSvc);
    DECLARE_SERVICE( GlastPropagatorSvc);
    DECLARE_SERVICE( GlastRandomSvc);
    DECLARE_SERVICE( DbEvtSelector );
    DECLARE_SERVICE( EventCnvSvc );
    DECLARE_CONVERTER( EventCnv );
    DECLARE_CONVERTER( MCEventCnv );
    DECLARE_ALGORITHM( CurrentEventAlg );
    DECLARE_SERVICE( GlastHistorySvc );

       DECLARE_ALGORITHM( SetSeeds );    

} 
