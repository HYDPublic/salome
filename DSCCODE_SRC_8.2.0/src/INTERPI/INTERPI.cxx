using namespace std;
#include "INTERPI.hxx"
#include <string>
#include <unistd.h>

#include <Calcium.hxx>
#include <CalciumInterface.hxx>
#include <calcium.h>

extern "C" void transit_(void *compo);
extern "C" void perma_(void *compo);

//! Constructor for component "INTERPI" instance
/*!
 *  
 */
INTERPI_i::INTERPI_i(CORBA::ORB_ptr orb, 
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

//! Destructor for component "INTERPI" instance
INTERPI_i::~INTERPI_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
INTERPI_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "INTERPI: prun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","tparoi");
          p1->setDependencyType(CalciumTypes::ITERATION_DEPENDENCY);
          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tpar");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "INTERPI: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "INTERPI: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "INTERPI: trun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","tparoit");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tpart");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "INTERPI: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "INTERPI: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void INTERPI_i::prun()
{
  std::cerr << "INTERPI_i::prun" << std::endl;
  beginService("INTERPI_i::prun");
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);
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
  endService("INTERPI_i::prun");
  std::cerr << "end of INTERPI_i::prun" << std::endl;
}

void INTERPI_i::trun()
{
  std::cerr << "INTERPI_i::trun" << std::endl;
  beginService("INTERPI_i::trun");
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);
  try
    {
      transit_(&component);
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
  endService("INTERPI_i::trun");
  std::cerr << "end of INTERPI_i::trun" << std::endl;
}

extern "C"
{
  PortableServer::ObjectId * INTERPIEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
                                                    const char *instanceName, 
                                                    const char *interfaceName)
  {
    std::cerr << "INTERPIEngine_factory" << std::endl;
    INTERPI_i * myEngine = new INTERPI_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }

  PortableServer::ObjectId * INTERPJEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
                                                    const char *instanceName, 
                                                    const char *interfaceName)
  {
    std::cerr << "INTERPJEngine_factory" << std::endl;
    INTERPI_i * myEngine = new INTERPI_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
  PortableServer::ObjectId * INTERPKEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
                                                    const char *instanceName, 
                                                    const char *interfaceName)
  {
    std::cerr << "INTERPKEngine_factory" << std::endl;
    INTERPI_i * myEngine = new INTERPI_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
