#ifndef DFC_RESULTSDEF_H
#define DFC_RESULTSDEF_H

/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   DFC_resultsDef.h
   \brief  Defines the data structure for summarizing the event-by-event
           results of the filtering process
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */

#include "tmr.h"


#ifdef __cplusplus
extern "C" {
#endif    


/* ---------------------------------------------------------------------- *//*!

  \struct _DFC_resultTemSizes
  \brief   The total size and sizes of the TEM record components
									  *//*!
  \typedef DFC_resultTemSizes
  \brief   Typedef for struct \a DFC_resultTemSizes

   Note that because of LCB padding the total size of the TEM record
   is not necessarily the sum of the TKR and CAL contributions.
									  */
/* ---------------------------------------------------------------------- */
typedef  struct _DFC_resultTemSizes
{
   unsigned int tem;  /*!< Size of the TEM record                         */
   unsigned int cal;  /*!< Size of the CAL record                         */
   unsigned int tkr;  /*!< Size of the TKR record                         */
}
DFC_resultTemSizes;
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \struct _DFC_resultSizes
  \brief   Describes the event-by-event sizes of the contributions
                                                                          *//*!
  \typedef DFC_resultSizes
  \brief   Typedef for DFC_resultSizes
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _DFC_resultSizes
{
    unsigned int           evt; /*!< Total size (in bytes)                */
    unsigned int           gem; /*!< Size of the GEM                      */
    unsigned int           acd; /*!< Size of the ACD                      */
    DFC_resultTemSizes tem[16]; /*!< Sizes of TEM records by tower        */
}
DFC_resultSizes;
/* ---------------------------------------------------------------------- */

    
          

/* ---------------------------------------------------------------------- *//*!

  \struct _DFC_results
  \brief   Describes the event-by-event results of the filtering process
                                                                          *//*!
  \typedef DFC_results
  \brief   Typedef for DFC_results
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _DFC_results
{
    int    evtNum;  /*!< Event number                                     */
    int    status;  /*!< DFC summary status                               */
    int   calSize;  /*!< CAL size in logs struck                          */
    int   tkrSize;  /*!< TKR size in strips hit                           */
    
    DFC_resultSizes
            sizes;  /*!< Sizes of the contributors                        */
    int    energy;  /*!< Total energy in the calorimeter                  */
    TMR_tick  beg;  /*!< Beginning time of the analysis                   */
    TMR_tick cal0;  /*!< Ending    time of the CAL analysis, phase 0      */
    TMR_tick  acd;  /*!< Ending    time of the ACD analysis               */
    TMR_tick  dir;  /*!< Ending    time of the DIRectory fill             */
    TMR_tick  atf;  /*!< Ending    time of the ATF analysis               */
    TMR_tick cal1;  /*!< Ending    time of the CAL analysis, phase 1      */
    TMR_tick  tkr;  /*!< Ending    time of the TKR finding                */
}
DFC_results;
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
}
#endif    


#endif

