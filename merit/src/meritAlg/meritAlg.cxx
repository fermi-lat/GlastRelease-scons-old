/** @file meritAlg.cxx
@brief Declaration and implementation of meritAlg

$Header$
*/
// Include files

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"


#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/MCEvent.h"
#include "Event/MonteCarlo/Exposure.h"
#include "LdfEvent/Gem.h"
#include "LdfEvent/EventSummaryData.h"

#include "AnalysisNtuple/IValsTool.h"

#include "FigureOfMerit.h"
#include "analysis/Tuple.h"

#include "facilities/Util.h" // for expandEnvVar    

#include "GuiSvc/IGuiSvc.h"
#include "gui/DisplayControl.h"
#include "gui/PrintControl.h"
#include "gui/GuiMgr.h"

#include "ntupleWriterSvc/INTupleWriterSvc.h"

#include "OnboardFilter/FilterStatus.h"
#include "OnboardFilter/FilterAlgTds.h"

#include <sstream>
#include <algorithm>
#include <map>
#include <numeric>
#include <cassert>

static std::string  default_cuts("LntA");

namespace gui 
{
#include "gui/Command.h"

    /** @class PrinterByPrefix_T
    *    @brief template class for adding a printer to the gui menu that
    *           depends on a prefix (actually any string)
    */
    template<class T> 
    class PrinterByPrefix_T : public Command 
    {
    public:
        PrinterByPrefix_T(const T* t, std::string prefix, std::ostream& out=std::cout)
            : m_t(t),m_out(out), m_prefix(prefix){}
            void execute(){m_t->printOnByPrefix(m_out, m_prefix);}
    private:
        const T* m_t;
        std::ostream& m_out;
        std::string m_prefix;
    };  
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class meritAlg
@brief Apply the merit processing as an Algorithm, constructing a NTuple
*/
class meritAlg : public Algorithm {

public:
    meritAlg(const std::string& name, ISvcLocator* pSvcLocator); 

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

    void printOn(std::ostream& out)const;
    ///Print the fields in the merit ntuple, that contain the string 'prefix' 
    void printOnByPrefix(std::ostream& out, std::string prefix)const;
private:

    StatusCode setupTools();

    //! add an item to the MeritTuple

    void addItem(const std::string & itemName,  double* pdouble)
    {
        new TupleItem(itemName, pdouble);

    }
    void addItem(const std::string & itemName,  float* pfloat)
    {
        new TupleItem(itemName, pfloat);
    }
    void addItem(const std::string & itemName,  int* pint)
    {
    }

    void calculate(); 

    /** nested class to manage extra TTrees 
    */
    class TTree { 
    public:
        /**
        */
        TTree(     INTupleWriterSvc* rootTupleSvc, 
            std::string treeName,  
            std::vector<const char* >leaf_names)
            :m_name(treeName), m_leafNames(leaf_names)
        {
            m_values.resize(leaf_names.size());
            int i=0;
            for( std::vector<const char*>::const_iterator  it = leaf_names.begin();
                it!=leaf_names.end(); 
                ++it){
                    rootTupleSvc->addItem(treeName,  *it,  &m_values[i++]);
                }
        }
        void fill(int n, float value){ m_values[n]=value;};

        /** called from gui printer */
        void printOn(std::ostream& out)const {
            out << "Tree "<< m_name << std::endl;
            std::vector<float>::const_iterator dit=m_values.begin();
            for( std::vector<const char*> ::const_iterator nit=m_leafNames.begin(); nit!=m_leafNames.end(); ++nit,++dit){
                out << std::setw(15) << *nit << "  " <<  *dit << std::endl;
            }
        }
        /// data values: name of tree, leaves, and the values
        std::string m_name;
        std::vector<const char*> m_leafNames;
        std::vector< float> m_values;
    };
    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    INTupleWriterSvc* m_rootTupleSvc;;

    IToolSvc* m_pToolSvc;

    // places to put stuff found in the TDS
    float m_run, m_event, m_mc_src_id;

    double m_time;
    double m_statusHi, m_statusLo,m_separation;
    double m_filterAlgStatus;


