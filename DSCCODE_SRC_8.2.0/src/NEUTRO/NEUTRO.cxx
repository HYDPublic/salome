using namespace std;
#include "NEUTRO.hxx"
#include <string>
#include <unistd.h>

#include <Calcium.hxx>
#include <CalciumInterface.hxx>
#include <calcium.h>

extern "C" void transit_(void *compo,double *dt);
extern "C" void perma_(void *compo);

//! Constructor for component "NEUTRO" instance
/*!
 *  
 */
NEUTRO_i::NEUTRO_i(CORBA::ORB_ptr orb, 
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

//! Destructor for component "NEUTRO" instance
NEUTRO_i::~NEUTRO_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
NEUTRO_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "NEUTRO: prun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","tempi");
          p1->setDependencyType(CalciumTypes::ITERATION_DEPENDENCY);
          calcium_integer_port_provides * p2;
          p2 = add_port<calcium_integer_port_provides>("CALCIUM_integer","provides","iconv");
          p2->setDependencyType(CalciumTypes::ITERATION_DEPENDENCY);

          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","puissi");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "NEUTRO: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "NEUTRO: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "NEUTRO: trun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","temperature");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","puissance");
          add_port<calcium_integer_port_uses>("CALCIUM_integer","uses","topo");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "NEUTRO: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "NEUTRO: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void NEUTRO_i::prun()
{
  std::cerr << "NEUTRO_i::prun" << std::endl;
  beginService("NEUTRO_i::prun");
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
  endService("NEUTRO_i::prun");
  std::cerr << "end of NEUTRO_i::prun" << std::endl;
}

void NEUTRO_i::trun(CORBA::Double dt)
{
  std::cerr << "NEUTRO_i::trun" << std::endl;
  beginService("NEUTRO_i::trun");
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
  endService("NEUTRO_i::trun");
  std::cerr << "end of NEUTRO_i::trun" << std::endl;
}

extern "C"
{
  PortableServer::ObjectId * NEUTROEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * NEUTROEngine_factory()");
    NEUTRO_i * myEngine = new NEUTRO_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
