#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
#include <functional>
#include <boost/shared_ptr.hpp>

#include "Logger.h"
#include "AuxException.h"
#include "XMLattribute.h"
#include "XMLobject.h"
#include "XMLgroup.h"
#include "XMLgrid.h"
#include "XMLread.h"
#include "XMLcollect.h"
#include "IndexIterator.h"

namespace Mantid
{

namespace XML
{

void
combineGrid(XML::XMLcollect& XOut,
	    const std::string& BName,const int blockCnt)
  /*!
    Combined Grid components together (if possible)
    \param XOut :: XMLcollect to obtain merged results from
    \param BName :: Regex naem to find
    \param blockCount :: to write the time separately
  */
{
  typedef XML::XMLgrid<std::vector,double,std::allocator<double> > xGridType;

  int notValidCnt(0);     // index count of items
  int gcnt(0);            // grid object cnt

  XML::XMLobject* oPtr=XOut.findObj(BName);     // Object with grid    

  // Place to put result
  XML::XMLgroup* groupPtr=
    (oPtr) ? dynamic_cast<XML::XMLgroup*>(oPtr->getParent()) : 0; 

  // Get XMLgrid object
  xGridType* gPtr=dynamic_cast<xGridType*>(oPtr);
  xGridType* gStore(0);   // Item before adding
  std::vector<xGridType*> gArray;

  int sNum(0);        
  while(oPtr)     // While findind an object
    {
      if (gPtr)
        {
	  if (!(gcnt % blockCnt))
	    {
	      if (gStore)
	        {
		  gStore->addAttribute("iStart",sNum);
		  gStore->addAttribute("iEnd",gcnt-1);
		  sNum=gcnt;
		  gArray.push_back(gStore);
		}
	      gStore=new xGridType(XOut.getCurrent(),"GridCluster");
	      for(int i=0;i<gPtr->getSize();i++)
		gStore->setComp(i,gPtr->getGVec(i));
	    }
	  else
	    {
	      // Add just the non-time points
	      for(int i=1;i<gPtr->getSize();i++)
		gStore->setComp(gStore->getSize(),gPtr->getGVec(i));
	    }
	  XOut.deleteObj(oPtr);
	  gcnt++;
	}
      else  // found object but not a grid 
        {
	  notValidCnt++;
	}
      oPtr=XOut.findObj(BName,notValidCnt);
      gPtr=dynamic_cast<xGridType*>(oPtr);
    }


  if (gStore)
    {
      gStore->addAttribute("iStart",sNum);
      gStore->addAttribute("iEnd",gcnt-1);
      gArray.push_back(gStore);
    }

  std::vector<xGridType*>::iterator vc;
  if (groupPtr)
    {
      for(vc=gArray.begin();vc!=gArray.end();vc++)
	groupPtr->addManagedObj(*vc);
    }
  else
    {
      for(vc=gArray.begin();vc!=gArray.end();vc++)
	XOut.getCurrent()->addManagedObj(*vc);
    }

  return;
}  

void
combineDeepGrid(XML::XMLcollect& XOut,
		const std::string& BName,const int blockCnt)
  /*!
    Combined Grid components together (if possible)
    assuming that the grid is a deep grid. e.g.
    - <Obj> <stuff> </stuff> <grid> </grid> </Obj>
    the requirement is that stuff is keep and that 
    
    \param XOut :: XMLcollect to obtain merged results from
    \param BName :: Regex name to find [depth]
    \param blockCount :: to write the time separately
  */
{
  typedef XML::XMLgrid<std::vector,double,std::allocator<double> > xGridType;
  int objectCnt(0);          // Object count
  int gcnt(0);            // grid object cnt

  XML::XMLgroup* oPtr=
    dynamic_cast<XML::XMLgroup*>(XOut.findObj(BName,objectCnt));
  // Place to put result
  XML::XMLgroup* groupPtr=(oPtr) ? 
    dynamic_cast<XML::XMLgroup*>(oPtr->getParent()) : 0;
  if (!groupPtr)  // Failed
    return;
  
  // Now search the group for specifics of 
  int individualGrpCnt(0);

  // Get XMLgrid object
  xGridType* gPtr=oPtr->getType<xGridType>(individualGrpCnt);

  xGridType* gStore(0);   // Item before adding
  std::vector<xGridType*> gArray;

  int sNum(0);        
  while(gPtr)     // While finding an object
    {
      if (!(gcnt % blockCnt))       // New grid object required
        {
	  if (gStore)  // push back old object
	    {
	      gStore->addAttribute("iStart",sNum);
	      gStore->addAttribute("iEnd",gcnt-1);
	      sNum=gcnt;
	      gArray.push_back(gStore);
	    }
	  gStore=new xGridType(XOut.getCurrent(),"GridCluster");
	  for(int i=0;i<gPtr->getSize();i++)
	    gStore->setComp(i,gPtr->getGVec(i));
	}
      else
        {
	  // Add just the non-time points
	  for(int i=1;i<gPtr->getSize();i++)
	    gStore->setComp(gStore->getSize(),gPtr->getGVec(i));
	}
      XOut.deleteObj(gPtr);
      gcnt++;
      gPtr=oPtr->getType<xGridType>();
      if (!gPtr)  // New individual group needed
        {   
	  objectCnt++;
	  XML::XMLgroup* oPtr=dynamic_cast<XML::XMLgroup*>
	    (XOut.findObj(BName,objectCnt));
	  gPtr=(oPtr) ? 
	    oPtr->getType<xGridType>(individualGrpCnt) : 0;
	}
    }
  // Add last componenet if valid
  if (gStore)
    {
      gStore->addAttribute("iStart",sNum);
      gStore->addAttribute("iEnd",gcnt-1);
      gArray.push_back(gStore);
    }
  // Now add back grid objects into stream 
  std::vector<xGridType*>::iterator vc;
  for(vc=gArray.begin();vc!=gArray.end();vc++)
    groupPtr->addManagedObj(*vc);
  return;
}  


}  // NAMESPACE XML

}  // NAMESPACE Mantid
