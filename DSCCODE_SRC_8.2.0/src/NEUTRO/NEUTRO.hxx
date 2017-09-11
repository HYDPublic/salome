#ifndef _NEUTRO_HXX_
#define _NEUTRO_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"

class NEUTRO_i:
  public virtual POA_DSCCODE::NEUTRO,
  public virtual Superv_Component_i
{
  public:
    NEUTRO_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~NEUTRO_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun();
    void trun(CORBA::Double dt);
};

extern "C"
{
    PortableServer::ObjectId * NEUTROEngine_factory( CORBA::ORB_ptr orb, 
                                                      PortableServer::POA_ptr poa, 
                                                      PortableServer::ObjectId * contId,
	                                                    const char *instanceName, 
                                                      const char *interfaceName);
}
#endif