    int m_generated;
    int m_warnNoFilterStatus;   // count WARNINGs: no FilterStatus found
    /// Common interface to analysis tools
    std::vector<IValsTool*> m_toolvec;
    /// the various tree names
    StringProperty m_eventTreeName;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<meritAlg>  Factory;
const IAlgFactory& meritAlgFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
meritAlg::meritAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator), m_tuple(0), m_rootTupleSvc(0)
{

    declareProperty("cuts" , m_cuts=default_cuts);
    declareProperty("generated" , m_generated=10000);
    declareProperty("EventTreeName",     m_eventTreeName="MeritTuple");

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StatusCode meritAlg::setupTools() {
    MsgStream log(msgSvc(), name());
    // set up tools
    IToolSvc* pToolSvc = 0;

    StatusCode sc = service("ToolSvc", pToolSvc, true);
    if (!sc.isSuccess ()){
        log << MSG::ERROR << "Can't find ToolSvc, will quit now" << endreq;
        return StatusCode::FAILURE;
    }

    //@@@FP 07/08/05
    //    const char * toolnames[] = {"McValsTool", "GltValsTool", "TkrValsTool", 
    //        "VtxValsTool", "CalValsTool", "AcdValsTool", "EvtValsTool"};

    const char * toolnames[] = {"McValsTool", "GltValsTool", "TkrValsTool", 
        "VtxValsTool", "CalValsTool", "AcdValsTool", "EvtValsTool","CalMipValsTool"};
    //@@@FP 07/08/05

    for( int i =0; i< (int)(sizeof(toolnames)/sizeof(void*)); ++i){
        m_toolvec.push_back(0);
        sc = pToolSvc->retrieveTool(toolnames[i], m_toolvec.back());
        if( sc.isFailure() ) {
            log << MSG::ERROR << "Unable to find a  tool" << toolnames[i] << endreq;
            return sc;
        }
    }
    //------------------------
    // nested vistor class to put Bill's tuple values into our tuple
    class VisitBill : virtual public IValsTool::Visitor
    {
    public:
        VisitBill( meritAlg* me) : m_merit(me){}
        IValsTool::Visitor::eVisitorRet analysisValue(std::string varName, const double& value) const
        {
            double * val = const_cast<double*>(&value);
            m_merit->addItem( varName, val);
            return IValsTool::Visitor::CONT;
        }
        IValsTool::Visitor::eVisitorRet analysisValue(std::string varName, const float& value) const
        {
            float * val = const_cast<float*>(&value);
            m_merit->addItem( varName, val);
            return IValsTool::Visitor::CONT;
        }
        IValsTool::Visitor::eVisitorRet analysisValue(std::string , const int& ) const
        {
            // ignore, for now
            return IValsTool::Visitor::CONT;
        }
        IValsTool::Visitor::eVisitorRet analysisValue(std::string , const unsigned int& ) const
        {
            // ignore, for now
            return IValsTool::Visitor::CONT;
        }

    private:
        meritAlg* m_merit;
    };
    //----------------------

    VisitBill visitor(this);
    for( std::vector<IValsTool*>::iterator it =m_toolvec.begin(); it != m_toolvec.end(); ++it){
        if( (*it)->traverse(&visitor,false)==IValsTool::Visitor::ERROR) {
            log << MSG::ERROR << *it << " traversal failed" << endreq;
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StatusCode meritAlg::initialize() {
    StatusCode  sc = StatusCode::SUCCESS;

    MsgStream log(msgSvc(), name());

    // Use the Job options service to get the Algorithm's parameters
    setProperties();
    m_warnNoFilterStatus = 0;   // Zero counter for warnings

    // get a pointer to RootTupleSvc 
    if( (sc = service("RootTupleSvc", m_rootTupleSvc, true) ). isFailure() ) {
        log << MSG::ERROR << " failed to get the RootTupleSvc" << endreq;
        return sc;
    }
    // set up the local tuple
    std::stringstream title;
    title << "TDS: gen(" << m_generated << ")";
    m_tuple = new Tuple(title.str());

    /** @page MeritTuple
  	       
      @section Filter FilterAlg Variables

      @verbatim

      Name                          Bit  Explanation
      
   DFC_V_STATUS_ACD                   0  ACD was analyzed                
   DFC_V_STATUS_DIR           	      1	 DIR was decoded                 
   DFC_V_STATUS_ATF            	      2	 ACD/TKR veto was analyzed       
   DFC_V_STATUS_CAL1          	      3  CAL was analyzed, phase 1       
   DFC_V_STATUS_TKR           	      4  TKR finding was done            
   DFC_V_STATUS_ACD_TOP     	      5  ACD top  tile struck            
   DFC_V_STATUS_ACD_SIDE     	      6  ACD side tile struck            
   DFC_V_STATUS_ACD_SIDE_FILTER       7	 ACD      filter tile struck     
   DFC_V_STATUS_TKR_POSSIBLE          8	 Possible track                  
   DFC_V_STATUS_TKR_TRIGGER           9	 Have a 3-in-a-row trigger       
   DFC_V_STATUS_CAL_LO         	     10  Cal Lo Trigger                  
   DFC_V_STATUS_CAL_HI         	     11  Cal Hi Trigger                  
   DFC_V_STATUS_TKR_EQ_1       	     12  Exactly 1 track                 
   DFC_V_STATUS_TKR_GE_2             13	 Greater or equal 2 tracks       
   DFC_V_STATUS_TKR_THROTTLE         14	 Throttle bit set                

   DFC_V_STATUS_TKR_LT_2_ELO         15  Low energy, no 2 track evidence   
   DFC_V_STATUS_TKR_SKIRT            16  Event into the skirt region     
   DFC_V_STATUS_TKR_EQ_0             17  No tracks                       
   DFC_V_STATUS_TKR_ROW2       	     18  Track Row 2 match               
   DFC_V_STATUS_TKR_ROW01            19  Track Row 0 or 1 match          
   DFC_V_STATUS_TKR_TOP              20  Track Top match                 
   DFC_V_STATUS_ZBOTTOM        	     21  No tracks into CAL with energy  
   DFC_V_STATUS_EL0_ETOT_90    	     22  E layer 0/ETOT > .90            
   DFC_V_STATUS_EL0_ETOT_01          23  E layer 0/ETOT < .01            
   DFC_V_STATUS_SIDE                 24	 Event has a side face veto      
   DFC_V_STATUS_TOP            	     25	 Event has a top  face veto      
   DFC_V_STATUS_SPLASH_1       	     26  Event has a splash veto         
   DFC_V_STATUS_E350_FILTER_TILE     27  Event <350Mev  + filter tiles   
   DFC_V_STATUS_E0_TILE              28  Event 0 energy + tile hit       
   DFC_V_STATUS_SPLASH_0             29  Event has a splash veto         
   DFC_V_STATUS_NOCALLO_FILTER_TILE  30  No CAL LO trigger + filter tile 
   DFC_V_STATUS_VETOED               31  Any veto                        
@endverbatim
<table>
<tr><th> Variable      </th><th> Description</th></tr>

<tr><td>  FilterStatus_HI
</td><td>     bits 15-31 of the filter status word 
</td></tr>
<tr><td>  FilterStatus_LO
</td><td>    bits  0-14 of the filter status word 
</td></tr>
<tr><td>  FilterAlgStatus
</td><td>Status generated by FilterAlg 
</td></tr>
<tr><td> FilterAngSep   
</td><td>    Filter status separation </td></tr>.

</table> 

*/
#if 0 // old: put in local tuple first
    addItem( "FilterStatus_HI",   &m_statusHi );
    addItem( "FilterStatus_LO",   &m_statusLo );

    addItem( "FilterAlgStatus",  &m_filterAlgStatus );
    addItem( "FilterAngSep",     &m_separation );
#else // add directly to the output tuple
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterStatus_HI",   &m_statusHi);
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterStatus_LO",  &m_statusLo );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterAlgStatus",  &m_filterAlgStatus );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterAngSep",     &m_separation );

#endif

    // add some of the AnalysisNTuple items
    if( setupTools().isFailure()) return StatusCode::FAILURE;

    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
#if 0 // AnalysisNtupleAlg now does this
    if( m_rootTupleSvc !=0) {
        // now tell the root tuple service about our tuple
        for(Tuple::iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
            TupleItem& item = **tit;
            if( item.isFloat() ){
                m_rootTupleSvc->addItem(m_eventTreeName.value(), item.name(), item.pvalue()) ;
            }else {
                m_rootTupleSvc->addItem(m_eventTreeName.value(), item.name(), &item.value());
            }
        }
     }
#endif

    // setup tuple output via the print service
    // get the Gui service
    IGuiSvc* guiSvc=0;
    sc = service("GuiSvc", guiSvc);

    if (!sc.isSuccess ()){
        log << MSG::DEBUG << "No GuiSvc, so no interactive printout" << endreq;
        return StatusCode::SUCCESS;
    }

    gui::GuiMgr* guimgr = guiSvc->guiMgr();
    gui::PrintControl* printer = &guimgr->printer();
    printer->addPrinter("merit tuple",new gui::Printer_T<meritAlg>(this));

    printer->addPrinter("Acd tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Acd"));
    printer->addPrinter("Cal tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Cal"));
    printer->addPrinter("Tkr tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Tkr"));
    printer->addPrinter("Vtx tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Vtx"));
    printer->addPrinter("Evt tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Evt"));
    printer->addPrinter("Mc  tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Mc"));
    printer->addPrinter("Glt tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Glt"));
    printer->addPrinter("IM  tree", new gui::PrinterByPrefix_T<meritAlg>(this,"IM"));
    printer->addPrinter("FT1 tree", new gui::PrinterByPrefix_T<meritAlg>(this,"FT1"));
    printer->addPrinter("Pt tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Pt"));
    printer->addPrinter("Filter tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Filt"));

    return sc;
}

