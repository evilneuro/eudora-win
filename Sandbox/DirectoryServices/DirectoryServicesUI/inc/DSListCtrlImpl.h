///////////////////////////////////////////////////////////////////////////////
// DSListCtrlImpl.h
// 
// Created: 09/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_LIST_CTRL_IMPL_H_
#define __DS_LIST_CTRL_IMPL_H_

// Enforce that only DSListCtrlImpl.cpp includes it.
#ifndef __DS_LIST_CTRL_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSListCtrlImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "guiutils.h"
#include "font.h"


#include "DSUtilsIface.h"
#include "DSListCtrlIface.h"
#include "DirectoryServicesCritSectionIface.h" // DS Template Crit Section Impl

char *m_pszText[] ={
    "Name",
    "Email",
    "Phone",
    "Database"
};

char *fieldTbl[] = {
    "Name",         // Name (ie. "Subhashis Mohanty")
    "Company",      // Company name
    "Division",     // Division of company
    "Position",     // Position held @ company
    "Location",     // Location in co. (ie. Bldg I-215A)
    "Address",	    // Work address
    "City",         // Work city
    "State",        // Work state
    "Zipcode",	    // Work Zip Code
    "Phone",        // Work phone number
    "Cell",	    // Work cell phone number
    "Pager",        // Work pager number
    "Pemail",	    // Work pager email address
    "Fax",          // Work Fax number
    "Email",        // Work email address
    "Pgpkey",	    // Work related PGP key
    "Pagertype",    // Text representation of Pager Type
    "Pagerpin",     // Pager PIN
    "Haddress",     // Home address
    "Hcity",        // Home city
    "Hstate",	    // Home state
    "Hzipcode",     // Home Zip Code
    "Hphone",       // Home phone number
    "Hcell", 	    // Home cell phone number
    "Hpager",	    // Home pager number
    "Hpemail",	    // Home pager email address
    "Hfax",         // Home Fax number
    "Hemail",       // Home email address
    "Hpgpkey",      // Home related PGP key
    "Hpagertype",   // Text representation of Pager Type
    "Hpagerpin",    // Pager PIN
    "Userdefined",  // User defined attribute
    "Unknown",	    // Unknown data type
    "Comments",     // Comments
    "Emailalias",   // Email Alias
    "Database"      // Database name
};

bool ShouldUpdateIni(int code = -1);

#endif // __DS_LIST_CTRL_IMPL_H_
