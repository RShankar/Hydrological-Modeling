/*****************************************************************************

    Copyright 2010
    Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.


   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  sca_information_info.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: Jan 19, 2010

   SVN Version       :  $Revision: 1086 $
   SVN last checkin  :  $Date: 2010-11-21 23:12:11 +0100 (Sun, 21 Nov 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_implementation_info.cpp 1086 2010-11-21 22:12:11Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "scams/utility/reporting/sca_implementation_info.h"

#define HAVE_CONFIG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef VERSION
#define VERSION "????"
#endif



#define SYSTEMC_AMS_VERSION 20100119
#define SCA_RELEASE_STRING  "1.0"

// \TODO move to config.h
#define SCA_RELEASE_DATE    "unknown release date"

namespace sca_core
{


  static const char copyright[] =
      "        Copyright (c) 1999-2010 by all Contributors\n"
      "                    ALL RIGHTS RESERVED";

  static const char systemc_ams_version[] =
      "             SystemC AMS " SCA_RELEASE_STRING " --- " VERSION " --- " SCA_RELEASE_DATE;


  const char* sca_copyright()
  {
      return copyright;
  }


  const char* sca_release()
  {




      return SCA_RELEASE_STRING "-Fraunhofer";
  }


  const char* sca_version()
  {
      return systemc_ams_version;
  }



} // namespace sca_core

