/**
 * @class ITkrFitTool
 *
 * @brief Interface to the track fitting tools. 
 * Basically, provides an interface to various single track fit 
 * classes. Currently there is but one, but this allows for future
 * ideas/expansion/etc.
 *
 * @author Tracy Usher
 */

#ifndef ITKRFITTOOL_H
#define ITKRFITTOOL_H

#include "GaudiKernel/IAlgTool.h"

namespace Event
{
    class TkrPatCand;
    class TkrTrack;
}

static const InterfaceID IID_ITkrFitTool("ITkrFitTool", 7111 , 0);

class ITkrFitTool : virtual public IAlgTool 
{
 public:
  /// Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_ITkrFitTool; }

  /// @brief Given a pattern track, perform the track fit
  virtual StatusCode doTrackFit(Event::TkrPatCand* patCand)=0;
  virtual StatusCode doTrackFit(Event::TkrTrack*   patCand)=0;

  /// @brief Given a pattern track, perform the track re-fit
  virtual StatusCode doTrackReFit(Event::TkrPatCand* patCand)=0;
  virtual StatusCode doTrackReFit(Event::TkrTrack*   patCand)=0;

};
#endif
