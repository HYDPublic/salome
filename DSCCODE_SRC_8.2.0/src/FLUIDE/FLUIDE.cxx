#include "FLUIDE.hxx"
#include <string>
#include <unistd.h>

#include <Calcium.hxx>
#include <CalciumException.hxx>
#include <calcium.h>

using namespace std;

extern "C" void transit_(void *compo,double *dt);
extern "C" void perma_(void *compo);

//! Constructor for component "FLUIDE" instance
/*!
 *  
 */
FLUIDE_i::FLUIDE_i(CORBA::ORB_ptr orb, 
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

//! Destructor for component "FLUIDE" instance
FLUIDE_i::~FLUIDE_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
FLUIDE_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "FLUIDE: prun: "  << std::endl;
          //initialization CALCIUM ports IN
          create_calcium_port(this,(char *)"tpi",(char *)"CALCIUM_real",(char *)"IN",(char *)"I");
          create_calcium_port(this,(char *)"iconv",(char *)"CALCIUM_integer",(char *)"IN",(char *)"I");
          //initialization CALCIUM ports OUT
          create_calcium_port(this,(char *)"tfi",(char *)"CALCIUM_real",(char *)"OUT",(char *)"I");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "FLUIDE: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "FLUIDE: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "FLUIDE: trun: "  << std::endl;
          //initialization CALCIUM ports IN
          create_calcium_port(this,(char *)"tparoi",(char *)"CALCIUM_real",(char *)"IN",(char *)"T");
          create_calcium_port(this,(char *)"rparoi",(char *)"CALCIUM_real",(char *)"IN",(char *)"T");
          //initialization CALCIUM ports OUT
          create_calcium_port(this,(char *)"tfluide",(char *)"CALCIUM_real",(char *)"OUT",(char *)"T");
          create_calcium_port(this,(char *)"rfluide",(char *)"CALCIUM_real",(char *)"OUT",(char *)"T");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "FLUIDE: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "FLUIDE: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void FLUIDE_i::prun()
{
  std::cerr << "FLUIDE_i::prun" << std::endl;
  beginService("FLUIDE_i::prun");
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);
  std::cerr << nom_instance << std::endl;
  try
    {
      perma_(&component);
      //to do or not to do ???
      cp_fin(component,CP_ARRET);
    }
  catch ( const CalciumException & ex)
    {
      std::cerr << ex.what() << std::endl;
      cp_fin(component,CP_ARRET);
    }
  catch (...)
    {
      std::cerr << "unexpected exception" << std::endl;
      cp_fin(component,CP_ARRET);
    }
  endService("FLUIDE_i::prun");
  std::cerr << "end of FLUIDE_i::prun" << std::endl;
}

void FLUIDE_i::trun(CORBA::Double dt)
{
  std::cerr << "FLUIDE_i::trun" << std::endl;
  beginService("FLUIDE_i::trun");
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);
  try
    {
      transit_(&component,&dt);
      //to do or not to do ???
      cp_fin(component,CP_ARRET);
    }
  catch ( const CalciumException & ex)
    {
      std::cerr << ex.what() << std::endl;
      cp_fin(component,CP_ARRET);
    }
  catch (...)
    {
      std::cerr << "unexpected exception" << std::endl;
      cp_fin(component,CP_ARRET);
    }
  endService("FLUIDE_i::trun");
  std::cerr << "end of FLUIDE_i::trun" << std::endl;
}

extern "C"
{
  PortableServer::ObjectId * FLUIDEEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * FLUIDEEngine_factory()");
    FLUIDE_i * myEngine = new FLUIDE_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
