using namespace std;
#include "DSCCODDENG.hxx"
#include <string>
#include <unistd.h>
#include "CalciumInterface.hxx"
#include <calcium.h>

//! Constructor for component "DSCCODD" instance
/*!
 *  
 */
DSCCODD_i::DSCCODD_i(CORBA::ORB_ptr orb, 
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

//! Destructor for component "DSCCODD" instance
DSCCODD_i::~DSCCODD_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
DSCCODD_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "DSCCODD: prun: "  << std::endl;
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
          std::cerr << "DSCCODD: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODD: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "DSCCODD: trun: "  << std::endl;
          calcium_real_port_provides * p1;
          /* ETP_RE  T       IN      REEL   */
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","ETP_RE");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          /* STP_RE  T       OUT     REEL   */
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","STP_RE");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "DSCCODD: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODD: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void DSCCODD_i::prun(CORBA::Long niter)
{
  cerr << "DSCCODD_i::prun" << endl;
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  std::cerr << "Valeur de component : " << component << std::endl;
  std::cerr << "Valeur de this : " << this << std::endl;
  char       nom_instance[INSTANCE_LEN];
  int   SDATA_EN [3]; // buffer
  int   EDATA_EN [3]; // buffer

  int info = cp_cd(component,nom_instance);

  int  i = 0; // not used
  float ti_re = 0.0; // step time
  float tf_re = 1.0; // step start time
  int max=3; // max size expected
  int n; // real size received

  info = cp_len(component,CP_TEMPS,&ti_re,&tf_re,&i,(char *)"ETP_EN",max,&n,EDATA_EN);

  for (int i = 0; i < n; ++i)
    {
      SDATA_EN[i]=EDATA_EN[i]*2;
      cerr << "seqLongData[" << i << "] = " << EDATA_EN[i] << endl;
    }

  info = cp_een(component,CP_TEMPS,ti_re,i,(char *)"STP_EN",n,SDATA_EN);
  cerr << "end of DSCCODD_i::prun" << endl;
}

void DSCCODD_i::trun(CORBA::Long niter)
{
  cerr << "DSCCODD_i::trun" << endl;
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);

  float     SDATA_RE [3], EDATA_RE [3];

  int  i = 0; // not used
  float ti_re = 0.0; // time
  int max=3; // max size expected
  int n=2; // real size received
  float tf_re = 1.0; // time

  info = cp_lre(component,CP_TEMPS,&ti_re,&tf_re,&i,(char *)"ETP_RE",max,&n,EDATA_RE);

  cerr << "apres cp_lre: " << n << endl;
  for (int i = 0; i < n; ++i)
    {
      SDATA_RE[i]=EDATA_RE[i]*2;
      cerr << "seqRealData[" << i << "] = " << EDATA_RE[i] << endl;
    }

  info = cp_ere(component,CP_TEMPS,ti_re,i,(char *)"STP_RE",n,SDATA_RE);
  cerr << "end of DSCCODD_i::trun" << endl;
}

extern "C"
{
  PortableServer::ObjectId * DSCCODDEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * DSCCODDEngine_factory()");
    DSCCODD_i * myEngine = new DSCCODD_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
