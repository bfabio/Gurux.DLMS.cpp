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

#include "../include/GXHelpers.h"

/**
    * Get array from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * index
    *            starting index.
    * Returns  CGXDLMSVariant array.
    */
static int GetArray(CGXByteBuffer& buff, CGXDataInfo& info, int index, CGXDLMSVariant& value)
{
    int ret;
    unsigned long cnt = 0;
    CGXDataInfo info2;
    CGXDLMSVariant tmp;
    if (info.GetCount() == 0)
    {
        if ((ret = GXHelpers::GetObjectCount(buff, cnt)) != 0)
        {
            return ret;
        }
        info.SetCount(cnt);
    }
    int size = buff.GetSize() - buff.GetPosition();
    if (info.GetCount() != 0 && size < 1)
    {
        info.SetCompleate(false);
        return 0;
    }
    int startIndex = index;
    value.vt = DLMS_DATA_TYPE_ARRAY;
    // Position where last row was found. Cache uses this info.
    int pos = info.GetIndex();
    for (; pos != info.GetCount(); ++pos)
    {
        info2.Clear();
        tmp.Clear();
        if ((ret = GXHelpers::GetData(buff, info2, tmp)) != 0)
        {
            return ret;
        }
        if (!info2.IsCompleate())
        {
            buff.SetPosition(startIndex);
            info.SetCompleate(false);
            break;
        }
        else
        {
            if (info2.GetCount() == info2.GetIndex())
            {
                startIndex = buff.GetPosition();
                value.Arr.push_back(tmp);
            }
        }
    }
    info.SetIndex(pos);
    return 0;
}

/**
    * Get time from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  Parsed time.
    */
int GetTime(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned char ch, hour, minute, second, ms;
    if (buff.GetSize() - buff.GetPosition() < 4)
    {
        // If there is not enough data available.
        info.SetCompleate(false);
        return 0;
    }
    // Get time.
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    hour = ch;
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    minute = ch;
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    second = ch;
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    ms = ch;
    CGXDateTime dt(-1, -1, -1, hour, minute, second, ms);
    value = dt;
    return 0;
}

/**
    * Get date from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  Parsed date.
    */
int GetDate(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret, year, month, day;
    unsigned char ch;
    if (buff.GetSize() - buff.GetPosition() < 5)
    {
        // If there is not enough data available.
        info.SetCompleate(false);
        return 0;
    }
    // Get year.
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    year = ch;
    // Get month
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    month = ch;
    // Get day
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    day = ch;
    // Skip week day
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    CGXDateTime dt(year, month, day, -1, -1, -1, -1);
    value = dt;
    return 0;
}

//Get UTC offset in minutes.
static void GetUtcOffset(int& hours, int& minutes)
{
    time_t zero = 24*60*60L;
    struct tm tm;

    // local time for Jan 2, 1900 00:00 UTC
#if _MSC_VER > 1000
    localtime_s(&tm, &zero);
#else
    tm = *localtime(&zero);
#endif
    hours = tm.tm_hour;

    //If the local time is the "day before" the UTC, subtract 24 hours from the hours to get the UTC offset
    if(tm.tm_mday < 2 )
    {
        hours -= 24;
    }
    minutes = tm.tm_min;
}

static time_t GetUtcTime(struct tm * timeptr)
{
    /* gets the epoch time relative to the local time zone,
    and then adds the appropriate number of seconds to make it UTC */
    int hours, minutes;
    GetUtcOffset(hours, minutes);
    return mktime( timeptr ) + (hours * 3600) + (minutes * 60);
}

/**
    * Get date and time from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  Parsed date and time.
    */
