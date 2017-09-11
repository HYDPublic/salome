using namespace std;
#include "SOLIDE.hxx"
#include <string>
#include <unistd.h>

#include <Calcium.hxx>
#include <CalciumInterface.hxx>
#include <calcium.h>

extern "C" void transit_(void *compo,double *dt);
extern "C" void perma_(void *compo);

//! Constructor for component "SOLIDE" instance
/*!
 *  
 */
SOLIDE_i::SOLIDE_i(CORBA::ORB_ptr orb, 
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

//! Destructor for component "SOLIDE" instance
SOLIDE_i::~SOLIDE_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
SOLIDE_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "SOLIDE: prun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","puissi");
          p1->setDependencyType(CalciumTypes::ITERATION_DEPENDENCY);
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","tfi");
          p1->setDependencyType(CalciumTypes::ITERATION_DEPENDENCY);
          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tempi");
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tpi");
          add_port<calcium_integer_port_uses>("CALCIUM_integer","uses","iconv");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "SOLIDE: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "SOLIDE: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "SOLIDE: trun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          calcium_integer_port_provides * p2;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","rext");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","puissa");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","text");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          p2 = add_port<calcium_integer_port_provides>("CALCIUM_integer","provides","topo");
          p2->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tparoi");
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","rparoi");
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","temperature");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "SOLIDE: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "SOLIDE: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void SOLIDE_i::prun()
{
  std::cerr << "SOLIDE_i::prun" << std::endl;
  beginService("SOLIDE_i::prun");
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
  endService("SOLIDE_i::prun");
  std::cerr << "end of SOLIDE_i::prun" << std::endl;
}

void SOLIDE_i::trun(CORBA::Double dt)
{
  std::cerr << "SOLIDE_i::trun" << std::endl;
  beginService("SOLIDE_i::trun");
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
  endService("SOLIDE_i::trun");
  std::cerr << "end of SOLIDE_i::trun" << std::endl;
}

extern "C"
{
  PortableServer::ObjectId * SOLIDEEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * SOLIDEEngine_factory()");
    SOLIDE_i * myEngine = new SOLIDE_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
