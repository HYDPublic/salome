#ifndef _DSCCODCENG_HXX_
#define _DSCCODCENG_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"

class DSCCODC_i:
  public virtual POA_DSCCODE::DSCCODC,
  public virtual Superv_Component_i
{
  public:
    DSCCODC_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~DSCCODC_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun(CORBA::Long niter);
    void trun(CORBA::Long niter);
};

extern "C"
{
    PortableServer::ObjectId * DSCCODCEngine_factory( CORBA::ORB_ptr orb, 
                                                      PortableServer::POA_ptr poa, 
                                                      PortableServer::ObjectId * contId,
	                                                    const char *instanceName, 
                                                      const char *interfaceName);
}
#endif