int GetDateTime(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    struct tm tm = {0};
    unsigned short year;
    short deviation;
    int ret, ms, status;
    unsigned char ch;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 12)
    {
        info.SetCompleate(false);
        return 0;
    }
    // Get year.
    if ((ret = buff.GetUInt16(&year)) != 0)
    {
        return ret;
    }
    tm.tm_year = year;
    // Get month
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    tm.tm_mon = ch;
    // Get day
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    tm.tm_mday = ch;
    // Skip week day
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    tm.tm_wday = ch;
    // Get time.
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    tm.tm_hour = ch;
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    tm.tm_min = ch;
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    tm.tm_sec = ch;
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    ms = ch;
    if (ms != 0xFF)
    {
        ms *= 10;
    }
    else
    {
        ms = 0;
    }
    if ((ret = buff.GetInt16(&deviation)) != 0)
    {
        return ret;
    }
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    status = ch;
    CGXDateTime dt;
    dt.SetStatus((DLMS_CLOCK_STATUS) status);
    DATETIME_SKIPS skip = DATETIME_SKIPS_NONE;
    if (year < 1 || year == 0xFFFF)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_YEAR);
        tm.tm_year = 0;
    }
    else
    {
        tm.tm_year -= 1900;
    }
    if (tm.tm_wday < 0 || tm.tm_wday > 7)
    {
        tm.tm_wday = 0;
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_DAYOFWEEK);
    }
    dt.SetDaylightSavingsBegin(tm.tm_mon == 0xFE);
    dt.SetDaylightSavingsEnd(tm.tm_mon == 0xFD);
    if (tm.tm_mon < 1 || tm.tm_mon > 12)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_MONTH);
        tm.tm_mon = 0;
    }
    else
    {
        tm.tm_mon -= 1;
    }
    if (tm.tm_mday == -1 || tm.tm_mday == 0 || tm.tm_mday > 31)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_DAY);
        tm.tm_mday = 1;
    }
    else if (tm.tm_mday < 0)
    {
        //TODO: day = cal.GetActualMaximum(Calendar.DATE) + day + 3;
    }
    if (tm.tm_hour < 0 || tm.tm_hour > 24)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_HOUR);
        tm.tm_hour = 0;
    }
    if (tm.tm_min < 0 || tm.tm_min > 60)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_MINUTE);
        tm.tm_min = 0;
    }
    if (tm.tm_sec < 0 || tm.tm_sec > 60)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_SECOND);
        tm.tm_sec = 0;
    }
    // If ms is Zero it's skipped.
    if (ms < 1 || ms > 1000)
    {
        skip = (DATETIME_SKIPS) (skip | DATETIME_SKIPS_MS);
        ms = 0;
    }
    if (deviation != -32768)//0x8000
    {
        tm.tm_min -= deviation;
        time_t t = GetUtcTime(&tm);
        if (t == -1)
        {
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
#if _MSC_VER > 1000
        localtime_s(&tm, &t);
#else
        tm = *localtime(&t);
#endif
    }
    // If summer time and it is not set on our environment.
    if (tm.tm_isdst == 0 && (status & DLMS_CLOCK_STATUS_DAYLIGHT_SAVE_ACTIVE) != 0)
    {
        tm.tm_hour += 1;
        if (mktime(&tm) == -1)
        {
            assert(0);
        }
    }
    dt.SetValue(tm);
    dt.SetDeviation(deviation);
    dt.SetSkip(skip);
    value = dt;
    return 0;
}

/**
    * Get double value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  Parsed double value.
    */
int GetDouble(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    double val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 8)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetDouble(&val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
    * Get float value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  Parsed float value.
    */
int GetFloat(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 4)
    {
        info.SetCompleate(false);
        return 0;
    }
    return buff.GetFloat(&value.fltVal);
}

/**
    * Get enumeration value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed enumeration value.
    */
int GetEnum(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned char ch;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 1)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    value = ch;
    value.vt = DLMS_DATA_TYPE_ENUM;
    return 0;
}

/**
    * Get UInt64 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed UInt64 value.
    */
