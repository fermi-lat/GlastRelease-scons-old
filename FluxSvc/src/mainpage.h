// $Header$
// Mainpage for doxygen

/*! \mainpage package FluxSvc

   \authors Toby Burnett, Sean Robinson, Theodore Hierath, and others.

 \section intro Introduction
  This package implements a Gaudi service, FluxSVc, and an algorithm, FluxAlg
  <br>

  The FluxSvc package is designed to manage: 
  - the orbital characteristics of GLAST,
  - the Emissive characteristics of various sources, and
  - the 'Current' time and orbital position, and instrument orientaiton.

<br>
Usage is primarily via the FluxAlg algorithm, which access the service to generate a particle and place it in the TDS.
    
  <hr>
  @section jobOptions jobOptions
    @param FluxSvc.source_lib   [\$(FLUXSVCROOT)/xml/source_library.xml] list of file names containing source_lib elements
    @param FluxSvc.dtd_file     [\$(FLUXSVCROOT)/xml/source.dtd]  DTD file used to parse the XML files
    @param FluxSvc.EvtMax       [0]  If non-zero, used as a maximum in the FluxSvc loop
    @param FluxSvc.StartTime    [0]  Mission elapsed time start
    @param FluxSvc.EndTime      [0]  Mission elapsed time end--if non-zero, will be the end.

    @param FluxAlg.source_name  ["default"] source name, name must be in the source_lib files
    @param FluxAlg.MCrun        [100] Initial run number
    @param FluxAlg.area         [5.0] target area in m^2
    @param FluxAlg.pointing_mode [0]
    @param FluxAlg.rocking_angle [0 deg]

 <hr>
  @section Basic_XML_Sources Sources
  This is a limited selection of sources. See the contents of the source library file for a complete list.
  @param default
  0.1 GeV gamma-rays coming from the vertical local direction.  Used for default tests.
  @param albedo_gamma
  Source that represents the Earth horizon albedo with Glast zenith pointing
  @param albedo_electronpositron
  Source that represents the spalsh and re-entrant albedo electrons and positrons
  @param diffuse
  diffuse extraglactic from 10 MeV: from APJ 494:523
  @param diffuse-100mev
  Diffuse extraglactic from 100 MeV
  @param crab-galactic
  the Crab, pulsed portion, with pointed observation, for photons above 100 MeV,
  based on Nolan APJ 409:697
  @param electron
  galactic electron spectrum
  @param normal_gamma
  E^-1 spectrum from 18 MeV to 18 GeV and normal incidence
  @param muons
  special source that mimics non-interacting cosmics

  <br>
  <h2> Defining an external source </h2>
    See the interface definition IRegisterSource for information on how to link code external to this package.


  <hr>
  \section notes release notes
  release.notes
  \section requirements requirements
  \include  requirements
  <hr>
   <hr>
  \todo Complete and recalibrate the CompositeDiffuse structure

*/

