/* ---------------------------------------------------------------------- *//*!
   
   \file  TFC_latUnpack.c
   \brief Routines to unpack the TKR data for all towers in the LAT.
   \author JJRussell - russell@slac.stanford.edu

\verbatim

 CVS $Id$
\endverbatim 
                                                                          */
/* ---------------------------------------------------------------------- */

#include "windowsCompat.h"

#include "DFC/DFC_endianness.h"
#include "DFC/TFC_latUnpack.h"
#include "DFC/TFC_latRecord.h"
#include "DFC/TFC_towerUnpack.h"
#include "DFC/TFC_towerRecord.h"
#include "DFC/EBF_directory.h"
#include "DFC/EBF_tkr.h"
#include "tmr.h"
#include "ffs.h"


/* -----------------------------------------------------------------------*//*!
   
  \def    NOT_CONTIGIOUS
  \brief  Determines the minimum strip separation for hit strips
          to be considered contigious
          
   This variable determines when 2 strip addresses are not contigious
   There is really only 1 right answer (2), but by setting this value
   to anything less than 2, no clusters will be formed, allowing debugging
   of this code.
                                                                          */
/* -----------------------------------------------------------------------*/
#define NOT_CONTIGIOUS 2
//#define NOT_CONTIGIOUS 0
/* -----------------------------------------------------------------------*/


/* -----------------------------------------------------------------------*//*!
  \def   USE_GAPS
  \brief Determines whether the code translates the strip addresses
         by the ladder gaps.

   This variable determines whether the code translates the strip addresses
   by the ladder gaps. It is used exclusively for debugging purposes. The
   default setting is 1, indicating that the gap correction is to be
   applied.
                                                                          */
/* -----------------------------------------------------------------------*/
#define USE_GAPS 1
//#define USE_GAPS 0
/* -----------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif

static inline void setActive (int xyL, TFC_towerLayer *layer);

    
/* -----------------------------------------------------------------------*//*!

  \def    TFC__stripToLen(_s, _x)
  \brief  Transforms a strip address into a linear displacement, measured
          in units of a strip width
  \param  _s The strip address to transform
  \param  _x An array of gaps to be applied to each 128 group of strips.
  \return The linear displacement to be associated with the specified
          strip address.

  Each TKR tower is constructed of 4 ladders of 384 strips. There is a gap
  between each of the ladders of approximately 9 strips. This means that
  for the first ladder, the linear displacement is the strip address,
  for the second ladder, the linear displacement ins the strip address
  + the gap between ladders 0 and 1, and so forth.

  The transformation is performed using an array (16) of displacements
  which correspond to each 128 strips. This is number was chosen because
  it was the largest binary power of 2 number, allowing one to do a shift
  rather than the more obvious, but much slower, divide by 384. The
  only burden on the caller is to provide an array of 16 numbers giving
  the offset for each group of 128 strips. Using a grouping of 128 means
  that the same offset will be repeated 3 times. 
                                                                          */
/* -----------------------------------------------------------------------*/
#define TFC__stripToLen(_s, _x)   ((_s) + _x[((_s) >> 7) & 0xf])
/* -----------------------------------------------------------------------*/


    

/* -----------------------------------------------------------------------*//*!
  \def   INIT_LAYERS(_lyrs, _accepts, _ylayers, _map, _layers, _topBit)
  \brief Process the layer bit masks associated with the either X or Y layers
  
  \param _lyrs    An array which receives the address of the layer
                  structure for each of the struck layers.
  \param _accepts The 18 bits representing on end of the layers
  \param _ylayers Bit mask of the struck y layers in layer order, as
                  opposed to readin order of the \a _accepts.
  \param _map     A an array mapping readin bit position order to
                  the canonical layer order.
  \param _layers  The array layer structures.
  \param _topBit  An integer with its MSB set. (Purely an optimization
                  to prevent the compiler from reloading this value
                  each time.
                  
   The macro continuously scans the \a _accepts bit array, seeking the
   first set bit. Each such found bit is translated to a canonical layer
   number. The layer number is used to construct a bit mask of which
   layers are struck (this is essentially just a reordered version of
   the \a _accepts, except that the layer ends are OR'd together. In
   addition, the addresses of the layer structure describing the hits
   on each struck layer are stored in \a lyrs array.
                                                                          */
/* -----------------------------------------------------------------------*/
/*
#define INIT_LAYERS(_lyrs, _accepts, _xlayers, _map, _layers, _topBit) \
({                                                                     \
   while (_accepts)                                                    \
   {                                                                   \
       int                 n;                                          \
       int          layerNum;                                          \
       TFC_towerLayer *layer;                                          \
                                                                       \
       n           = FFS (_accepts);                                   \
      _accepts    &= ~(_topBit >> n);                                  \
       layerNum    = _map[n];                                          \
       layer       = &_layers[layerNum];                               \
     *_lyrs++      = layer;                                            \
       layer->end  = layer->beg;                                       \
      _xlayers    |= 1 << layerNum;                                    \
   }                                                                   \
                                                                       \
   _lyrs;                                                              \
})
*/

/* -----------------------------------------------------------------------*/

inline TFC_towerLayer **INIT_LAYERS(TFC_towerLayer **_lyrs, int _accepts, int _xlayers, const unsigned char *_map, TFC_towerLayer *_layers, unsigned int _topBit){
   while (_accepts)
   {
       int                 n;
       int          layerNum;
       TFC_towerLayer *layer;

       n           = FFS (_accepts);
      _accepts    &= ~(_topBit >> n);
       layerNum    = _map[n];
       layer       = &_layers[layerNum];
     *_lyrs++      = layer;
       layer->end  = layer->beg;
      _xlayers    |= 1 << layerNum;
   }

   return _lyrs;
}

