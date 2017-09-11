#ifndef _DSCCODBENG_HXX_
#define _DSCCODBENG_HXX_

#include "Superv_Component_i.hxx"
#include "DSCCODE.hh"
#include "data_short_port_uses.hxx"
#include "data_short_port_provides.hxx"

class DSCCODB_i:
  public virtual POA_DSCCODE::DSCCODB,
  public virtual Superv_Component_i
{
  public:
    DSCCODB_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, 
              PortableServer::ObjectId * contId, 
	            const char *instanceName, const char *interfaceName);
    virtual ~DSCCODB_i();
    CORBA::Boolean init_service(const char * service_name);
    void prun(CORBA::Long niter);
    void trun(CORBA::Long niter);
  private:
    data_short_port_provides * _in_port;
    data_short_port_uses * _out_port;
};

extern "C"
{
    PortableServer::ObjectId * DSCCODBEngine_factory( CORBA::ORB_ptr orb, 
                                                      PortableServer::POA_ptr poa, 
                                                      PortableServer::ObjectId * contId,
	                                                    const char *instanceName, 
                                                      const char *interfaceName);
}
#endif