int GetUInt64(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned long long val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 8)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetUInt64(&val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
    * Get Int64 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed Int64 value.
    */
int GetInt64(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    long long val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 8)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetInt64(&val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
    * Get UInt16 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed UInt16 value.
    */
int GetUInt16(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned short val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 2)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetUInt16(&val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
    * Get UInt8 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed UInt8 value.
    */
int GetUInt8(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned char val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 1)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetUInt8(&val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
    * Get Int16 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed Int16 value.
    */
int GetInt16(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    short val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 2)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetInt16(&val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
    * Get Int8 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed Int8 value.
    */
int GetInt8(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    char val;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 1)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetUInt8((unsigned char*) &val)) != 0)
    {
        return ret;
    }
    value = val;
    return 0;
}

/**
* Get BCD value from DLMS data.
*
* buff
*            Received DLMS data.
* info
*            Data info.
* Returns  parsed BCD value.
*/
int GetBcd(CGXByteBuffer& buff, CGXDataInfo& info, bool knownType, CGXDLMSVariant& value)
{
    const char hexArray[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    int ret;
    unsigned long len = 0;
    unsigned char ch;
    if (knownType)
    {
        len = buff.GetSize();
    }
    else
    {
        if ((ret = GXHelpers::GetObjectCount(buff, len)) != 0)
        {
            return ret;
        }
        // If there is not enough data available.
        if ((buff.GetSize() - buff.GetPosition()) < (int) len)
        {
            info.SetCompleate(false);
            return 0;
        }
    }
    CGXByteBuffer bcd(len * 2);
    for (unsigned long a = 0; a != len; ++a)
    {
        if ((ret = buff.GetUInt8(&ch)) != 0)
        {
            return ret;
        }
        int idHigh = ch >> 4;
        int idLow = ch & 0x0F;
        bcd.SetUInt8(hexArray[idHigh]);
        bcd.SetUInt8(hexArray[idLow]);
    }
    value = bcd.ToString();
    return 0;
}

int GXHelpers::GetObjectCount(CGXByteBuffer& data, unsigned long& count)
{
    int ret;
    unsigned char cnt;
    if ((ret = data.GetUInt8(&cnt)) != 0)
    {
        return ret;
    }
    if (cnt > 0x80)
    {
        if (cnt == 0x81)
        {
            if ((ret = data.GetUInt8(&cnt)) != 0)
            {
                return ret;
            }
            count = cnt;
            return DLMS_ERROR_CODE_OK;
        }
        else if (cnt == 0x82)
        {
            unsigned short tmp;
            if ((ret = data.GetUInt16(&tmp)) != 0)
            {
                return ret;
            }
            count = tmp;
            return DLMS_ERROR_CODE_OK;
        }
        else if (cnt == 0x84)
        {
            unsigned long tmp;
            if ((ret = data.GetUInt32(&tmp)) != 0)
            {
                return ret;
            }
            count = tmp;
            return DLMS_ERROR_CODE_OK;
        }
        else
        {
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    count = cnt;
    return DLMS_ERROR_CODE_OK;
}

unsigned char GXHelpers::GetObjectCountSizeInBytes(unsigned long count)
{
    if (count < 0x80)
    {
        return 1;
    }
    else if (count < 0x100)
    {
        return 2;
    }
    else if (count < 0x10000)
    {
        return 3;
    }
    else
    {
        return 5;
    }
}

void GXHelpers::SetObjectCount(unsigned long count, CGXByteBuffer& buff)
{
    if (count < 0x80)
    {
        buff.SetUInt8((unsigned char) count);
    }
    else if (count < 0x100)
    {
        buff.SetUInt8(0x81);
        buff.SetUInt8((unsigned char) count);
    }
    else if (count < 0x10000)
    {
        buff.SetUInt8(0x82);
        buff.SetUInt16((unsigned short) count);
    }
    else
    {
        buff.SetUInt8(0x84);
        buff.SetUInt32(count);
    }
}

std::vector< std::string > GXHelpers::Split(std::string& s, char separator)
{
    std::vector< std::string > items;
    int last = 0;
    int pos = -1;
    while((pos = s.find(separator, pos + 1)) != -1)
    {
        std::string str;
        str.append(s, last, pos - last);
        items.push_back(str);
        last = pos + 1;
    }
    int len = s.length();
    if (last == 0 || last != len)
    {
        std::string str;
        str.append(s, last, pos);
        items.push_back(str);
    }
    return items;
}

std::vector< std::string > GXHelpers::Split(std::string& s, std::string separators, bool ignoreEmpty)
{
    std::vector< std::string > items;
    int last = 0;
    int pos = -1;
    while((pos = s.find_first_of(separators, pos + 1)) != -1)
    {
        if (!ignoreEmpty || pos - last != 0)
        {
            std::string str;
            str.append(s, last, pos - last);
            items.push_back(str);
        }
        last = pos + 1;
    }
    int len = s.length();
    if (!ignoreEmpty || len - last != 0)
    {
        std::string str;
        str.append(s, last, len - last);
        items.push_back(str);
    }
    return items;
}

void GXHelpers::Replace(std::string& str, std::string oldString, std::string newString)
{
    int index;
    int len = oldString.length();
    while ((index = str.rfind(oldString)) != -1)
    {
        str.replace(index, len, newString);
    }
}

std::string& GXHelpers::ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string GXHelpers::BytesToHex(unsigned char* pBytes, int count)
{
    const char hexArray[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    std::string hexChars(3 * count, 0);
    int tmp;
    for (int pos = 0; pos != count; ++pos)
    {
        tmp = pBytes[pos] & 0xFF;
        hexChars[pos * 3] = hexArray[tmp >> 4];
        hexChars[pos * 3 + 1] = hexArray[tmp & 0x0F];
        hexChars[pos * 3 + 2] = ' ';
    }
    //Remove last white space.
    if (count != 0)
    {
        hexChars.resize(hexChars.size() - 1);
    }
    return hexChars;
}
/**
* Get UTF std::string value from DLMS data.
*
* buff
*            Received DLMS data.
* info
*            Data info.
* Returns  parsed UTF std::string value.
*/
int GetUtfString(CGXByteBuffer& buff, CGXDataInfo& info, bool knownType, CGXDLMSVariant& value)
{
    int ret;
    unsigned long len = 0;
    wchar_t *tmp;
    if (knownType)
    {
        len = buff.GetSize();
    }
    else
    {
        if ((ret = GXHelpers::GetObjectCount(buff, len)) != 0)
        {
            return ret;
        }
        // If there is not enough data available.
        if (buff.GetSize() - buff.GetPosition() < (int) len)
        {
            info.SetCompleate(false);
            return 0;
        }
    }
    if (len > 0)
    {

        tmp = new wchar_t[len];
        buff.Get((unsigned char*) tmp, 2 * len);
        value.strUtfVal.append(tmp, len);
    }
    else
    {
        value = "";
    }
    return 0;
}

/**
    * Get octect std::string value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed octet std::string value.
    */
int GetOctetString(CGXByteBuffer& buff, CGXDataInfo& info, bool knownType, CGXDLMSVariant& value)
{
    int ret;
    unsigned long len = 0;
    if (knownType)
    {
        len = buff.GetSize();
    }
    else
    {
        if ((ret = GXHelpers::GetObjectCount(buff, len)) != 0)
        {
            return ret;
        }
        // If there is not enough data available.
        if (buff.GetSize() - buff.GetPosition() < (int) len)
        {
            info.SetCompleate(false);
            return 0;
        }
    }
    value.size = (unsigned short) len;
    if (len == 0)
    {
        value.byteArr = NULL;
    }
    else
    {
        value.byteArr = new unsigned char[len];
        if ((ret = buff.Get(value.byteArr, len)) != 0)
        {
            return ret;
        }
    }
    value.vt = DLMS_DATA_TYPE_OCTET_STRING;
    return 0;
}

/**
    * Get std::string value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed std::string value.
    */
int GetString(CGXByteBuffer& buff, CGXDataInfo& info, bool knownType, CGXDLMSVariant& value)
{
    int ret;
    unsigned long len = 0;
    char *tmp;
    if (knownType)
    {
        len = buff.GetSize();
    }
    else
    {
        if ((ret = GXHelpers::GetObjectCount(buff, len)) != 0)
        {
            return ret;
        }
        // If there is not enough data available.
        if (buff.GetSize() - buff.GetPosition() < (int) len)
        {
            info.SetCompleate(false);
            return 0;
        }
    }
    if (len > 0)
    {
        tmp = new char[len + 1];
        tmp[len] = '\0';
        if ((ret = buff.Get((unsigned char*) tmp, len)) != 0)
        {
            delete tmp;
            return ret;
        }
        value = tmp;
        delete tmp;
    }
    else
    {
        value = "";
    }
    return 0;
}

/**
    * Get UInt32 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed UInt32 value.
    */
int GetUInt32(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 4)
    {
        info.SetCompleate(false);
        return 0;
    }
    value.vt = DLMS_DATA_TYPE_UINT32;
    return buff.GetUInt32(&value.ulVal);
}

/**
    * Get Int32 value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed Int32 value.
    */
int GetInt32(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 4)
    {
        info.SetCompleate(false);
        return 0;
    }
    value.vt = DLMS_DATA_TYPE_INT32;
    return buff.GetInt32(&value.lVal);
}


//Reserved for internal use.
static void ToBitString(CGXByteBuffer& sb, unsigned char value, int count)
{
    if (count > 8)
    {
        count = 8;
    }
    char* data = new char[count];
    for (int pos = 0; pos != count; ++pos)
    {
        if ((value & (1 << pos)) != 0)
        {
            data[count - pos - 1] = '1';
        }
        else
        {
            data[count - pos - 1] = '0';
        }
    }
    sb.Set(data, count);
}

/**
* Get bit std::string value from DLMS data.
*
* buff : Received DLMS data.
* info : Data info.
* Returns parsed bit std::string value.
*/
static int GetBitString(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    unsigned long cnt = 0;
    int ret;
    unsigned char ch;
    if ((ret = GXHelpers::GetObjectCount(buff, cnt)) != 0)
    {
        return ret;
    }
    double t = cnt;
    t /= 8;
    if (cnt % 8 != 0)
    {
        ++t;
    }
    int byteCnt = (int) t;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < byteCnt)
    {
        info.SetCompleate(false);
        return 0;
    }

    CGXByteBuffer bb;
    while (cnt > 0)
    {
        if ((ret = buff.GetUInt8(&ch)) != 0)
        {
            return ret;
        }
        ToBitString(bb, ch, cnt);
        cnt -= 8;
    }
    value = bb.ToString();
    return 0;
}

/**
    * Get bool value from DLMS data.
    *
    * buff
    *            Received DLMS data.
    * info
    *            Data info.
    * Returns  parsed bool value.
    */
static int GetBool(CGXByteBuffer& buff, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned char ch;
    // If there is not enough data available.
    if (buff.GetSize() - buff.GetPosition() < 1)
    {
        info.SetCompleate(false);
        return 0;
    }
    if ((ret = buff.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    value.vt = DLMS_DATA_TYPE_BOOLEAN;
    value.boolVal = ch != 0;
    return 0;
}

int GXHelpers::GetData(CGXByteBuffer& data, CGXDataInfo& info, CGXDLMSVariant& value)
{
    int ret;
    unsigned char ch;
    int startIndex = data.GetPosition();
    value.Clear();
    if (data.GetPosition() == data.GetSize())
    {
        info.SetCompleate(false);
        return 0;
    }
    info.SetCompleate(true);
    bool knownType = info.GetType() != DLMS_DATA_TYPE_NONE;
    // Get data type if it is unknown.
    if (!knownType)
    {
        if ((ret = data.GetUInt8(&ch)) != 0)
        {
            return ret;
        }
        info.SetType((DLMS_DATA_TYPE) ch);
    }
    if (info.GetType() == DLMS_DATA_TYPE_NONE)
    {
        return 0;
    }
    if (data.GetPosition() == data.GetSize())
    {
        info.SetCompleate(false);
        return 0;
    }
    switch (info.GetType())
    {
    case DLMS_DATA_TYPE_ARRAY:
    case DLMS_DATA_TYPE_STRUCTURE:
        ret = GetArray(data, info, startIndex, value);
        value.vt = info.GetType();
        break;
    case DLMS_DATA_TYPE_BOOLEAN:
        ret = GetBool(data, info, value);
        break;
    case DLMS_DATA_TYPE_BIT_STRING:
        ret = GetBitString(data, info, value);
        break;
    case DLMS_DATA_TYPE_INT32:
        ret = GetInt32(data, info, value);
        break;
    case DLMS_DATA_TYPE_UINT32:
        ret = GetUInt32(data, info, value);
        break;
    case DLMS_DATA_TYPE_STRING:
        ret = GetString(data, info, knownType, value);
        break;
    case DLMS_DATA_TYPE_STRING_UTF8:
        ret = GetUtfString(data, info, knownType, value);
        break;
    case DLMS_DATA_TYPE_OCTET_STRING:
        ret = GetOctetString(data, info, knownType, value);
        break;
    case DLMS_DATA_TYPE_BINARY_CODED_DESIMAL:
        ret = GetBcd(data, info, knownType, value);
        break;
    case DLMS_DATA_TYPE_INT8:
        ret = GetInt8(data, info, value);
        break;
    case DLMS_DATA_TYPE_INT16:
        ret = GetInt16(data, info, value);
        break;
    case DLMS_DATA_TYPE_UINT8:
        ret = GetUInt8(data, info, value);
        break;
    case DLMS_DATA_TYPE_UINT16:
        ret = GetUInt16(data, info, value);
        break;
    case DLMS_DATA_TYPE_COMPACT_ARRAY:
        assert(0);
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        break;
    case DLMS_DATA_TYPE_INT64:
        ret = GetInt64(data, info, value);
        break;
    case DLMS_DATA_TYPE_UINT64:
        ret = GetUInt64(data, info, value);
        break;
    case DLMS_DATA_TYPE_ENUM:
        ret = GetEnum(data, info, value);
        break;
    case DLMS_DATA_TYPE_FLOAT32:
        ret = GetFloat(data, info, value);
        break;
    case DLMS_DATA_TYPE_FLOAT64:
        ret = GetDouble(data, info, value);
        break;
    case DLMS_DATA_TYPE_DATETIME:
        ret = GetDateTime(data, info, value);
        break;
    case DLMS_DATA_TYPE_DATE:
        ret = GetDate(data, info, value);
        break;
    case DLMS_DATA_TYPE_TIME:
        ret = GetTime(data, info, value);
        break;
    default:
        assert(0);
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

/**
    * Convert time to DLMS bytes.
    *
    * buff
    *            Byte buffer where data is write.
    * value
    *            Added value.
    */
static int SetTime(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    DATETIME_SKIPS skip = value.dateTime.GetSkip();
    struct tm dt = value.dateTime.GetValue();
    // Add size
    buff.SetUInt8(4);
    // Add time.
    if ((skip & DATETIME_SKIPS_HOUR) != 0)
    {
        buff.SetUInt8(0xFF);
    }
    else
    {
        buff.SetUInt8(dt.tm_hour);
    }
    if ((skip & DATETIME_SKIPS_MINUTE) != 0)
    {
        buff.SetUInt8(0xFF);
    }
    else
    {
        buff.SetUInt8(dt.tm_min);
    }
    if ((skip & DATETIME_SKIPS_SECOND) != 0)
    {
        buff.SetUInt8(0xFF);
    }
    else
    {
        buff.SetUInt8(dt.tm_sec);
    }
    // Hundredths of second is not used.
    buff.SetUInt8(0xFF);
    return 0;
}

/**
    * Convert date to DLMS bytes.
    *
    * buff
    *            Byte buffer where data is write.
    * value
    *            Added value.
    */
static int SetDate(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    struct tm dt = value.dateTime.GetValue();
    DATETIME_SKIPS skip = value.dateTime.GetSkip();
    // Add size
    buff.SetUInt8(5);
    // Add year.
    if ((skip & DATETIME_SKIPS_YEAR) != 0)
    {
        buff.SetUInt16(0xFFFF);
    }
    else
    {
        buff.SetUInt16(dt.tm_year);
    }
    // Add month
    if (value.dateTime.GetDaylightSavingsBegin())
    {
        buff.SetUInt8(0xFE);
    }
    else if (value.dateTime.GetDaylightSavingsEnd())
    {
        buff.SetUInt8(0xFD);
    }
    else if ((skip & DATETIME_SKIPS_MONTH) != 0)
    {
        buff.SetUInt8(0xFF);
    }
    else
    {
        buff.SetUInt8(dt.tm_mon + 1);
    }
    // Add day
    if ((skip & DATETIME_SKIPS_DAY) != 0)
    {
        buff.SetUInt8(0xFF);
    }
    else
    {
        buff.SetUInt8(dt.tm_mday);
    }
    // Week day is not spesified.
    buff.SetUInt8(0xFF);
    return 0;
}

/**
    * Convert date time to DLMS bytes.
    *
    * buff
    *            Byte buffer where data is write.
    * value
    *            Added value.
    */
static int SetDateTime(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    // Add size
    buff.SetUInt8(12);
    //Add year.
    unsigned short year = 0xFFFF;
    struct tm dt = value.dateTime.GetValue();
    DATETIME_SKIPS skip = value.dateTime.GetSkip();
    if (dt.tm_year != -1 && (skip & DATETIME_SKIPS_YEAR) == 0)
    {
        year = 1900 + dt.tm_year;
    }
    //If sumer time
    if (dt.tm_isdst != 0)
    {
        dt.tm_hour -= 1;
    }

    buff.SetUInt16(year);
    //Add month
    if (value.dateTime.GetDaylightSavingsBegin())
    {
        buff.SetUInt8(0xFE);
    }
    else if (value.dateTime.GetDaylightSavingsEnd())
    {
        buff.SetUInt8(0xFD);
    }
    else if (dt.tm_mon != -1 && (skip & DATETIME_SKIPS_MONTH) == 0)
    {
        buff.SetUInt8(dt.tm_mon + 1);
    }
    else
    {
        buff.SetUInt8(0xFF);
    }
    //Add day
    if (dt.tm_mday != -1 && (skip & DATETIME_SKIPS_DAY) == 0)
    {
        buff.SetUInt8(dt.tm_mday);
    }
    else
    {
        buff.SetUInt8(0xFF);
    }
    //Add week day
    buff.SetUInt8(0xFF);
    //Add Hours
    if (dt.tm_hour != -1 && (skip & DATETIME_SKIPS_HOUR) == 0)
    {
        buff.SetUInt8(dt.tm_hour);
    }
    else
    {
        buff.SetUInt8(0xFF);
    }
    //Add Minutes
    if (dt.tm_min != -1 && (skip & DATETIME_SKIPS_MINUTE) == 0)
    {
        buff.SetUInt8(dt.tm_min);
    }
    else
    {
        buff.SetUInt8(0xFF);
    }
    //Add seconds.
    if (dt.tm_sec != -1 && (skip & DATETIME_SKIPS_SECOND) == 0)
    {
        buff.SetUInt8(dt.tm_sec);
    }
    else
    {
        buff.SetUInt8(0xFF);
    }
    //Add ms.
    buff.SetUInt8(0xFF);
    // devitation not used.
    if ((skip & DATETIME_SKIPS_DEVITATION) != 0)
    {
        buff.SetUInt16(0x8000);
    }
    else
    {
        // Add devitation.
        buff.SetUInt16(value.dateTime.GetDeviation());
    }
    // Add clock_status
    if (dt.tm_isdst)
    {
        buff.SetUInt8(value.dateTime.GetStatus() | DLMS_CLOCK_STATUS_DAYLIGHT_SAVE_ACTIVE);
    }
    else
    {
        buff.SetUInt8(value.dateTime.GetStatus());
    }
    return 0;
}

static unsigned char GetBCD(char ch)
{
    if (ch <= '0')
    {
        return ch - '0';
    }
    if (ch <= 'G')
    {
        return ch - 'A';
    }
    return ch - 'a';
}

/**
* Convert BCD to DLMS bytes.
*
* buff
*            Byte buffer where data is write.
* value
*            Added value.
*/
static int SetBcd(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    int ch1, ch2;
    if (value.vt != DLMS_DATA_TYPE_STRING)
    {
        //BCD value must give as std::string.
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    std::string str = value.strVal;
    int len = str.length();
    if (len % 2 != 0)
    {
        str = "0" + str;
        ++len;
    }
    len /= 2;
    buff.SetUInt8(len);
    const char *tmp = str.c_str();
    for (int pos = 0; pos != len; ++pos)
    {
        ch1 = GetBCD(tmp[pos]);
        ch2 = GetBCD(tmp[pos + 1]);
        buff.SetUInt8(ch1 << 4 | ch2);
    }
    return 0;
}

/**
    * Convert Array to DLMS bytes.
    *
    * buff
    *            Byte buffer where data is write.
    * value
    *            Added value.
    */
static int SetArray(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    int ret;
    GXHelpers::SetObjectCount(value.Arr.size(), buff);
    for (std::vector<CGXDLMSVariant>::iterator it = value.Arr.begin(); it != value.Arr.end(); ++it)
    {
        if ((ret = GXHelpers::SetData(buff, it->vt, *it)) != 0)
        {
            return ret;
        }
    }
    return 0;
}

/**
    * Convert Octet std::string to DLMS bytes.
    *
    * buff
    *            Byte buffer where data is write.
    * value
    *            Added value.
    */
static int SetOctetString(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    int val;
    // Example Logical name is octet std::string, so do not change to
    // std::string...
    if (value.vt == DLMS_DATA_TYPE_STRING)
    {
        std::vector< std::string > items = GXHelpers::Split(value.strVal, '.');
        // If data is std::string.
        if (items.size() == 1)
        {
            GXHelpers::SetObjectCount(value.strVal.size(), buff);
            buff.AddString(value.strVal.c_str());
        }
        else
        {
            GXHelpers::SetObjectCount(items.size(), buff);
            for (std::vector< std::string >::iterator it = items.begin(); it != items.end(); ++it)
            {
#if _MSC_VER > 1000
                sscanf_s(it->c_str(), "%d", &val);
#else
                sscanf(it->c_str(), "%d", &val);
#endif
                buff.SetUInt8(val);
            }
        }
    }
    else if (value.vt == DLMS_DATA_TYPE_OCTET_STRING)
    {
        GXHelpers::SetObjectCount(value.size, buff);
        buff.Set(value.byteArr, value.size);
    }
    else if (value.vt == DLMS_DATA_TYPE_NONE)
    {
        GXHelpers::SetObjectCount(0, buff);
    }
    else
    {
        // Invalid data type.
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

/**
* Convert UTC std::string to DLMS bytes.
*
* buff
*            Byte buffer where data is write.
* value
*            Added value.
*/
static int SetUtfString(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    if (value.vt != DLMS_DATA_TYPE_NONE)
    {
        GXHelpers::SetObjectCount(value.strVal.size(), buff);
        buff.AddString(value.strVal.c_str());
    }
    else
    {
        buff.SetUInt8(0);
    }
    return 0;
}

int GXHelpers::SetLogicalName(const char* name, unsigned char ln[6])
{
    int ret;
    int v1, v2, v3, v4, v5, v6;
#if _MSC_VER > 1000
    ret = sscanf_s(name, "%u.%u.%u.%u.%u.%u", &v1, &v2, &v3, &v4, &v5, &v6);
#else
    ret = sscanf(name, "%u.%u.%u.%u.%u.%u", &v1, &v2, &v3, &v4, &v5, &v6);
#endif
    if (ret != 6)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    ln[0] = (unsigned char) v1;
    ln[1] = (unsigned char) v2;
    ln[2] = (unsigned char) v3;
    ln[3] = (unsigned char) v4;
    ln[4] = (unsigned char) v5;
    ln[5] = (unsigned char) v6;
    return DLMS_ERROR_CODE_OK;
}

/**
* Convert ASCII std::string to DLMS bytes.
*
* buff
*            Byte buffer where data is write.
* value
*            Added value.
*/
static int SetString(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    if (value.vt != DLMS_DATA_TYPE_NONE)
    {
        GXHelpers::SetObjectCount(value.strVal.size(), buff);
        buff.AddString(value.strVal.c_str());
    }
    else
    {
        buff.SetUInt8(0);
    }
    return 0;
}

/**
* Convert Bit std::string to DLMS bytes.
*
* buff
*            Byte buffer where data is write.
* value
*            Added value.
*/
static int SetBitString(CGXByteBuffer& buff, CGXDLMSVariant& value)
{
    unsigned char val = 0;
    int ret, index = 0;
    if (value.vt == DLMS_DATA_TYPE_STRING)
    {
        CGXByteBuffer tmp;
        GXHelpers::SetObjectCount(value.strVal.size(), buff);
        for (std::string::iterator it = value.strVal.begin(); it != value.strVal.end(); ++it)
        {
            if (*it == '1')
            {
                val |= (1 << index++);
            }
            else if (*it == '0')
            {
                index++;
            }
            else
            {
                return DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
            if (index == 8)
            {
                index = 0;
                tmp.SetUInt8(val);
                val = 0;
            }
        }
        if (index != 0)
        {
            tmp.SetUInt8(val);
        }
        for (int pos = tmp.GetSize() - 1; pos != -1; --pos)
        {
            if ((ret = tmp.GetUInt8(pos, &val)) != 0)
            {
                return ret;
            }
            buff.SetUInt8(val);
        }
    }
    else if (value.vt == DLMS_DATA_TYPE_OCTET_STRING)
    {
        GXHelpers::SetObjectCount(value.size, buff);
        buff.Set(value.byteArr, value.size);
    }
    else if (value.vt == DLMS_DATA_TYPE_NONE)
    {
        buff.SetUInt8(0);
    }
    else
    {
        //BitString must give as std::string.
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}

int GXHelpers::SetData(CGXByteBuffer& buff, DLMS_DATA_TYPE type, CGXDLMSVariant& value)
{
    if (type == DLMS_DATA_TYPE_OCTET_STRING
            && (value.vt == DLMS_DATA_TYPE_DATETIME || value.vt == DLMS_DATA_TYPE_DATE))
    {
        type = DLMS_DATA_TYPE_DATETIME;
    }
    if (type == DLMS_DATA_TYPE_DATETIME || type == DLMS_DATA_TYPE_DATE ||
            type == DLMS_DATA_TYPE_TIME)
    {
        buff.SetUInt8(DLMS_DATA_TYPE_OCTET_STRING);
    }
    else if ((type == DLMS_DATA_TYPE_ARRAY || type == DLMS_DATA_TYPE_STRUCTURE)
             && value.vt == DLMS_DATA_TYPE_OCTET_STRING)
    {
        // If byte array is added do not add type.
        buff.Set(value.byteArr, value.size);
        return 0;
    }
    else
    {
        buff.SetUInt8(type);
    }
    if (type == DLMS_DATA_TYPE_NONE)
    {
        return 0;
    }
    if (type == DLMS_DATA_TYPE_BOOLEAN)
    {
        buff.SetUInt8(value.boolVal != 0);
    }
    else if (type == DLMS_DATA_TYPE_INT8 || type == DLMS_DATA_TYPE_UINT8
             || type == DLMS_DATA_TYPE_ENUM)
    {
        buff.SetUInt8(value.bVal);
    }
    else if (type == DLMS_DATA_TYPE_INT16 || type == DLMS_DATA_TYPE_UINT16)
    {
        buff.SetUInt16(value.iVal);
    }
    else if (type == DLMS_DATA_TYPE_INT32 || type == DLMS_DATA_TYPE_UINT32)
    {
        buff.SetUInt32(value.lVal);
    }
    else if (type == DLMS_DATA_TYPE_INT64 || type == DLMS_DATA_TYPE_UINT64)
    {
        buff.SetUInt64(value.llVal);
    }
    else if (type == DLMS_DATA_TYPE_FLOAT32)
    {
        buff.SetFloat(value.fltVal);
    }
    else if (type == DLMS_DATA_TYPE_FLOAT64)
    {
        buff.SetDouble(value.dblVal);
    }
    else if (type == DLMS_DATA_TYPE_BIT_STRING)
    {
        return SetBitString(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_STRING)
    {
        return SetString(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_STRING_UTF8)
    {
        return SetUtfString(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_OCTET_STRING)
    {
        return SetOctetString(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_ARRAY || type == DLMS_DATA_TYPE_STRUCTURE)
    {
        return SetArray(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_BINARY_CODED_DESIMAL)
    {
        return SetBcd(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_COMPACT_ARRAY)
    {
        assert(0);
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    else if (type == DLMS_DATA_TYPE_DATETIME)
    {
        return SetDateTime(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_DATE)
    {
        return SetDate(buff, value);
    }
    else if (type == DLMS_DATA_TYPE_TIME)
    {
        return SetTime(buff, value);
    }
    else
    {
        assert(0);
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

unsigned char GXHelpers::GetValue(char c)
{
    unsigned char value;
    if (c > '9')
    {
        if (c > 'Z')
        {
            value = (c - 'a' + 10);
        }
        else
        {
            value = (c - 'A' + 10);
        }
    }
    else
    {
        value = (c - '0');
    }
    return value;
}

/**
     * Convert std::string to byte array.
     *
     * @param value
     *            Hex std::string.
     * @param buffer
     *            byte array.
     * @return Occurred error.
     */
void GXHelpers::HexToBytes(std::string value, CGXByteBuffer& buffer)
{
    buffer.Clear();
    buffer.Capacity(value.length() / 2);
    int lastValue = -1;
    int index = 0;
    for (std::string::iterator ch = value.begin(); ch != value.end(); ++ch)
    {
        if (*ch >= '0' && *ch < 'g')
        {
            if (lastValue == -1)
            {
                lastValue = GetValue(*ch);
            }
            else if (lastValue != -1)
            {
                buffer.SetUInt8(lastValue << 4 | GetValue(*ch));
                lastValue = -1;
                ++index;
            }
        }
        else if (lastValue != -1)
        {
            buffer.SetUInt8(GetValue(*ch));
            lastValue = -1;
            ++index;
        }
    }
}

void GXHelpers::Write(char* fileName, char* pData, int len)
{
    if (len != 0 && pData != NULL)
    {
        std::ofstream trace;
        trace.open(fileName, std::ios::out | std::ios::app);
        trace.write(pData, len);
        trace.close();
    }
}

void GXHelpers::Write(std::string fileName, std::string data)
{
    if (data.size() != 0)
    {
        std::ofstream trace;
        trace.open(fileName.c_str(), std::ios::out | std::ios::app);
        trace.write(&data[0], data.size());
        trace.close();
    }
}

bool GXHelpers::GetBits(unsigned char& tmp, unsigned char BitMask)
{
    return (tmp & BitMask) != 0;
}

bool GXHelpers::StringCompare(const char* c1, const char* c2)
{
    return strcmp(c1, c2) == 0;
}