/* -----------------------------------------------------------------------*//*!

  \def   S2(_d0,_d1)
  \brief Assembles the 2nd strip address from its two pieces
  \param _d0   The 32 bit word holding the upper 8 bits of the strip address
  \param _d1   The 32 bit word holding the lower 4 bits of the strip address

   Three 32 bit words hold exactly 8 12 bit strip addresses. In this
   series of 8 strip addresses, two span a 32 bit boundary, the 2nd and
   the 5th. This macro assembles the 2nd strip address from its two halves.
                                                                          */
/* -----------------------------------------------------------------------*/  
#define S2(_d0,_d1) ((_d0.bf.s2a << 4) | _d1.bf.s2b)
/* -----------------------------------------------------------------------*/



/* -----------------------------------------------------------------------*//*!

  \def   S5(_d1,_d2)
  \brief Assembles the 5th strip address from its two pieces
  \param _d1   The 32 bit word holding the upper 4 bits of the strip address
  \param _d2   The 32 bit word holding the lower 8 bits of the strip address

   Three 32 bit words hold exactly 8 12 bit strip addresses. In this
   series of 8 strip addresses, two span a 32 bit boundary, the 2nd and
   the 5th. This macro assembles the 5th strip address from its two halves.
                                                                          */
/* -----------------------------------------------------------------------*/  
#define S5(_d1,_d2) ((_d1.bf.s5a << 8) | _d2.bf.s5b)
/* -----------------------------------------------------------------------*/



/* -----------------------------------------------------------------------*//*!
   
  \def   NPROCESS(_strip, _beg, _prv, _cur, _lcnt, _layer, _lyrs, _No, _Po)
  \brief Macro to process a strip address when starting a new cluster.
  \param _strip  The strip address to process
  \param _beg    Set to the new strip address if this is not the last
                 strip on the layer.
  \param _prv    Set to the new strip address if this is not the last
                 strip on the layer.
  \param _cur    Current address to store the strip address.
  \param _lcnt   Number of layers left to process.
  \param _layer  The current TFC tower layer address
  \param _lyrs   A stack of the TFC tower layer addresses.
  \param _No     Target statement label when the next strip to process
                 begins a new cluster.
  \param _Po     Target statement label when the next strip to process
                 may be a member of the cluster in progress

   This macro is used only when starting the construction of a new cluster.
   If the strip address to be processed is not the last strip on a layer,
   a new cluster is initialized by setting the beginning and ending of
   the cluster to the strip address.
   
   If this is the last strip on a layer, the strip address is stored along
   with the ending address of this layer. If there are more layers to
   process, the address to store the new strip addresses is retrieved
   and the cluster status is set to no cluster in progress by setting
   the beginning cluster strip address to -1.

   If there are no more layers to process the action indicated by \e _action
   is taken. This is generally a \e break or \e goto statement.
                                                                          */
/* -----------------------------------------------------------------------*/
#define NPROCESS(_strip, _beg, _prv, _cur, _lcnt, _layer, _lyrs, _No, _Po) \
     {                                                                \
          int strip = _strip;                                         \
          int     s = TFC__stripToLen (strip & 0x7ff, sXl);           \
                                                                      \
          if ((strip & (1 << 11)))                                    \
          {                                                           \
            /*                                                        \
             | By definition, there is no cluster in progress, so,    \
             | since this is the last strip on a layer, just store it \
             | (after removing the end of layer bit) and commence     \
             | the usual end of layer processing. The goto directs    \
             | the flow to the code which processes the next strip    \
             | address when there is no cluster currently being built.\
            */                                                        \
             _lcnt       -= 1;                                        \
            *_cur++       = s;                                        \
             _layer->end  = _cur;                                     \
              if (_lcnt <= 0) { break; }                              \
                                                                      \
             _layer       = *_lyrs++;                                 \
             _cur         =  _layer->end;                             \
              goto _No;                                               \
          }                                                           \
          else                                                        \
          {                                                           \
             /*                                                       \
              | By definition, there is no cluster in progress, so,   \
              | since this is not the last strip on a layer, a new    \
              | cluster is created.                                   \
             */                                                       \
             _prv = _beg = s;                                         \
             goto _Po;                                                \
          }                                                           \
     }
/* -----------------------------------------------------------------------*/




/* -----------------------------------------------------------------------*//*!
   
  \def   CPROCESS(_strip, _beg, _prv, _cur, _lcnt, _layer, _lyrs, _No, _Po)
  \brief Macro to process a strip address when a cluster is being built
  \param _strip  The strip address to process
  \param _beg    Beginning strip address of the cluster.
  \param _prv    Ending strip address of the cluster, i.e. the previous
                 strip address.
  \param _cur    Current address to store the strip address.
  \param _lcnt   Number of layers left to process.
  \param _layer  The current TFC tower layer address
  \param _lyrs   A stack of the TFC tower layer addresses.
  \param _No     Target statement label when the next strip to process
                 begins a new cluster.
  \param _Po     Target statement label when the next strip to process
                 may be a member of the cluster in progress.

   This macro is used only when a cluster is in the process of being built.
   The parameters \a _beg and \a _prv indicate the beginning and ending
   strip addresses of the current cluster.

   A check is made to see if the new strip address to be stored is
   contiguous the current cluster. If so the ending address of the cluster
   is updated.

   If the current cluster is not contiguous, there are two possible
   reasons. The first is the obvious, they two strip addresses are not
   contiguous. The second is that the new strip address is actually
   at the end of a layer. If this is so. the check for being contiguous
   is remade after stripping the layer end bit. If not, the old
   cluster centroid is stored and a new one begun.

   If a cluster is in the process of being built, the flow of the code
   is directed to the statement label specified by \a _Po. If no cluster
   is currently being built, the flow of the code is directed to the
   statement label specified by \a _No.
                                                                          */
