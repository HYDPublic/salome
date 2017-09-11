#ifndef _DSCCODDENG_HXX_
#define _DSCCODDENG_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"
#include "data_short_port_uses.hxx"
#include "data_short_port_provides.hxx"

class DSCCODD_i:
  public virtual POA_DSCCODE::DSCCODD,
  public virtual Superv_Component_i
{
  public:
    DSCCODD_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~DSCCODD_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun(CORBA::Long niter);
    void trun(CORBA::Long niter);
};

extern "C"
{
    PortableServer::ObjectId * DSCCODDEngine_factory( CORBA::ORB_ptr orb, 
                                                      PortableServer::POA_ptr poa, 
                                                      PortableServer::ObjectId * contId,
	                                                    const char *instanceName, 
                                                      const char *interfaceName);
}
#endif
