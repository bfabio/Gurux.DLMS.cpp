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

#include "../include/GXDLMSVariant.h"
#include "../include/GXDLMSClient.h"
#include "../include/GXDLMSSpecialDaysTable.h"
#include <sstream>

//Constructor.
CGXDLMSSpecialDaysTable::CGXDLMSSpecialDaysTable() : CGXDLMSObject(DLMS_OBJECT_TYPE_SPECIAL_DAYS_TABLE)
{
}

//SN Constructor.
CGXDLMSSpecialDaysTable::CGXDLMSSpecialDaysTable(unsigned short sn) : CGXDLMSObject(DLMS_OBJECT_TYPE_SPECIAL_DAYS_TABLE, sn)
{

}

//LN Constructor.
CGXDLMSSpecialDaysTable::CGXDLMSSpecialDaysTable(std::string ln) : CGXDLMSObject(DLMS_OBJECT_TYPE_SPECIAL_DAYS_TABLE, ln)
{

}

std::vector<CGXDLMSSpecialDay>& CGXDLMSSpecialDaysTable::GetEntries()
{
    return m_Entries;
}

void CGXDLMSSpecialDaysTable::SetValue(std::vector<CGXDLMSSpecialDay>& value)
{
    m_Entries = value;
}

// Returns amount of attributes.
int CGXDLMSSpecialDaysTable::GetAttributeCount()
{
    return 2;
}

// Returns amount of methods.
int CGXDLMSSpecialDaysTable::GetMethodCount()
{
    return 2;
}

void CGXDLMSSpecialDaysTable::GetValues(std::vector<std::string>& values)
{
    values.clear();
    std::string ln, str;
    GetLogicalName(ln);
    values.push_back(ln);
    std::stringstream sb;
    sb << '[';
    bool empty = true;
    for(std::vector<CGXDLMSSpecialDay>::iterator it = m_Entries.begin(); it != m_Entries.end(); ++it)
    {
        if (!empty)
        {
            sb << ", ";
        }
        empty = false;
        std::string str = it->ToString();
        sb.write(str.c_str(), str.size());
    }
    sb << ']';
    values.push_back(sb.str());
}

void CGXDLMSSpecialDaysTable::GetAttributeIndexToRead(std::vector<int>& attributes)
{
    //LN is static and read only once.
    if (CGXDLMSObject::IsLogicalNameEmpty(m_LN))
    {
        attributes.push_back(1);
    }
    //Entries
    if (CanRead(2))
    {
        attributes.push_back(2);
    }
}

int CGXDLMSSpecialDaysTable::GetDataType(int index, DLMS_DATA_TYPE& type)
{
    if (index == 1)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
        return DLMS_ERROR_CODE_OK;
    }
    //Entries
    if (index == 2)
    {
        type = DLMS_DATA_TYPE_ARRAY;
        return DLMS_ERROR_CODE_OK;
    }
    return DLMS_ERROR_CODE_INVALID_PARAMETER;
}

// Returns value of given attribute.
int CGXDLMSSpecialDaysTable::GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    if (e.GetIndex() == 1)
    {
        int ret;
        CGXDLMSVariant tmp;
        if ((ret = GetLogicalName(this, tmp)) != 0)
        {
            return ret;
        }
        e.SetValue(tmp);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 2)
    {
        CGXByteBuffer data;
        data.SetUInt8(DLMS_DATA_TYPE_ARRAY);
        //Add count
        GXHelpers::SetObjectCount(m_Entries.size(), data);
        int ret;
        CGXDLMSVariant index, date, id;
        for (std::vector<CGXDLMSSpecialDay>::iterator it = m_Entries.begin(); it != m_Entries.end(); ++it)
        {
            data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
            data.SetUInt8(3); //Count
            index = it->GetIndex();
            date = it->GetDate();
            id = it->GetDayId();
            if ((ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT16, index)) != DLMS_ERROR_CODE_OK ||
                    (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_DATETIME, date)) != DLMS_ERROR_CODE_OK ||
                    (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, id)) != DLMS_ERROR_CODE_OK)
            {
                return ret;
            }
        }
        return DLMS_ERROR_CODE_OK;
    }
    return DLMS_ERROR_CODE_INVALID_PARAMETER;
}

// Set value of given attribute.
int CGXDLMSSpecialDaysTable::SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    if (e.GetIndex() == 1)
    {
        int ret;
        CGXDLMSVariant tmp;
        if ((ret = GetLogicalName(this, tmp)) != 0)
        {
            return ret;
        }
        e.SetValue(tmp);
        return DLMS_ERROR_CODE_OK;
    }
    else if (e.GetIndex() == 2)
    {
        m_Entries.clear();
        if (e.GetValue().vt == DLMS_DATA_TYPE_ARRAY)
        {
            CGXDLMSVariant tmp;
            for (std::vector<CGXDLMSVariant>::iterator item = e.GetValue().Arr.begin(); item != e.GetValue().Arr.end(); ++item)
            {
                CGXDLMSSpecialDay it;
                it.SetIndex((*item).Arr[0].ToInteger());
                CGXDLMSClient::ChangeType((*item).Arr[1], DLMS_DATA_TYPE_DATE, tmp);
                it.SetDate(tmp.dateTime);
                it.SetDayId((*item).Arr[2].ToInteger());
                m_Entries.push_back(it);
            }
        }
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}