/* -----------------------------------------------------------------------*/
#define CPROCESS(_strip, _beg, _prv, _cur, _lcnt, _layer, _lyrs, _No, _Po)\
     {                                                                \
          int strip = _strip;                                         \
          int     s = TFC__stripToLen (strip & 0x7ff, sXl);           \
                                                                      \
                                                                      \
          /*                                                          \
           |  Check if this strip is at the end of a layer.           \
          */                                                          \
          if ((strip & (1 << 11)))                                    \
          {                                                           \
              _lcnt      -= 1;                                        \
                                                                      \
              /*                                                      \
               |  If this strip is contiguous with the current        \
               |  cluster, extend the cluster and store the centroid. \
              */                                                      \
              if (s - _prv <= NOT_CONTIGIOUS)                         \
              {                                                       \
                 *_cur++ = (_beg + s) >> 1;                           \
              }                                                       \
              else                                                    \
              {                                                       \
                 /*                                                   \
                  |  This strip is not contiguous with the current    \
                  |  cluster, store the centroid of the current       \
                  |  cluster and then store the current strip.        \
                 */                                                   \
                 _cur[0] = (_beg + _prv) >> 1;                        \
                 _cur[1] = s;                                         \
                 _cur   += 2;                                         \
              }                                                       \
                                                                      \
              /*                                                      \
               |  Perform the standard end of layer processing.       \
               |  Direct the flow to the code that processes the      \
               |  next strip when there is no cluster in progress     \
              */                                                      \
              _layer->end = _cur;                                     \
               if (_lcnt  <= 0) { break;  }                           \
              _layer      = *_lyrs++;                                 \
              _cur        =  _layer->end;                             \
               goto _No;                                              \
          }                                                           \
          else                                                        \
          {                                                           \
              /*                                                      \
               |  Not at the end of a layer, so check if this strip   \
               |  is contiguous with the current cluster. If it is,   \
               |  then update the current cluster's end point.        \
              */                                                      \
              if (s - _prv > NOT_CONTIGIOUS)                          \
              {                                                       \
                 /*                                                   \
                  |  Current strip is not contiguous with the current \
                  |  cluster, so store the current cluster and start  \
                  |  a new cluster.                                   \
                 */                                                   \
                *_cur++ = (_beg + _prv) >> 1;                         \
                 _beg   = s;                                          \
              }                                                       \
                                                                      \
              /*                                                      \
               | In either case, either in starting a new cluster     \
               | (the code path which falls through from above) or    \
               | extending the current cluster, just extend the       \
               | the end point of the current cluster and go to the   \
               | code which process the next strip address when a     \
               | cluster is currently opened.                         \
              */                                                      \
             _prv = s;                                                \
              goto _Po;                                               \
          }                                                           \
                                                                      \
     }
/* -----------------------------------------------------------------------*/





/* -----------------------------------------------------------------------*//*!

  \def  PROCESS(_Ni,_Pi         
                _strip, _beg, _prv, _cur, _lcnt, _layer, _layers,
                _No,_Po)
  \brief A convenience macro to drop down a process cluster in progress
         and a new cluster macro.
  \param  _Ni    C statement label to assign to the new cluster macro
  \param  _Ci    C statement label to assing to the cluster in progress macro
  \param _strip  The strip address to process
  \param _beg    Beginning strip address of the cluster.
  \param _prv    Ending strip address of the cluster, i.e. the previous
                 strip address.
  \param _cur    Current address to store the strip address.
  \param _lcnt   Number of layers left to process.
  \param _layer  The current TFC tower layer address
  \param _lyrs   A stack of the TFC tower layer addresses.
  \param _No     Target statement label when the next strip to process
                 begins a new cluster.
  \param _Po     Target statement label when the next strip to process
                 may be a member of the cluster in progress
                                                                          */
/* -----------------------------------------------------------------------*/
#define PROCESS(_Ni,_Pi,                                                   \
                _strip, _beg, _prv, _cur, _lcnt, _layer, _layers,          \
                _No,_Po)                                                   \
                                                                           \
_Ni: NPROCESS(_strip, _beg, _prv, _cur, _lcnt, _layer, _layers, _No, _Po); \
_Pi: CPROCESS(_strip, _beg, _prv, _cur, _lcnt, _layer, _layers, _No, _Po);
/* -----------------------------------------------------------------------*/




/* -----------------------------------------------------------------------*//*!

  \var    Map
  \brief  The mapping of the entries in the layer accept list to the
          physical layer numbers

   The accept list is processed as 4 pairs of cables. The first cable of
   the pair carries the even layers, the second carries the odd layers.
   The 4 pairs represent the X lo, X hi and Y lo, Y hi cable pairs. In
   general one would need to map all 72 layer ends, but because all cable
   pairs are identical, so the same map can be used for each.

   This mapping could have been easily handled in the INIT_LAYERS macro,
   but the cost would have been some additional arthimetic. The current
   set bit would have to be translated into a layer number by doubling it
   and adding 1 iff the current bit was greater than 8. The lookup method
   seems simpler and faster.
                                                                          */
