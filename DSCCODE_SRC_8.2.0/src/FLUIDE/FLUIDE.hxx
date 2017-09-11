#ifndef _FLUIDE_HXX_
#define _FLUIDE_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"

class FLUIDE_i:
  public virtual POA_DSCCODE::FLUIDE,
  public virtual Superv_Component_i
{
  public:
    FLUIDE_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~FLUIDE_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun();
    void trun(CORBA::Double dt);
};

extern "C"
{
    PortableServer::ObjectId * FLUIDEEngine_factory( CORBA::ORB_ptr orb, 
                                                      PortableServer::POA_ptr poa, 
                                                      PortableServer::ObjectId * contId,
	                                                    const char *instanceName, 
                                                      const char *interfaceName);
}
#endif
