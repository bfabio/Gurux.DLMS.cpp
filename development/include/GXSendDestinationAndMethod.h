//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL$
//
// Version:         $Revision$,
//                  $Date$
//                  $Author$
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------
//
//  DESCRIPTION
//
// This file is a part of Gurux Device Framework.
//
// Gurux Device Framework is Open Source software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version 2 of the License.
// Gurux Device Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// More information of Gurux products: http://www.gurux.org
//
// This code is licensed under the GNU General Public License v2.
// Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
//---------------------------------------------------------------------------

#ifndef GXSENDDESTINATIONANDMETHOD_H
#define GXSENDDESTINATIONANDMETHOD_H

#include "enums.h"
#include <string>

class CGXSendDestinationAndMethod
{
private:
    DLMS_SERVICE_TYPE m_Service;
    std::string m_Destination;
    DLMS_MESSAGE_TYPE m_Message;

public:
    DLMS_SERVICE_TYPE GetService();
    void SetService(DLMS_SERVICE_TYPE value);

    std::string GetDestination();
    void SetDestination(std::string value);

    DLMS_MESSAGE_TYPE GetMessage();
    void SetMessage(DLMS_MESSAGE_TYPE value);
};
#endif //GXSENDDESTINATIONANDMETHOD_H