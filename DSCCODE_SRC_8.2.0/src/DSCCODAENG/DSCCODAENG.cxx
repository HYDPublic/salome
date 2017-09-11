using namespace std;
#include "DSCCODAENG.hxx"
#include <string>
#include <unistd.h>

//! Constructor for component "DSCCODA" instance
/*!
 *  
 */
DSCCODA_i::DSCCODA_i(CORBA::ORB_ptr orb, 
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

//! Destructor for component "DSCCODA" instance
DSCCODA_i::~DSCCODA_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
DSCCODA_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "DSCCODA: prun: uses "  << std::endl;
          add_port("BASIC_short", "uses", "prun_out_port");
          std::cerr << "DSCCODA: prun: provides "  << std::endl;
          add_port("BASIC_short", "provides", "prun_in_port");
        }
      //catch(const PortAlreadyDefined& ex)
      catch(PortAlreadyDefined ex)
        {
          std::cerr << "DSCCODA: deja defini" << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODA: exception inconnue" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "DSCCODA: trun: uses "  << std::endl;
          add_port("BASIC_short", "uses", "trun_out_port");
          std::cerr << "DSCCODA: trun: provides "  << std::endl;
          add_port("BASIC_short", "provides", "trun_in_port");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "DSCCODA: deja defini" << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "DSCCODA: exception inconnue" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void DSCCODA_i::prun(CORBA::Long niter)
{
  cout << "DSCCODA_i::prun" << endl;
  uses_port * temp = NULL;
  get_port(temp, "prun_out_port");
  _out_port = dynamic_cast<data_short_port_uses *>(temp);
  //get_port((uses_port*&)_out_port, "run_out_port");

  provides_port * temp2 = NULL;
  get_port(temp2, "prun_in_port");
  _in_port = dynamic_cast<data_short_port_provides *>(temp2);

  int j=0;
  _out_port->put(j);
  for(int i=0;i<niter;i++)
    {
      j=_in_port->get();
      cout << "DSCCODA Got: " << j << endl;
      usleep(100000);
      j=j+5;
      _out_port->put(j);
    }
  j=_in_port->get();
  cout << "DSCCODA Got: " << j << endl;
}
void DSCCODA_i::trun(CORBA::Long niter)
{
  cout << "DSCCODA_i::trun" << endl;
  uses_port * temp = NULL;
  get_port(temp, "trun_out_port");
  _out_port = dynamic_cast<data_short_port_uses *>(temp);
  //get_port((uses_port*&)_out_port, "run_out_port");

  provides_port * temp2 = NULL;
  get_port(temp2, "trun_in_port");
  _in_port = dynamic_cast<data_short_port_provides *>(temp2);

  int j=0;
  _out_port->put(j);
  for(int i=0;i<niter;i++)
    {
      j=_in_port->get();
      cout << "DSCCODA Got: " << j << endl;
      usleep(100000);
      j=j+5;
      _out_port->put(j);
    }
  j=_in_port->get();
  cout << "DSCCODA Got: " << j << endl;
}

extern "C"
{
  PortableServer::ObjectId * DSCCODAEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
			                            const char *instanceName, 
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * DSCCODAEngine_factory()");
    DSCCODA_i * myEngine = new DSCCODA_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