/* -----------------------------------------------------------------------*/
static const unsigned char Map[18] =
{
    1,  3,  5,  7,  9, 11, 13, 15, 17,  /* Odd  layers */
    0,  2,  4,  6,  8, 10, 12, 14, 16   /* Even layers */
};
/* -----------------------------------------------------------------------*/





/* -----------------------------------------------------------------------*//*!

  \struct _TFC_2strips_bf
  \brief  Maps out the first word containing strip addresses
                                                                          *//*!
  \typedef TFC_2strips_bf
  \brief   Typedef for struct \e _TFC_2strips_bf

   The first 32-bit word containing strip addresses also contains the
   last 8 bits of the accept list. This structure maps these fields out
   for easy access.
                                                                          */
/* -----------------------------------------------------------------------*/
#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_BIG

   typedef struct _TFC_2strips_bf
   {
       unsigned int accepts:8;  /*!< The final 8 accept bits              */
       unsigned int     s0:12;  /*!< Strip 0 address                      */
       unsigned int     s1:12;  /*!< Strip 1 address                      */
   }
   TFC_2strips_bf;

#else

   typedef struct _TFC_2strips_bf
   {
       unsigned int     s1:12;  /*!< Strip 1 address                      */
       unsigned int     s0:12;  /*!< Strip 0 address                      */
       unsigned int accepts:8;  /*!< The final 8 accept bits              */
   }
   TFC_2strips_bf;

#endif
/* -----------------------------------------------------------------------*/




/* -----------------------------------------------------------------------*//*!

  \struct _TFC_8strips0_bf
  \brief  Maps out the first word of the 3 strip address pattern words as
          bit fields.

   Strip addresses are 12-bit fields densely packed into 32-bit words. This
   means that some strip addresses cross 32 bit boundaries. Since every 3
   32-bit words will contain exactly 8 strip addresses, the pattern repeats
   every 3 32-bit words.

   This structure maps out how the first two strip addresses and part
   of the third address fits in the first word of this three word pattern.

   See also \e _TFC_8strips1_bf and \e _TFC_8strips2_bf, which gives the
   mapping for the other 2 words
                                                                          *//*!
  \typedef TFC_8strips0_bf
  \brief   Typedef for struct \e _TFC_8strips0_bf
                                                                          */
/* -----------------------------------------------------------------------*/
#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_BIG

   typedef struct _TFC_8strips0_bf
   {
       unsigned int s0 :12;  /*!< Strip address 0                         */
       unsigned int s1 :12;  /*!< Strip address 1                         */
       unsigned int s2a: 8;  /*!< Strip address 2, the upper 8 bits       */
   }
   TFC_8strips0_bf;

#else

   typedef struct _TFC_8strips0_bf
   {
       unsigned int s2a: 8;  /*!< Strip address 2, the upper 8 bits       */
       unsigned int s1 :12;  /*!< Strip address 1                         */
       unsigned int s0 :12;  /*!< Strip address 0                         */
   }
   TFC_8strips0_bf;

#endif
/* -----------------------------------------------------------------------*/

   



   
/* -----------------------------------------------------------------------*//*!

  \struct _TFC_8strips1_bf
  \brief  Maps out the second word of the 3 strip address pattern words as
          bit fields.

   Strip addresses are 12-bit fields densely packed into 32-bit words. This
   means that some strip addresses cross 32 bit boundaries. Since every 3
   32-bit words will contain exactly 8 strip addresses, the pattern repeats
   every 3 32-bit words.

   This structure maps out how the part of second strip address, the
   third and fourth strip addresses and part of the fifth address fits
   in the second word of this three word pattern.

   See also \e _TFC_8strips0_bf and \e _TFC_8strips2_bf, which gives the
   mapping for the other 2 words.
                                                                          *//*!
  \typedef TFC_8strips1_bf
  \brief   Typedef for struct \e _TFC_8strips1_bf
                                                                          */
/* -----------------------------------------------------------------------*/
#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_BIG

   typedef struct _TFC_8strips1_bf
   {
       unsigned int s2b: 4;  /*!< Strip address 2, lower 4 bits           */
       unsigned int s3 :12;  /*!< Strip address 3                         */ 
       unsigned int s4 :12;  /*!< Strip address 4                         */  
       unsigned int s5a: 4;  /*!< Strip address 5, upper 4 bits           */
   }
   TFC_8strips1_bf;

#else

   typedef struct _TFC_8strips1_bf
   {
       unsigned int s5a: 4;  /*!< Strip address 5, upper 4 bits           */
       unsigned int s4 :12;  /*!< Strip address 4                         */ 
       unsigned int s3 :12;  /*!< Strip address 3                         */  
       unsigned int s2b: 4;  /*!< Strip address 2, lower 4 bits           */
   }
   TFC_8strips1_bf;

#endif
/* -----------------------------------------------------------------------*/





/* -----------------------------------------------------------------------*//*!

  \struct _TFC_8strips2_bf
  \brief  Maps out the third word of the 3 strip address pattern words as
          bit fields.


   Strip addresses are 12-bit fields densely packed into 32-bit words.
   This means that some strip addresses cross 32 bit boundaries. Since
   every 3 32-bit words will contain exactly 8 strip addresses, the
   pattern repeats every 3 32-bit words.

   This structure maps out how the part of fifth strip address, and the
   sixth and seventh strip addresses fits in the third word of this three
   word pattern.

   See also \e _TFC_8strips0_bf and \e _TFC_8strips1_bf, which gives the
   mapping for the other 2 words.
                                                                          *//*!
  \typedef TFC_8strips2_bf
  \brief   Typedef for struct \e _TFC_8strips2_bf
                                                                          */
