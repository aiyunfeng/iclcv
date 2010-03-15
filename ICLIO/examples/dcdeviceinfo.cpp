/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/examples/dcdeviceinfo.cpp                        **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLIO/DCGrabber.h>

using namespace std;
using namespace icl;


int main(int n, char **ppc){
  std::vector<DCDevice> devs = DCGrabber::getDeviceList();
  printf("found %d cameras \n",(unsigned int)devs.size());

  
  for(unsigned int i=0;i<devs.size();i++){
    DCDevice &d = devs[i];
    char acBuf[100];
    sprintf(acBuf,"%2d",i);
    d.show(acBuf);
  }
  
  return 0;
}

