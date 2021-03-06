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

#include "../include/GXDLMSNotify.h"

void CGXDLMSNotify::SetCipher(CGXCipher* value)
{
    m_Settings.SetCipher(value);
}

CGXDLMSSettings& CGXDLMSNotify::GetSettings()
{
    return m_Settings;
}

CGXDLMSNotify::CGXDLMSNotify(bool useLogicalNameReferencing,
                             int clientAddress,
                             int serverAddress,
                             DLMS_INTERFACE_TYPE interfaceType) : m_Settings(true)
{
    m_Settings.SetUseLogicalNameReferencing(useLogicalNameReferencing);
    m_Settings.SetClientAddress(clientAddress);
    m_Settings.SetServerAddress(serverAddress);
    m_Settings.SetInterfaceType(interfaceType);
}

bool CGXDLMSNotify::GetGeneralBlockTransfer()
{
    if (m_Settings.GetUseLogicalNameReferencing())
    {
        return m_Settings.GetLnSettings().GetGeneralBlockTransfer();
    }
    return m_Settings.GetSnSettings().GetGeneralBlockTransfer();
}

void CGXDLMSNotify::SetGeneralBlockTransfer(bool value)
{
    if (m_Settings.GetUseLogicalNameReferencing())
    {
        m_Settings.GetLnSettings().SetGeneralBlockTransfer(value);
    }
    else
    {
        m_Settings.GetSnSettings().SetGeneralBlockTransfer(value);
    }
}

CGXDLMSObjectCollection& CGXDLMSNotify::GetObjects()
{
    return m_Settings.GetObjects();
}

CGXDLMSLimits& CGXDLMSNotify::GetLimits()
{
    return m_Settings.GetLimits();
}

int CGXDLMSNotify::GetMaxReceivePDUSize()
{
    return m_Settings.GetMaxReceivePDUSize();
}

void CGXDLMSNotify::SetMaxReceivePDUSize(int value)
{
    m_Settings.SetMaxReceivePDUSize(value);
}

bool CGXDLMSNotify::GetUseLogicalNameReferencing()
{
    return m_Settings.GetUseLogicalNameReferencing();
}

void CGXDLMSNotify::SetUseLogicalNameReferencing(bool value)
{
    m_Settings.SetUseLogicalNameReferencing(value);
}

DLMS_PRIORITY CGXDLMSNotify::GetPriority()
{
    return m_Settings.GetPriority();
}

void CGXDLMSNotify::SetPriority(DLMS_PRIORITY value)
{
    m_Settings.SetPriority(value);
}

DLMS_SERVICE_CLASS CGXDLMSNotify::GetServiceClass()
{
    return m_Settings.GetServiceClass();
}

void CGXDLMSNotify::SetServiceClass(DLMS_SERVICE_CLASS value)
{
    m_Settings.SetServiceClass(value);
}

unsigned char CGXDLMSNotify::GetInvokeID()
{
    return m_Settings.GetInvokeID();
}

void CGXDLMSNotify::SetInvokeID(unsigned char value)
{
    m_Settings.SetInvokeID(value);
}

int CGXDLMSNotify::GetData(CGXByteBuffer& reply, CGXReplyData& data)
{
    return CGXDLMS::GetData(m_Settings, reply, data);
}

int CGXDLMSNotify::AddData(
    CGXDLMSObject* obj,
    unsigned char index,
    CGXByteBuffer& buff)
{
    int ret;
    DLMS_DATA_TYPE dt;
    CGXDLMSValueEventArg e(obj, index);
    if ((ret = obj->GetValue(m_Settings, e)) != 0)
    {
        return ret;
    }
    if ((ret = obj->GetDataType(index, dt)) != 0)
    {
        return ret;
    }
    if (dt == DLMS_DATA_TYPE_ARRAY)
    {
        buff.Set(e.GetValue().byteArr, e.GetValue().GetSize());
        return 0;
    }
    if (dt == DLMS_DATA_TYPE_NONE)
    {
        dt = e.GetValue().vt;
    }
    return GXHelpers::SetData(buff, e.GetValue().vt, e.GetValue());
}

int CGXDLMSNotify::GenerateDataNotificationMessages(
    struct tm* date,
    CGXByteBuffer& data,
    std::vector<CGXByteBuffer>& reply)
{
    return CGXDLMS::GetMessages(m_Settings, DLMS_COMMAND_DATA_NOTIFICATION, 0, data, date, reply);
}

int CGXDLMSNotify::GenerateDataNotificationMessages(
    struct tm* date,
    std::vector<std::pair<CGXDLMSObject*, unsigned char> >& objects,
    std::vector<CGXByteBuffer>& reply)
{
    CGXByteBuffer buff;
    buff.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
    GXHelpers::SetObjectCount(objects.size(), buff);
    for (std::vector<std::pair<CGXDLMSObject*, unsigned char> >::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        AddData(it->first, it->second, buff);
    }
    return GenerateDataNotificationMessages(date, buff, reply);
}

int CGXDLMSNotify::GeneratePushSetupMessages(
    struct tm* date,
    CGXDLMSPushSetup* push,
    std::vector<CGXByteBuffer>& reply)
{
    if (push == NULL)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    CGXByteBuffer buff;
    buff.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
    GXHelpers::SetObjectCount(push->GetPushObjectList().size(), buff);
    for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject> >::iterator it = push->GetPushObjectList().begin(); it != push->GetPushObjectList().end(); ++it)
    {
        AddData(it->first, it->second.GetAttributeIndex(), buff);
    }
    return GenerateDataNotificationMessages(date, buff, reply);
}

int CGXDLMSNotify::ParsePush(std::vector<CGXDLMSVariant>& data, std::vector<std::pair<CGXDLMSObject*, unsigned char> >& items)
{
    CGXDLMSObject *obj;
    int index, pos, ret;
    DLMS_DATA_TYPE dt;
    CGXDLMSVariant tmp, value;
    CGXDLMSVariant ln;
    for (std::vector<CGXDLMSVariant>::iterator it = data.at(0).Arr.begin(); it != data.at(0).Arr.end(); ++it)
    {
        int classID = it->Arr[0].ToInteger() & 0xFFFF;
        if (classID > 0)
        {
            ln.Clear();
            if ((ret = CGXDLMSClient::ChangeType(it->Arr[1], DLMS_DATA_TYPE_OCTET_STRING, ln)) != 0)
            {
                return ret;
            }
            obj = GetObjects().FindByLN((DLMS_OBJECT_TYPE) classID, ln.strVal);
            if (obj == NULL)
            {
                obj = CGXDLMSObjectFactory::CreateObject((DLMS_OBJECT_TYPE) classID, ln.strVal);
                GetObjects().push_back(obj);
            }
            items.push_back(std::pair<CGXDLMSObject*, unsigned char>(obj, it->Arr[2].ToInteger()));
        }
    }
    pos = 0;
    for (std::vector<std::pair<CGXDLMSObject*, unsigned char> >::iterator it = items.begin(); it != items.end(); ++it)
    {
        obj = it->first;
        value = data[pos];
        ++pos;
        index = it->second;
        if (value.vt == DLMS_DATA_TYPE_OCTET_STRING)
        {
            ret = obj->GetUIDataType(index, dt);
            if (dt != DLMS_DATA_TYPE_NONE)
            {
                tmp = value;
                if ((ret = CGXDLMSClient::ChangeType(tmp, dt, value)) != 0)
                {
                    return ret;
                }
            }
        }
        CGXDLMSValueEventArg e(obj, index);
        e.SetValue(value);
        obj->SetValue(m_Settings, e);
    }
    return 0;
}