/* -----------------------------------------------------------------------*/
#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_BIG

   typedef struct _TFC_8strips2_bf
   {
       unsigned int s5b: 8;   /*!< Strip address 5, upper 8 bits          */
       unsigned int s6 :12;   /*!< Strip address 6                        */ 
       unsigned int s7 :12;   /*!< Strip address 7                        */ 
   }
   TFC_8strips2_bf;

#else

   typedef struct _TFC_8strips2_bf
   {
       unsigned int s7 :12;   /*!< Strip address 7                        */
       unsigned int s6 :12;   /*!< Strip address 6                        */ 
       unsigned int s5b: 8;   /*!< Strip address 5, upper 8 bits          */
   }
   TFC_8strips2_bf;

#endif
/* -----------------------------------------------------------------------*/




/* -----------------------------------------------------------------------*//*!

  \struct _TFC_2strips
  \brief  Maps out the first word containing strip addresses
                                                                          *//*!
  \typedef TFC_2strips
  \brief   Typedef for struct \e _TFC_2strips_bf

   The first 32-bit word containing strip addresses also contains the
   last 8 bits of the accept list. This structure maps these fields out
   as both an uninterpreted 32 bit value and as bit fields for easy
   access.
                                                                          */
/* -----------------------------------------------------------------------*/
typedef union _TFC_2strips
{
    TFC_2strips_bf bf;  /*!< Initial strip address/accept as bit fields   */
    unsigned int   ui;  /*!< Initial strip address/accept as 32-bit int   */
}
TFC_2strips;
/* -----------------------------------------------------------------------*/



/* -----------------------------------------------------------------------*//*!

  \union _TFC_8strips0
  \brief  Maps out the first word of the 3 strip address pattern words as
          bit fields and uninterpretted 32-bit integer.

   Strip addresses are 12-bit fields densely packed into 32-bit words.
   This means that some strip addresses cross 32 bit boundaries. Since
   every 3 32-bit words will contain exactly 8 strip addresses, the
   pattern repeats every 3 32-bit words.
   
   This union maps out how the first two strip addresses and part of the
   third address fits in the first word of this three word pattern.

   See also \e _TFC_8strips1 and \e _TFC_8strips2, which gives the
   mapping for the other 2 words
                                                                          *//*!
  \typedef TFC_8strips0
  \brief   Typedef for union \e _TFC_8strips0.
                                                                          */
/* -----------------------------------------------------------------------*/
typedef union _TFC_8strips0
{
    TFC_8strips0_bf bf; /*!< Strip address word 0 as bit fields           */ 
    unsigned int    ui; /*!< Strip address word 0 as 32-bit int           */
}
TFC_8strips0;
/* -----------------------------------------------------------------------*/





/* -----------------------------------------------------------------------*//*!

  \union _TFC_8strips1
  \brief  Maps out the second word of the 3 strip address pattern words as
          bit fields and uninterpretted 32-bit integer.

   Strip addresses are 12-bit fields densely packed into 32-bit words.
   This means that some strip addresses cross 32 bit boundaries. Since
   every 3 32-bit words will contain exactly 8 strip addresses, the
   pattern repeats every 3 32-bit words.
          
   This union maps out how the part of second strip address, the
   third and fourth strip addresses and part of the fifth address fits
   in the second word of this three word pattern.

   See also \e _TFC_8strips0 and \e _TFC_8strips2, which gives the
   mapping for the other 2 words.
                                                                          *//*!
  \typedef TFC_8strips1
  \brief   Typedef for union \e _TFC_8strips1.
                                                                          */
/* -----------------------------------------------------------------------*/
typedef union _TFC_8strips1
{
    TFC_8strips1_bf bf; /*!< Strip address word 1 as bit fields           */ 
    unsigned int    ui; /*!< Strip address word 1 as 32-bit int           */ 
}
TFC_8strips1;
/* -----------------------------------------------------------------------*/




/* -----------------------------------------------------------------------*//*!

  \union _TFC_8strips2
  \brief  Maps out the third word of the 3 strip address pattern words as
          bit fields and uninterpretted 32-bit integer.


   Strip addresses are 12-bit fields densely packed into 32-bit words.
   This means that some strip addresses cross 32 bit boundaries. Since
   every 3 32-bit words will contain exactly 8 strip addresses, the pattern
   repeats every 3 32-bit words.

   This union maps out how the part of fifth strip address, and the
   sixth and seventh strip addresses fits in the third word of this three
   word pattern.

   See also \e _TFC_8strips0 and \e _TFC_8strips1, which gives the
   mapping for the other 2 words.
                                                                          *//*!
  \typedef TFC_8strips2
  \brief   Typedef for union \e _TFC_8strips2.
                                                                          */
/* -----------------------------------------------------------------------*/
typedef union _TFC_8strips2
{
    TFC_8strips2_bf bf; /*!< Strip address word 2 as bit fields           */
    unsigned int    ui; /*!< Strip address word 2 as 32-bit int           */ 
}
TFC_8strips2;
/* -----------------------------------------------------------------------*/





/* -----------------------------------------------------------------------*//*!

  \fn     void setActive (int xyL, TFC_towerLayer *layer)
  \brief  Sets the bit mask representing the valid strip numbers for
          each layer
  \param   xyL A bit mask of the active layers
  \param layer The array of layer descriptors

  \warning
  This works only if the number of struck strips is 32 or less
                                                                          */
