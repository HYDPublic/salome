#ifndef _INTERPI_HXX_
#define _INTERPI_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"

class INTERPI_i:
  public virtual POA_DSCCODE::INTERPI,
  public virtual Superv_Component_i
{
  public:
    INTERPI_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~INTERPI_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun();
    void trun();
};

extern "C"
{
    PortableServer::ObjectId * INTERPIEngine_factory( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::ObjectId * contId, const char *instanceName, const char *interfaceName);
    PortableServer::ObjectId * INTERPJEngine_factory( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::ObjectId * contId, const char *instanceName, const char *interfaceName);
    PortableServer::ObjectId * INTERPKEngine_factory( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::ObjectId * contId, const char *instanceName, const char *interfaceName);
}
#endif
