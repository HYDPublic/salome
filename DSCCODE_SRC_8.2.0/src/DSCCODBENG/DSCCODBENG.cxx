using namespace std;
#include "DSCCODBENG.hxx"
#include <string>
#include <unistd.h>

//! Constructor for component "DSCCODB" instance
/*!
 *  
 */
DSCCODB_i::DSCCODB_i(CORBA::ORB_ptr orb, 
                     PortableServer::POA_ptr poa, 
                     PortableServer::ObjectId * contId, 
                     const char *instanceName, 
                     const char *interfaceName) 
                 : Superv_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  cout << "create component" << endl;
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
  _in_port=NULL;
  _out_port=NULL;
}

//! Destructor for component "DSCCODB" instance
DSCCODB_i::~DSCCODB_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
DSCCODB_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "DSCCODB: prun: uses "  << std::endl;
          add_port("BASIC_short", "uses", "prun_out_port");
          std::cerr << "DSCCODB: prun: provides "  << std::endl;
          add_port("BASIC_short", "provides", "prun_in_port");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "DSCCODB: prun: deja defini "  << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODB: exception inconnue" << std::endl;
        }
      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "DSCCODB: trun: uses "  << std::endl;
          add_port("BASIC_short", "uses", "trun_out_port");
          std::cerr << "DSCCODB: trun: provides "  << std::endl;
          add_port("BASIC_short", "provides", "trun_in_port");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "DSCCODB: trun: deja defini "  << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODB: exception inconnue" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

//! Main method for service "prun"
void DSCCODB_i::prun(CORBA::Long niter)
{
  cout << "DSCCODB_i::prun" << endl;
  uses_port * temp = NULL;
  get_port(temp, "prun_out_port");
  _out_port = dynamic_cast<data_short_port_uses *>(temp);
  //get_port((uses_port*&)_out_port, "prun_out_port");

  provides_port * temp2 = NULL;
  get_port(temp2, "prun_in_port");
  _in_port = dynamic_cast<data_short_port_provides *>(temp2);

  int j=0;
  _out_port->put(j);
  for(int i=0;i<niter;i++)
    {
      j=_in_port->get();
      cout << "DSCCODB Got: " << j << endl;
      usleep(100000);
      j=2*j;
      _out_port->put(j);
    }
  j=_in_port->get();
  cout << "DSCCODB Got: " << j << endl;
}

//! Main method for service "trun"
void DSCCODB_i::trun(CORBA::Long niter)
{
  cout << "DSCCODB_i::trun" << endl;
  uses_port * temp = NULL;
  get_port(temp, "trun_out_port");
  _out_port = dynamic_cast<data_short_port_uses *>(temp);
  //get_port((uses_port*&)_out_port, "trun_out_port");

  provides_port * temp2 = NULL;
  get_port(temp2, "trun_in_port");
  _in_port = dynamic_cast<data_short_port_provides *>(temp2);

  int j=0;
  _out_port->put(j);
  for(int i=0;i<niter;i++)
    {
      j=_in_port->get();
      cout << "DSCCODB Got: " << j << endl;
      usleep(100000);
      j=2*j;
      _out_port->put(j);
    }
  j=_in_port->get();
  cout << "DSCCODB Got: " << j << endl;
}


extern "C"
{
//! Factory for component "DSCCODB"
  PortableServer::ObjectId * DSCCODBEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * DSCCODBEngine_factory()");
    DSCCODB_i * myEngine = new DSCCODB_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