/* -----------------------------------------------------------------------*/
static void setActive (int xyL, TFC_towerLayer *layer)
{ 
    /*
     | Adjust the array of layer descriptors to be compatiable with the
     | FFS function.
    */
    layer += 31;

    /* Loop over the layers with hits */
    while (xyL)
    {
        TFC_towerLayer *l;
        int             n;
        int           cnt;
        
        n      = FFS (xyL);            /* Locate the next struck layer */
        xyL   &= ~(0x80000000 >> n);   /* Eliminate it from the mask   */
        l      = layer - n;            /* Index to the correct layer   */
        cnt    = l->end - l->beg;      /* Number of strips this layer  */
        l->cnt = cnt;                  /* Save count                   */


        /*
         | !!! KLUDGE !!!
         | --------------
         | This formula works iff cnt is between 1 and 32. The low end
         | is protected by the fact that the layer mask is non-zero
         | The upper end is unprotected and fails if there are more
         | than 32 hits.
        */
        l->map = -1 << (32 - cnt);     /* Create the bit mask          */
    }

    return;
}
/* -----------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif
    


/* -----------------------------------------------------------------------*//*!

  \fn  int TFC_latUnpack (struct _TFC_latRecord       *tlr,
                          const struct _EBF_directory *dir,
                          int                         tmsk)

  \brief Driver routine to unpack the specified TKR towers.
  \param tlr  The TKR LAT record structure to receive the unpacked data
  \param dir  The standard directory structure allowing the routine
              to traverse the LAT event record.
  \param tmsk A bit mask of which towers to unpack. MSB = Tower 0.
  \return     Status, currently always success.
                                                                          */
/* -----------------------------------------------------------------------*/
int TFC_latUnpack (struct _TFC_latRecord       *tlr,
                   const struct _EBF_directory *dir,
                   int                         tmsk)
{
   const EBF_contributor *contributors;
   TFC_strip                   *strips;
   int                          twrMap;
   unsigned int                 topBit;
   

   /*
    | Limit towers to be unpacked to those requested, that exist and still
    | need to be unpacked.
   */
   twrMap       = tlr->twrMap;
   tmsk        &= EBF_DIRECTORY_TWRS_TKR (dir->ctids) & ~twrMap;
   tlr->twrMap |= tmsk;
   contributors = dir->contributors;
   strips       = tlr->strips + tlr->stripCnt;
   topBit       = 1 << 31;
   
   
   do
   {
       int                          tower;
       int                         calcnt;
       int                            cnt;
       const struct _EBF_tkr         *tkr;       
       const EBF_contributor *contributor;
       TFC_towerRecord               *ttr;

       
       /*
        | Locate the first tower to unpack
        | The tower number is used to locate the tracker data and the
        | structure to unpack into.
       */       
       tower       = FFS (tmsk);
       contributor = &contributors[tower];
       ttr         = &tlr->twr[tower];
       tmsk       &= ~(topBit >> tower);
       cnt         = contributor->len / sizeof (int);
       calcnt      = contributor->calcnt;
       tkr         = EBF__tkrLocate (contributor->ptr, calcnt);
       cnt        -= calcnt;
       ttr->id     = tower;
       strips      = TFC_towerUnpack (ttr, strips, tkr, cnt);
   }
   while (tmsk);

   return 0;
}
/* -----------------------------------------------------------------------*/



       
/* --------------------------------------------------------------------- *//*!

  \fn TFC_strip *TFC_towerUnpack (struct _TFC_towerRecord *ttr,
                                  TFC_strip            *strips,
                                  const struct _EBF_tkr   *tkr,
                                  int                      cnt)
  \brief Unpacks the data from one tower
  \param ttr  Pointer to the data structure to receive the unpacked data
  \param strips UNUSED...
  \param tkr    The tracker data as received from the hardware
  \parm  cnt    UNUSED
                                                                         *//*!
  \def       GAP(_n)
  \brief     Calculates the cumulative length due to the gaps between the
             ladders
  \param  _n The ladder number (0-3)
  \return    The cumulative length due to the gaps between the ladders
             in units of strips.
                                                                         */
