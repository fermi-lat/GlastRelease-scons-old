echo off
IF .%1==. (set CMTCONFIG=VisualC) ELSE set CMTCONFIG=%1
set CMTROOT=c:\packages\CMT\v1r5p1
set GLASTMEDIAROOT=c:\packages\GlastGaudi\glastmedia\v3
set GLASTMEDIACONFIG=%CMTCONFIG%
set GISMOROOT=c:\packages\gismosys_v3\gismo\v3r1
set GISMOCONFIG=%CMTCONFIG%
set GEOMREPROOT=c:\packages\gismosys_v3\geomrep\v2
set GEOMREPCONFIG=%CMTCONFIG%
set GLASTSVCROOT=c:\packages\GlastGaudi\GlastSvc\v1r4
set GLASTSVCCONFIG=%CMTCONFIG%
set GLASTEVENTROOT=c:\packages\GlastGaudi\GlastEvent\v1r3
set GLASTEVENTCONFIG=%CMTCONFIG%
set INSTRUMENTROOT=c:\packages\GlastGaudi\instrument\v2r3
set INSTRUMENTCONFIG=%CMTCONFIG%
set DATAROOT=c:\packages\GlastGaudi\data\v3
set DATACONFIG=%CMTCONFIG%
set GEOMETRYROOT=c:\packages\gismosys_v3\geometry\v1r2
set GEOMETRYCONFIG=%CMTCONFIG%
set GUISVCROOT=c:\packages\GlastGaudi\GuiSvc\v1r1
set GUISVCCONFIG=%CMTCONFIG%
set GUIROOT=c:\packages\gismosys_v3\gui\v1r1
set GUICONFIG=%CMTCONFIG%
set XMLROOT=c:\packages\GlastGaudi\xml\v2r1
set XMLCONFIG=%CMTCONFIG%
set FACILITIESROOT=c:\packages\gismosys_v3\facilities\v1r1
set FACILITIESCONFIG=%CMTCONFIG%
set IDENTSROOT=c:\packages\GlastGaudi\idents\v1r1
set IDENTSCONFIG=%CMTCONFIG%
set GLAST_SETTINGSROOT=c:\packages\external\glast_settings\v3r1
set GLAST_SETTINGSCONFIG=%CMTCONFIG%
set GAUDISVCROOT=c:\packages\GlastGaudi\GaudiSvc\v4
set GAUDISVCCONFIG=%CMTCONFIG%
set CLHEPROOT=c:\packages\external\CLHEP\v1r6
set CLHEPCONFIG=%CMTCONFIG%
set XMLEXTROOT=c:\packages\external\XMLEXT\v1r1
set XMLEXTCONFIG=%CMTCONFIG%
set AUDITORSROOT=c:\packages\GlastGaudi\Auditors\v1
set AUDITORSCONFIG=%CMTCONFIG%
set ROOTHISTCNVROOT=c:\packages\GlastGaudi\RootHistCnv\v1
set ROOTHISTCNVCONFIG=%CMTCONFIG%
set GAUDIROOT=c:\packages\GlastGaudi\Gaudi\v8
set GAUDICONFIG=%CMTCONFIG%
set LHCBCMTROOT=c:\packages\GlastGaudi\LHCbCMT\v3r4
set LHCBCMTCONFIG=%CMTCONFIG%
set HTLROOT=c:\packages\external\HTL\v13
set HTLCONFIG=%CMTCONFIG%
set ROOTROOT=c:\packages\external\ROOT\v2r25
set ROOTCONFIG=%CMTCONFIG%
set EXTLIBROOT=c:\packages\external\EXTLIB\v2r3
set EXTLIBCONFIG=%CMTCONFIG%
set F2CROOT=c:\packages\external\f2c\v1
set F2CCONFIG=%CMTCONFIG%
set DETDISPLAYROOT=c:\packages\GlastGaudi\DetDisplay\v1
set DETDISPLAYCONFIG=%CMTCONFIG%
set EXT_DIR=%GLAST_EXT%
set LHCXX_DIR=%EXT_DIR%
set ROOT_DIR=%EXT_DIR%\ROOT
set LHCBDBASE=%EXT_DIR%\LHCBDBASE
set CLHEP_DIR=%LHCXX_DIR%\CLHEP\1.6.0.0
set PEGS4PATH=%GISMOROOT%\PEGS4
set XMLEXT_DIR=%EXT_DIR%\xerces-c_1_3_0
set DEBUGDIR=Win32Debug
set BINDIR=Win32Debug
set GlastEventFactoriesShr=%GLASTEVENTROOT%\%DEBUGDIR%\libGlastEventFactories
set HTL_DIR=%LHCXX_DIR%\HTL\1.3.0.1
set GaudiSvcShr=%GAUDISVCROOT%\%DEBUGDIR%\libGaudiSvc
set AuditorsShr=$(AuditorsDir)\libAuditors
set ROOT_PATH=%ROOT_DIR%\2.25.03
set ROOTSYS=%ROOT_PATH%
set RootHistCnvShr=%ROOTHISTCNVROOT%\%BINDIR%\libRootHistCnv
set GlastSvcShr=%GLASTSVCROOT%\%DEBUGDIR%\libGlastSvc
set DetDisplayShr=%DETDISPLAYROOT%\%DEBUGDIR%\libDetDisplay
set CLASSPATH=%CMTROOT%\java
set PATH=%GEOMETRYROOT%\%CMTCONFIG%;%GEOMREPROOT%\%CMTCONFIG%;%FACILITIESROOT%\%CMTCONFIG%;%GISMOROOT%\%CMTCONFIG%;%XMLEXT_DIR%\bin;%GAUDIROOT%\%BINDIR%;%HTL_DIR%\lib;%ROOT_PATH%\bin;%ROOT_PATH%\lib;%PATH%