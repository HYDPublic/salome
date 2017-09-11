#ifndef _SOLIDE_HXX_
#define _SOLIDE_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"

class SOLIDE_i:
  public virtual POA_DSCCODE::SOLIDE,
  public virtual Superv_Component_i
{
  public:
    SOLIDE_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~SOLIDE_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun();
    void trun(CORBA::Double dt);
};

extern "C"
{
    PortableServer::ObjectId * SOLIDEEngine_factory( CORBA::ORB_ptr orb, 
                                                      PortableServer::POA_ptr poa, 
                                                      PortableServer::ObjectId * contId,
	                                                    const char *instanceName, 
                                                      const char *interfaceName);
}
#endif