//------------------------------------------------------------------------------
void meritAlg::calculate(){

    for( std::vector<IValsTool*>::iterator i =m_toolvec.begin(); i != m_toolvec.end(); ++i){
        (*i)->doCalcIfNotDone();
    }
}
//------------------------------------------------------------------------------
void meritAlg::printOn(std::ostream& out)const{
    out << "Merit tuple, " << "$Revision$" << std::endl;

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        out << std::setw(25) << item.name() 
            << "  " << std::setprecision(4)<< double(item) << std::endl;
    }
}

//------------------------------------------------------------------------------
void meritAlg::printOnByPrefix(std::ostream& out, std::string prefix)const{
    out << "Merit tuple/Prefix= "<<prefix<< std::endl;

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        if(!item.name().find(prefix))
        {
            out << std::setw(25) << item.name() 
                << "  " << std::setprecision(4)<< double(item) << std::endl;
        }
    }
}

//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {

    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

#if 0
    calculate(); // setup Bill's tuple items
#endif
    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    SmartDataPtr<Event::MCEvent>     mcheader(eventSvc(),    EventModel::MC::Event);
 
    m_run = header->run();
    if( mcheader )    m_mc_src_id = mcheader->getSourceId();
    m_time = header->time();
    m_event = header->event();

    SmartDataPtr<OnboardFilterTds::FilterStatus> filterStatus(eventSvc(), "/Event/Filter/FilterStatus");
    if( filterStatus ){
        m_statusHi=filterStatus->getHigh();
        m_statusLo=filterStatus->getLow();
        m_separation=filterStatus->getSeparation();
    }else {
        m_statusHi = m_statusLo = 0;

        m_warnNoFilterStatus++;
        if (   m_warnNoFilterStatus <= 10 ) {
            log << MSG::WARNING << "FilterStatus not found" ;
            if ( m_warnNoFilterStatus == 10 ) {
                log << " -- Further WARNINGs on missing FilterStatus are suppressed"; }
            log  << endreq;
        }
    }
    SmartDataPtr<FilterAlgTds::FilterAlgData> filterAlgStatus(eventSvc(),"/Event/Filter/FilterAlgData");
    if(filterAlgStatus){
        m_filterAlgStatus=(double)filterAlgStatus->getVetoWord();
    }
    m_fm->execute();
    
    // always write the event tuple
    m_rootTupleSvc->storeRowFlag(this->m_eventTreeName.value(), true);

    return sc;
}
//------------------------------------------------------------------------------
StatusCode meritAlg::finalize() {

    MsgStream log(msgSvc(), name());
    log << MSG::INFO ;
    if(log.isActive()) {
        try {
            m_fm->report(log.stream());
        } catch (...){
            log << "Failure to generate full output due to unknown exception";
        }
    }
    log << endreq;
    if(m_warnNoFilterStatus>0)
        log << MSG::INFO << "Number of warnings (FilterStatus not found): "<< m_warnNoFilterStatus << endreq;

    delete m_tuple;
    delete m_fm;
    setFinalized(); //  prevent being called again

    return StatusCode::SUCCESS;
}