/* --------------------------------------------------------------------- */
TFC_strip *TFC_towerUnpack (struct _TFC_towerRecord *ttr,
                            TFC_strip            *strips,
                            const struct _EBF_tkr   *tkr,
                            int                      cnt)
{
# if USE_GAPS 
# define GAP(_n) ((int)((_n) * ((.974 + .2 + .974) / .228) + 0.5))
# else 
# define GAP(_n) 0
# endif 
   static const unsigned char SxL[16] =
   { GAP(0), GAP(0), GAP(0),
     GAP(1), GAP(1), GAP(1),
     GAP(2), GAP(2), GAP(2),
     GAP(3), GAP(3), GAP(3),
     GAP(3), GAP(3), GAP(3)   /* Should be unused since only 12 segments */
   };
 
 
   int  a0;
   int  a1;
   int  axlo, axhi;
   int  aylo, ayhi;
   int  xL;
   int  yL;
   
   TFC_2strips              a2;
   int                    lcnt;
   TFC_towerLayer      *layers;
   TFC_towerLayer       *layer;
   TFC_towerLayer       **lyrs;
   TFC_towerLayer *lyrsBuf[72];

   TFC_strip              *cur;
   int                     beg;
   int                     prv;
   unsigned int            b31;
   const unsigned int    *data;
   TFC_8strips0             d0;
   TFC_8strips1             d1;
   TFC_8strips2             d2;
   const unsigned char    *sXl;
   

   /*
    |  Extract accept list for the 4 pairs of cables representing
    |
    |    X lo even | X lo odd  : Bits  0-17
    |    X hi even | X hi odd  : Bits 18-35
    |    Y lo even | Y lo odd  : Bits 36-53
    |    Y hi even | Y hi odd  : Bits 54-71
    |
    |  Note that each mask is 18 bits long and they are cut out
    |  of the 72 accept bits in sequential order with MSB of accept
    |  word 0 carrying the X0 lo even layer bit.
   */
   a0    = tkr->accepts[0];
   a1    = tkr->accepts[1];
   a2.ui = tkr->accepts[2];
   
   axlo  = EBF_TKR_ACCEPTS_LEFT_XLO (a0);
   axhi  = EBF_TKR_ACCEPTS_LEFT_XHI (a0, a1);
   aylo  = EBF_TKR_ACCEPTS_LEFT_YLO (a1);
   ayhi  = EBF_TKR_ACCEPTS_LEFT_YHI (a1, a2.ui);

   /*
        printf ("axlo = %8.8x\n"
                "axhi = %8.8x\n"
                "aylo = %8.8x\n"
                "ayhi = %8.8x\n",
                axlo, axhi, aylo, ayhi);
   */        
   
   xL    = 0;
   yL    = 0;
   b31   = (1 << 31);
   lyrs  = lyrsBuf;

   
   /*
    | Build a dense array of which layers have strip hits. The set
    | bits in the 4-18 bit accept lists are translated into a layer
    | number by extracting the bit numbers of the set bits and
    | indexing a lookup table (Map) which maps the bit number into
    | its corresponding layer number. The address of the layer
    | descriptor corresponding to this layer number is stored in
    | the layer descriptor array.
    |
    | This list of hit layers is then consumed by the strip address
    | decoding piece of this code. Each time the end of the strip
    | addresses for a given layer is found (end is bit 11 of the
    | strip address) the next layer descriptor is read. The layer
    | descriptor contains information about where to store the 
    | strip addresses for this layer. The accept list is laid out
    | such that all lower strip addresses for a particular layer
    | are processed first, followed by the high strip addresses.
    | This allows the high strip addresses to be simply appended
    | to the existing list of low strip addresses (if any).
    |
    | WARNING
    | -------
    | The order of the following statements is important. The
    | order must match the order of the layers in the strip
    | address vector list.  Do not rearrange this code.
    |
   */

   /* Process the X layers */
   layers = ttr->layers;
   lyrs   = INIT_LAYERS (lyrs, axlo, xL, Map, layers, b31);
   lyrs   = INIT_LAYERS (lyrs, axhi, xL, Map, layers, b31);

   /* Process the Y layers */
   layers = ttr->layers + sizeof(ttr->layers)/(2*sizeof(*ttr->layers));
   lyrs   = INIT_LAYERS (lyrs, aylo, yL, Map, layers, b31);
   lyrs   = INIT_LAYERS (lyrs, ayhi, yL, Map, layers, b31);

   /* Count the number of active layers in this tower */
   lcnt   = lyrs - lyrsBuf;

   /*
       printf ("xL  = %8.8x\n"
               "yL  = %8.8x\n"
               "lcnt= %2d\n",
               xL, yL, lcnt);
   */
   

   /*
    | Initialize the tower structure with the beginning strip address
    | and the x & y layer masks. Note that the layer masks are in the
    | canonical order. Thus bit 0 represents layer 0, bit 1, layer 1,
    | etc. This is because the track finding is done starting from the
    | layers closest to the ACD top face, i.e. from the highest number
    | layers. Therefore the layer bit maps match the FFS convention,
    | that is, the first bit (layer) that will be found by the FFS
    | will be the highest layer number.
   */
   ttr->layerMaps[0] = xL;
   ttr->layerMaps[1] = yL;


   /*
    | 
    | MAIN LOOP TO UNPACK THE STRIP ADDRESSES
    | ---------------------------------------
    | The while loop is arranged so that it handles 3 words (8 strip
    | addresses in a pass. This is the lowest harmonic of the 12 (the
    | number of bits in a strip address) and 32, (the number of bits
    | in a 32 bit word.)
    |
    | The loop is phased so that the first two strip addresses left in
    | the last accept word are correctly processed. This means sticking
    | the 32 bit pattern that has 8 bits, followed by 2 12 bit strip
    | addresses at the beginning (this occurs in the TFC_8strips2 
    | structure). After the shard is processed, a fresh set of 3 32 bit
    | words is extracted and processed.
    |
    | The loop needs to have some context established. The current layer
    | pointer and the current strip storage address must be established,
    | along with a pointer the strip data.
   */
   lyrs  = lyrsBuf;     /* Array of active layer descriptors pointers */
   layer = *lyrs++;     /* The first layer descriptor                 */
   cur   = layer->beg;  /* Address to store the next strip address    */
   data  = &tkr->accepts[3] - 3;  /* Beginning of the strip data....  */
   d2.ui = a2.ui;       /* .. after the shard has been processed      */
   sXl   = SxL;         /* Array giving the inter-ladder gap          */

   
   
   /*   
    | The loop is completed when all the struck layers, represented by
    | lcnt, have been processed. Also do not insert any statements between
    | the following code. The flow does not proceed linearly. Instead it
    | is directed to one of the two statement labels (the last two arguments
    | of the PROCESSing macros). Each PROCESSing macro has two major
    | pieces, one to process a strip address when a cluster is being built
    | and one to process a strip address when no cluster is being built.
    | In short, any code inserted between the PROCESSing macros will never
    | be executed. I don't generally like writing code in this style, but
    | the alternative of caring the context in a variable and testing and
    | branching on this variable costs ~5-10% in performance in this
    | critical piece of code.
   */ 
   while (1)
   {
       PROCESS (N6, P6, d2.bf.s6, beg, prv, cur, lcnt, layer, lyrs, N7, P7);
       PROCESS (N7, P7, d2.bf.s7, beg, prv, cur, lcnt, layer, lyrs, N0, P0);

       
       /*
        | Now on a even word boundary, decode 3 words or 8 strips worth.
        | Note that there is no guarantee that 8 strips of data remain,
        | but the cost of checking is not worth the cost of just blindly
        | extracting the next 96 bits. If some of the data is not used,
        | no harm, beyond the 3 or less extra fetches, is done.
        |
        | Again remember the code is not linear in this region. The flow
        | from the "PROCESS (N7, P7, .... NO, P0)" is directed at either
        | NO or PO section, but not both.
       */
       N0:
       data += 3;       
       d0.ui = data[0];
       d1.ui = data[1];
       d2.ui = data[2];
       NPROCESS (d0.bf.s0, beg, prv, cur, lcnt, layer, lyrs,  N1, P1);     

       P0:
       data += 3;       
       d0.ui = data[0];
       d1.ui = data[1];
       d2.ui = data[2];
       CPROCESS (d0.bf.s0, beg, prv, cur, lcnt, layer, lyrs, N1, P1);     
       

       PROCESS (N1,P1, d0.bf.s1,  beg, prv, cur, lcnt, layer, lyrs, N2, P2);
       PROCESS (N2,P2, S2(d0,d1), beg, prv, cur, lcnt, layer, lyrs, N3, P3);
       PROCESS (N3,P3, d1.bf.s3,  beg, prv, cur, lcnt, layer, lyrs, N4, P4);
       PROCESS (N4,P4, d1.bf.s4,  beg, prv, cur, lcnt, layer, lyrs, N5, P5);
       PROCESS (N5,P5, S5(d1,d2), beg, prv, cur, lcnt, layer, lyrs, N6, P6);
   }

   
   /*
    | Set the bit masks of active strips in each struck layer. This mask
    | is used to keep track of the which strip addresses on a layer
    | are valid.
    |
    | WARNING
    | Only the first 32 strip addresses can be handled using this method
   */
   setActive (xL, ttr->layers);
   setActive (yL, ttr->layers + sizeof(ttr->layers)/(2*sizeof(*ttr->layers)));

   
   return strips + TFC_K_MAX_STRIPS_PER_TOWER;
}
/* ---------------------------------------------------------------------- */


   

