﻿// ISOCurrencySet.cpp: implementation of the ISOCurrencySet class.
//
//////////////////////////////////////////////////////////////////////
#include "ISOCurrencySet.h"
#include <set>
#include <string>
#include <string.h>
using namespace  std;

set<string> strISOSet;
#define ADD_ISOCURRENCY(ISOCode) strISOSet.insert(ISOCode)

static void Initialize();

extern "C" BOOL CurrencyCodeFitISOList(const char cCurrency[3])
{
    char temp[4] = {0};
    //strncpy_s(temp, cCurrency, 3);
    strncpy(temp, cCurrency, 3);
    if (strISOSet.size() == 0)
    {
        Initialize();
    }
    set<string>::iterator it = strISOSet.find(temp);
    if (it != strISOSet.end())
    {
        return TRUE;
    }
    return FALSE;
}

void Initialize()
{
    ADD_ISOCURRENCY("AED");
    ADD_ISOCURRENCY("AFN");
    ADD_ISOCURRENCY("ALL");
    ADD_ISOCURRENCY("AMD");
    ADD_ISOCURRENCY("ANG");
    ADD_ISOCURRENCY("AOA");
    ADD_ISOCURRENCY("ARS");
    ADD_ISOCURRENCY("AUD");
    ADD_ISOCURRENCY("AWG");
    ADD_ISOCURRENCY("AZM");
    ADD_ISOCURRENCY("BAM");
    ADD_ISOCURRENCY("BBD");
    ADD_ISOCURRENCY("BDT");
    ADD_ISOCURRENCY("BGN");
    ADD_ISOCURRENCY("BHD");
    ADD_ISOCURRENCY("BIF");
    ADD_ISOCURRENCY("BMD");
    ADD_ISOCURRENCY("BND");
    ADD_ISOCURRENCY("BOB");
    ADD_ISOCURRENCY("BOV");
    ADD_ISOCURRENCY("BRL");
    ADD_ISOCURRENCY("BSD");
    ADD_ISOCURRENCY("BTN");
    ADD_ISOCURRENCY("BWP");
    ADD_ISOCURRENCY("BYR");
    ADD_ISOCURRENCY("BZD");
    ADD_ISOCURRENCY("CAD");
    ADD_ISOCURRENCY("CDF");
    ADD_ISOCURRENCY("CHF");
    ADD_ISOCURRENCY("CLF");
    ADD_ISOCURRENCY("CLP");
    ADD_ISOCURRENCY("CNH");
    ADD_ISOCURRENCY("CNY");
    ADD_ISOCURRENCY("COP");
    ADD_ISOCURRENCY("COU");
    ADD_ISOCURRENCY("CRC");
    ADD_ISOCURRENCY("CSD");
    ADD_ISOCURRENCY("CUC");
    ADD_ISOCURRENCY("CUP");
    ADD_ISOCURRENCY("CVE");
    ADD_ISOCURRENCY("CYP");
    ADD_ISOCURRENCY("CZK");
    ADD_ISOCURRENCY("DJF");
    ADD_ISOCURRENCY("DKK");
    ADD_ISOCURRENCY("DOP");
    ADD_ISOCURRENCY("DZD");
    ADD_ISOCURRENCY("EEK");
    ADD_ISOCURRENCY("EGP");
    ADD_ISOCURRENCY("ERN");
    ADD_ISOCURRENCY("ETB");
    ADD_ISOCURRENCY("EUR");
    ADD_ISOCURRENCY("FJD");
    ADD_ISOCURRENCY("FKP");
    ADD_ISOCURRENCY("GBP");
    ADD_ISOCURRENCY("GEL");
    ADD_ISOCURRENCY("GHC");
    ADD_ISOCURRENCY("GIP");
    ADD_ISOCURRENCY("GMD");
    ADD_ISOCURRENCY("GNF");
    ADD_ISOCURRENCY("GTQ");
    ADD_ISOCURRENCY("GYD");
    ADD_ISOCURRENCY("HKD");
    ADD_ISOCURRENCY("HNL");
    ADD_ISOCURRENCY("HRK");
    ADD_ISOCURRENCY("HTG");
    ADD_ISOCURRENCY("HUF");
    ADD_ISOCURRENCY("IDR");
    ADD_ISOCURRENCY("ILS");
    ADD_ISOCURRENCY("INR");
    ADD_ISOCURRENCY("IQD");
    ADD_ISOCURRENCY("IRR");
    ADD_ISOCURRENCY("ISK");
    ADD_ISOCURRENCY("JMD");
    ADD_ISOCURRENCY("JOD");
    ADD_ISOCURRENCY("JPY");
    ADD_ISOCURRENCY("KES");
    ADD_ISOCURRENCY("KGS");
    ADD_ISOCURRENCY("KHR");
    ADD_ISOCURRENCY("KMF");
    ADD_ISOCURRENCY("KPW");
    ADD_ISOCURRENCY("KRW");
    ADD_ISOCURRENCY("KWD");
    ADD_ISOCURRENCY("KYD");
    ADD_ISOCURRENCY("KZT");
    ADD_ISOCURRENCY("LAK");
    ADD_ISOCURRENCY("LBP");
    ADD_ISOCURRENCY("LKR");
    ADD_ISOCURRENCY("LRD");
    ADD_ISOCURRENCY("LSL");
    ADD_ISOCURRENCY("LTL");
    ADD_ISOCURRENCY("LVL");
    ADD_ISOCURRENCY("LYD");
    ADD_ISOCURRENCY("MAD");
    ADD_ISOCURRENCY("MDL");
    ADD_ISOCURRENCY("MGA");
    ADD_ISOCURRENCY("MKD");
    ADD_ISOCURRENCY("MMK");
    ADD_ISOCURRENCY("MNT");
    ADD_ISOCURRENCY("MOP");
    ADD_ISOCURRENCY("MRO");
    ADD_ISOCURRENCY("MTL");
    ADD_ISOCURRENCY("MUR");
    ADD_ISOCURRENCY("MVR");
    ADD_ISOCURRENCY("MWK");
    ADD_ISOCURRENCY("MXN");
    ADD_ISOCURRENCY("MXV");
    ADD_ISOCURRENCY("MYR");
    ADD_ISOCURRENCY("MZM");
    ADD_ISOCURRENCY("NAD");
    ADD_ISOCURRENCY("NGN");
    ADD_ISOCURRENCY("NIO");
    ADD_ISOCURRENCY("NOK");
    ADD_ISOCURRENCY("NPR");
    ADD_ISOCURRENCY("NZD");
    ADD_ISOCURRENCY("OMR");
    ADD_ISOCURRENCY("PAB");
    ADD_ISOCURRENCY("PEN");
    ADD_ISOCURRENCY("PGK");
    ADD_ISOCURRENCY("PHP");
    ADD_ISOCURRENCY("PKR");
    ADD_ISOCURRENCY("PLN");
    ADD_ISOCURRENCY("PYG");
    ADD_ISOCURRENCY("QAR");
    ADD_ISOCURRENCY("ROL");
    ADD_ISOCURRENCY("RUB");
    ADD_ISOCURRENCY("RWF");
    ADD_ISOCURRENCY("SAR");
    ADD_ISOCURRENCY("SBD");
    ADD_ISOCURRENCY("SCR");
    ADD_ISOCURRENCY("SDD");
    ADD_ISOCURRENCY("SEK");
    ADD_ISOCURRENCY("SGD");
    ADD_ISOCURRENCY("SHP");
    ADD_ISOCURRENCY("SIT");
    ADD_ISOCURRENCY("SKK");
    ADD_ISOCURRENCY("SLL");
    ADD_ISOCURRENCY("SOS");
    ADD_ISOCURRENCY("SRD");
    ADD_ISOCURRENCY("STD");
    ADD_ISOCURRENCY("SVC");
    ADD_ISOCURRENCY("SYP");
    ADD_ISOCURRENCY("SZL");
    ADD_ISOCURRENCY("THB");
    ADD_ISOCURRENCY("TJS");
    ADD_ISOCURRENCY("TMM");
    ADD_ISOCURRENCY("TND");
    ADD_ISOCURRENCY("TOP");
    ADD_ISOCURRENCY("TRL");
    ADD_ISOCURRENCY("TRY");
    ADD_ISOCURRENCY("TTD");
    ADD_ISOCURRENCY("TWD");
    ADD_ISOCURRENCY("TZS");
    ADD_ISOCURRENCY("UAH");
    ADD_ISOCURRENCY("UGX");
    ADD_ISOCURRENCY("USD");
    ADD_ISOCURRENCY("USN");
    ADD_ISOCURRENCY("USS");
    ADD_ISOCURRENCY("UYI");
    ADD_ISOCURRENCY("UYU");
    ADD_ISOCURRENCY("UZS");
    ADD_ISOCURRENCY("VEB");
    ADD_ISOCURRENCY("VEF");
    ADD_ISOCURRENCY("VND");
    ADD_ISOCURRENCY("VUV");
    ADD_ISOCURRENCY("WST");
    ADD_ISOCURRENCY("XAF");
    ADD_ISOCURRENCY("XAG");
    ADD_ISOCURRENCY("XAU");
    ADD_ISOCURRENCY("XBA");
    ADD_ISOCURRENCY("XBB");
    ADD_ISOCURRENCY("XBC");
    ADD_ISOCURRENCY("XBD");
    ADD_ISOCURRENCY("XCD");
    ADD_ISOCURRENCY("XDR");
    ADD_ISOCURRENCY("XFO");
    ADD_ISOCURRENCY("XFU");
    ADD_ISOCURRENCY("XOF");
    ADD_ISOCURRENCY("XPD");
    ADD_ISOCURRENCY("XPF");
    ADD_ISOCURRENCY("XPT");
    ADD_ISOCURRENCY("XTS");
    ADD_ISOCURRENCY("XXX");
    ADD_ISOCURRENCY("YER");
    ADD_ISOCURRENCY("ZAR");
    ADD_ISOCURRENCY("ZMK");
    ADD_ISOCURRENCY("ZWD");
}
