using namespace std;
#include "INTERPK.hxx"
#include <string>
#include <unistd.h>

#include <Calcium.hxx>
#include <CalciumInterface.hxx>
#include <calcium.h>

//! Constructor for component "INTERPK" instance
/*!
 *  
 */
INTERPK_i::INTERPK_i(CORBA::ORB_ptr orb, 
                     PortableServer::POA_ptr poa, 
                     PortableServer::ObjectId * contId, 
                     const char *instanceName, 
                     const char *interfaceName) 
          : Superv_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  cerr << "create component" << endl;
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
}

//! Destructor for component "INTERPK" instance
INTERPK_i::~INTERPK_i()
{
}

//! Register datastream ports for a component service given its name
/*!
 *  \param service_name : service name
 *  \return true if port registering succeeded, false if not
 */
CORBA::Boolean
INTERPK_i::init_service(const char * service_name) {
  CORBA::Boolean rtn = false;
  string s_name(service_name);
  if (s_name == "prun") 
    {
      try
        {
          std::cerr << "INTERPK: prun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","tparoi");
          p1->setDependencyType(CalciumTypes::ITERATION_DEPENDENCY);
          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tpar");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "INTERPK: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "INTERPK: unknown exception" << std::endl;
        }

      rtn = true;
    }
  if (s_name == "trun") 
    {
      try
        {
          std::cerr << "INTERPK: trun: "  << std::endl;
          //initialization CALCIUM ports IN
          calcium_real_port_provides * p1;
          p1 = add_port<calcium_real_port_provides>("CALCIUM_real","provides","tparoit");
          p1->setDependencyType(CalciumTypes::TIME_DEPENDENCY);
          calcium_integer_port_provides * p2;
          p2 = add_port<calcium_integer_port_provides>("CALCIUM_integer","provides","topo");
          p2->setDependencyType(CalciumTypes::TIME_DEPENDENCY);

          //initialization CALCIUM ports OUT
          add_port<calcium_real_port_uses>("CALCIUM_real","uses","tpart");
          add_port<calcium_integer_port_uses>("CALCIUM_integer","uses","stopo");
        }
      catch(const PortAlreadyDefined& ex)
        {
          std::cerr << "INTERPK: " << ex.what() << std::endl;
          //Ports already created : we use them
        }
      catch ( ... ) 
        {
          std::cerr << "INTERPK: unknown exception" << std::endl;
        }
      rtn = true;
    }
  return rtn;
}

void INTERPK_i::perma(Superv_Component_i * component)
{
  float t=0.,tparoi[100],tp[100];
  int ii,nval,info;
  while(1)
    {
      info=cp_lre(component,CP_SEQUENTIEL,&t,&t,&ii,(char*)"tparoi",100,&nval,tparoi);
      if(info!=CPOK)break;
      for(int i=0;i<nval;i++)
        tp[i]=tparoi[i];
      printf("INTERPK:iteration= %d\n",ii);
      info=cp_ere(component,CP_TEMPS, t, ii, (char*)"tpar", nval, tp);
      if(info!=CPOK)break;
    }
  cp_fin(component,CP_ARRET);
}

void INTERPK_i::prun()
{
  std::cerr << "INTERPK_i::prun" << std::endl;
  beginService("INTERPK_i::prun");
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);
  try
    {
      perma(component);
    }
  catch ( const CalciumException & ex)
    {
      std::cerr << ex.what() << std::endl;
      cp_fin(component,CP_ARRET);
    }
  catch (...)
    {
      std::cerr << "unexpected exception" << std::endl;
      cp_fin(component,CP_ARRET);
    }
  endService("INTERPK_i::prun");
  std::cerr << "end of INTERPK_i::prun" << std::endl;
}

void INTERPK_i::transit(Superv_Component_i * component)
{
  float t=0.,tparoi[100],tp[100];
  int topo[20],stopo[20];
  int ii,nval,info,mval;
  for(int i=0;i<20;i++)
    stopo[i]=0;

  while(1)
    {
      info=cp_lre(component,CP_SEQUENTIEL,&t,&t,&ii,(char*)"tparoit",100,&nval,tparoi);
      if(info!=CPOK)break;
      for(int i=0;i<nval;i++)
        tp[i]=tparoi[i];
      info=cp_len(component,CP_SEQUENTIEL,&t,&t,&ii,(char*)"topo",20,&mval,topo);
      if(info!=CPOK)break;

      printf("INTERPK:temps= %f\n",t);
      for(int i=0;i<mval;i++)
        {
          stopo[i]=stopo[i]+topo[i];
          printf("stopo[%d]=%d;",i,stopo[i]);
        }
      printf("\n");

      info=cp_ere(component,CP_TEMPS, t, ii, (char*)"tpart", nval, tp);
      if(info!=CPOK)break;
      info=cp_een(component,CP_TEMPS, t, ii, (char*)"stopo", mval, stopo);
      if(info!=CPOK)break;
    }
  cp_fin(component,CP_ARRET);
}

void INTERPK_i::trun()
{
  std::cerr << "INTERPK_i::trun" << std::endl;
  beginService("INTERPK_i::trun");
  Superv_Component_i * component = dynamic_cast<Superv_Component_i*>(this);
  char       nom_instance[INSTANCE_LEN];
  int info = cp_cd(component,nom_instance);
  try
    {
      transit(component);
    }
  catch ( const CalciumException & ex)
    {
      std::cerr << ex.what() << std::endl;
      cp_fin(component,CP_ARRET);
    }
  catch (...)
    {
      std::cerr << "unexpected exception" << std::endl;
      cp_fin(component,CP_ARRET);
    }
  endService("INTERPK_i::trun");
  std::cerr << "end of INTERPK_i::trun" << std::endl;
}

extern "C"
{
  PortableServer::ObjectId * INTERPKEngine_factory( CORBA::ORB_ptr orb, 
                                                    PortableServer::POA_ptr poa, 
                                                    PortableServer::ObjectId * contId,
                                                    const char *instanceName, 
                                                    const char *interfaceName)
  {
    std::cerr << "INTERPKEngine_factory" << std::endl;
    INTERPK_i * myEngine = new INTERPK_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