/* ---------------------------------------------------------------------- *//*!

  \fn     int TFC_latUnpackSizeof (void)
  \brief  Returns the size, in bytes, of a TKR LAT record.
  \return The size, in bytes, of a TKR LAT record.

   This function should be called to determine the size of a \a tlr. This
   allows the calling program to avoid including the structure definition
   of a \a tlr.

   After allocating a \a tlr, the structure should be initialized using
   \a TFC_latInit().
                                                                          */
/* ---------------------------------------------------------------------- */
int TFC_latUnpackSizeof (void)
{
   return sizeof (struct _TFC_latRecord);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

 \fn         void TFC_latUnpackInit (struct _TFC_latRecord *tlr)
 \brief      Performs one time initialization of a Track LAT record.
 \param  tlr Pointer to the structure to initialize

  This function should be called only once to initialize the record. After
  that, TFC_latRecordReset should be called to reset the structure before
  each unpack.
                                                                          */
/* ---------------------------------------------------------------------- */
void TFC_latUnpackInit (struct _TFC_latRecord *tlr)
{
   int           twrNum;
   TFC_strip     *strip;
   TFC_towerRecord *twr;

   /*
    | Initialize the global part of the structure
   */
   tlr->stripCnt = 0;
   tlr->twrMap   = 0;


   /*
    | Loop over all the towers of the LAT, initializing their layer maps,
    | both X and Y to empty, and then initialize their layer descriptors.
   */
   for (twrNum = 0, twr = tlr->twr, strip = tlr->strips;
        twrNum < sizeof (tlr->twr) / sizeof (tlr->twr[0]);
        twrNum++, twr++)
   {
       int          lyrNum;
       TFC_towerLayer *lyr;
       
       twr->id           = twrNum;
       twr->layerMaps[0] = 0;
       twr->layerMaps[1] = 0;



       /*
        | Initialize the layer descriptors. This structure contains pointers
        | to the beginning and ending of the unpacked strips. Technically
        | only the beginning pointer needs to be initialized, but, for
        | completeness, the ending pointer is also set. Note that once the
        | beginning pointer is initialized, it need not be touched again.
       */
       for (lyrNum = 0, lyr = twr->layers;
            lyrNum < sizeof (twr->layers) / sizeof (twr->layers[0]);
            lyrNum++, lyr++)
       {
           lyr->beg = strip;
           lyr->end = strip;
           strip   += TFC_K_MAX_STRIPS_PER_LAYER;
       }
   }

   return;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

 \fn         void TFC_latUnpackReset (struct _TFC_latRecord *tlr)
 \brief      Resets the \a tlr record, preparing it to receive another
             unpacked event.
 \param  tlr Pointer to the structure to reset.

  This function should be called prior to unpacking a new event. It
  performs the minimal work needed to prepare the structure to receive
  a new event.
                                                                          */
/* ---------------------------------------------------------------------- */
void TFC_latUnpackReset (struct _TFC_latRecord *tlr)
{
   tlr->stripCnt = 0;
   tlr->twrMap   = 0;
   return;
}
/* ---------------------------------------------------------------------- */
