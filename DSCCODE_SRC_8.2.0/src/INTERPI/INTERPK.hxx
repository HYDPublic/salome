#ifndef _INTERPK_HXX_
#define _INTERPK_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"

class INTERPK_i:
  public virtual POA_DSCCODE::INTERPI,
  public virtual Superv_Component_i
{
  public:
    INTERPK_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~INTERPK_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun();
    void trun();
    void perma(Superv_Component_i * component);
    void transit(Superv_Component_i * component);
};

extern "C"
{
    PortableServer::ObjectId * INTERPKEngine_factory( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::ObjectId * contId, 
                                                      const char *instanceName, const char *interfaceName);
}
#endif
