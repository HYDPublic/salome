using namespace std;
#include "DSCCODCENG.hxx"
#include <string>
#include <unistd.h>
#include "CalciumInterface.hxx"
#include <calcium.h>

//! Constructor for component "DSCCODC" instance
/*!
 *  
 */
DSCCODC_i::DSCCODC_i(CORBA::ORB_ptr orb, 
                     PortableServer::POA_ptr poa, 
                     PortableServer::ObjectId * contId, 
                     const char *instanceName, 
                     const char *interfaceName) 
          : Superv_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  cerr << "create component" << endl;
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
}

//! Destructor for component "DSCCODC" instance
DSCCODC_i::~DSCCODC_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
DSCCODC_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "DSCCODC: prun: "  << std::endl;

          //initialization CALCIUM ports
          calcium_integer_port_provides * p1;
          /* ETP_EN  T       IN      ENTIER */
          p1 = add_port<calcium_integer_port_provides>("CALCIUM_integer","provides","ETP_EN");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          /* STP_EN  T       OUT     ENTIER */
          add_port<calcium_integer_port_uses>("CALCIUM_integer","uses","STP_EN");
          //end of initialization CALCIUM ports
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "DSCCODC: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODC: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "DSCCODC: trun: "  << std::endl;
          calcium_real_port_provides * p1;
          /* ETP_RE  T       IN      REEL   */
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","ETP_RE");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          /* STP_RE  T       OUT     REEL   */
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","STP_RE");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "DSCCODC: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODC: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void DSCCODC_i::prun(CORBA::Long niter)
{
  cerr << "DSCCODC_i::prun" << endl;
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  std::cerr << "Valeur de component : " << component << std::endl;
  std::cerr << "Valeur de this : " << this << std::endl;
  char       nom_instance[INSTANCE_LEN];
  int   SDATA_EN [3]; // buffer
  int   EDATA_EN [3]; // buffer

  int info = cp_cd(component,nom_instance);

  float ti_re = 0.0; // step time
  int  i = 0; // not used
  SDATA_EN[0] = 3;
  SDATA_EN[1] = 1;
  SDATA_EN[2] = 2;
  info = cp_een(component,CP_TEMPS,ti_re,i,(char *)"STP_EN",2,SDATA_EN);

  float tf_re = 1.0; // step start time
  int max=3; // max size expected
  int n; // real size received

  info = cp_len(component,CP_TEMPS,&ti_re,&tf_re,&i,(char *)"ETP_EN",max,&n,EDATA_EN);

  for (int i = 0; i < n; ++i)
    cerr << "seqLongData[" << i << "] = " << EDATA_EN[i] << endl;

  cerr << "end of DSCCODC_i::prun" << endl;
}

void DSCCODC_i::trun(CORBA::Long niter)
{
  cerr << "DSCCODC_i::trun" << endl;
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);

  float     SDATA_RE [3], EDATA_RE [3];
  SDATA_RE[0] = 3.;
  SDATA_RE[1] = 1.;
  SDATA_RE[2] = 2.;

  float ti_re = 0.0; // time
  int  i = 0; // not used

  info = cp_ere(component,CP_TEMPS,ti_re,i,(char *)"STP_RE",2,SDATA_RE);
  cerr << "apres cp_ere: " << info << endl;

  int max=3; // max size expected
  int n=2; // real size received
  float tf_re = 1.0; // time
  info = cp_lre(component,CP_TEMPS,&ti_re,&tf_re,&i,(char *)"ETP_RE",max,&n,EDATA_RE);

  for (int i = 0; i < n; ++i)
    cerr << "seqRealData[" << i << "] = " << EDATA_RE[i] << endl;
  cerr << "end of DSCCODC_i::trun" << endl;
}

extern "C"
{
  PortableServer::ObjectId * DSCCODCEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * DSCCODCEngine_factory()");
    DSCCODC_i * myEngine = new DSCCODC_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
