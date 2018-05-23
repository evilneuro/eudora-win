// lang_info.cpp -- retrieving locale info under Win32

#include <afxwin.h>

#include "stdlib.h"
#include "stdio.h"
#include "tchar.h"
#include "assert.h"

#include "EuLang.h"

#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// table for mapping between 3166 Alpha-2 and Alpha-3

typedef struct _iso3166_map_ {
   TCHAR a2[3];
   TCHAR a3[4];
} ISO3166_MAP;

ISO3166_MAP g_countryCodeTable[] = {
    _T("AF"),   _T("AFG"),            // AFGHANISTAN
    _T("AL"),   _T("ALB"),            // ALBANIA
    _T("DZ"),   _T("DZA"),            // ALGERIA
    _T("AS"),   _T("ASM"),            // AMERICAN SAMOA
    _T("AD"),   _T("AND"),            // ANDORRA
    _T("AO"),   _T("AGO"),            // ANGOLA
    _T("AI"),   _T("AIA"),            // ANGUILLA
    _T("AQ"),   _T("ATA"),            // ANTARCTICA
    _T("AG"),   _T("ATG"),            // ANTIGUA AND BARBUDA
    _T("AR"),   _T("ARG"),            // ARGENTINA
    _T("AM"),   _T("ARM"),            // ARMENIA
    _T("AW"),   _T("ABW"),            // ARUBA
    _T("AU"),   _T("AUS"),            // AUSTRALIA
    _T("AT"),   _T("AUT"),            // AUSTRIA
    _T("AZ"),   _T("AZE"),            // AZERBAIJAN
    _T("BS"),   _T("BHS"),            // BAHAMAS
    _T("BH"),   _T("BHR"),            // BAHRAIN
    _T("BD"),   _T("BGD"),            // BANGLADESH
    _T("BB"),   _T("BRB"),            // BARBADOS
    _T("BY"),   _T("BLR"),            // BELARUS
    _T("BE"),   _T("BEL"),            // BELGIUM
    _T("BZ"),   _T("BLZ"),            // BELIZE
    _T("BJ"),   _T("BEN"),            // BENIN
    _T("BM"),   _T("BMU"),            // BERMUDA
    _T("BT"),   _T("BTN"),            // BHUTAN
    _T("BO"),   _T("BOL"),            // BOLIVIA
    _T("BA"),   _T("BIH"),            // BOSNIA AND HERZEGOWINA
    _T("BW"),   _T("BWA"),            // BOTSWANA
    _T("BV"),   _T("BVT"),            // BOUVET ISLAND
    _T("BR"),   _T("BRA"),            // BRAZIL
    _T("IO"),   _T("IOT"),            // BRITISH INDIAN OCEAN TERRITORY
    _T("BN"),   _T("BRN"),            // BRUNEI DARUSSALAM
    _T("BG"),   _T("BGR"),            // BULGARIA
    _T("BF"),   _T("BFA"),            // BURKINA FASO
    _T("BI"),   _T("BDI"),            // BURUNDI
    _T("KH"),   _T("KHM"),            // CAMBODIA
    _T("CM"),   _T("CMR"),            // CAMEROON
    _T("CA"),   _T("CAN"),            // CANADA
    _T("CV"),   _T("CPV"),            // CAPE VERDE
    _T("KY"),   _T("CYM"),            // CAYMAN ISLANDS
    _T("CF"),   _T("CAF"),            // CENTRAL AFRICAN REPUBLIC
    _T("TD"),   _T("TCD"),            // CHAD
    _T("CL"),   _T("CHL"),            // CHILE
    _T("CN"),   _T("CHN"),            // CHINA
    _T("CX"),   _T("CXR"),            // CHRISTMAS ISLAND
    _T("CC"),   _T("CCK"),            // COCOS (KEELING) ISLANDS
    _T("CO"),   _T("COL"),            // COLOMBIA
    _T("KM"),   _T("COM"),            // COMOROS
    _T("CG"),   _T("COG"),            // CONGO
    _T("CD"),   _T("COD"),            // CONGO, THE DEMOCRATIC REPUBLIC OF THE
    _T("CK"),   _T("COK"),            // COOK ISLANDS
    _T("CR"),   _T("CRI"),            // COSTA RICA
    _T("CI"),   _T("CIV"),            // COTE D'IVOIRE
    _T("HR"),   _T("HRV"),            // CROATIA (local name: Hrvatska)
    _T("CU"),   _T("CUB"),            // CUBA
    _T("CY"),   _T("CYP"),            // CYPRUS
    _T("CZ"),   _T("CZE"),            // CZECH REPUBLIC
    _T("DK"),   _T("DNK"),            // DENMARK
    _T("DJ"),   _T("DJI"),            // DJIBOUTI
    _T("DM"),   _T("DMA"),            // DOMINICA
    _T("DO"),   _T("DOM"),            // DOMINICAN REPUBLIC
    _T("TP"),   _T("TMP"),            // EAST TIMOR
    _T("EC"),   _T("ECU"),            // ECUADOR
    _T("EG"),   _T("EGY"),            // EGYPT
    _T("SV"),   _T("SLV"),            // EL SALVADOR
    _T("GQ"),   _T("GNQ"),            // EQUATORIAL GUINEA
    _T("ER"),   _T("ERI"),            // ERITREA
    _T("EE"),   _T("EST"),            // ESTONIA
    _T("ET"),   _T("ETH"),            // ETHIOPIA
    _T("FK"),   _T("FLK"),            // FALKLAND ISLANDS (MALVINAS)
    _T("FO"),   _T("FRO"),            // FAROE ISLANDS
    _T("FJ"),   _T("FJI"),            // FIJI
    _T("FI"),   _T("FIN"),            // FINLAND
    _T("FR"),   _T("FRA"),            // FRANCE
    _T("FX"),   _T("FXX"),            // FRANCE, METROPOLITAN
    _T("GF"),   _T("GUF"),            // FRENCH GUIANA
    _T("PF"),   _T("PYF"),            // FRENCH POLYNESIA
    _T("TF"),   _T("ATF"),            // FRENCH SOUTHERN TERRITORIES
    _T("GA"),   _T("GAB"),            // GABON
    _T("GM"),   _T("GMB"),            // GAMBIA
    _T("GE"),   _T("GEO"),            // GEORGIA
    _T("DE"),   _T("DEU"),            // GERMANY
    _T("GH"),   _T("GHA"),            // GHANA
    _T("GI"),   _T("GIB"),            // GIBRALTAR
    _T("GR"),   _T("GRC"),            // GREECE
    _T("GL"),   _T("GRL"),            // GREENLAND
    _T("GD"),   _T("GRD"),            // GRENADA
    _T("GP"),   _T("GLP"),            // GUADELOUPE
    _T("GU"),   _T("GUM"),            // GUAM
    _T("GT"),   _T("GTM"),            // GUATEMALA
    _T("GN"),   _T("GIN"),            // GUINEA
    _T("GW"),   _T("GNB"),            // GUINEA-BISSAU
    _T("GY"),   _T("GUY"),            // GUYANA
    _T("HT"),   _T("HTI"),            // HAITI
    _T("HM"),   _T("HMD"),            // HEARD AND MC DONALD ISLANDS
    _T("VA"),   _T("VAT"),            // HOLY SEE (VATICAN CITY STATE)
    _T("HN"),   _T("HND"),            // HONDURAS
    _T("HK"),   _T("HKG"),            // HONG KONG
    _T("HU"),   _T("HUN"),            // HUNGARY
    _T("IS"),   _T("ISL"),            // ICELAND
    _T("IN"),   _T("IND"),            // INDIA
    _T("ID"),   _T("IDN"),            // INDONESIA
    _T("IR"),   _T("IRN"),            // IRAN (ISLAMIC REPUBLIC OF)
    _T("IQ"),   _T("IRQ"),            // IRAQ
    _T("IE"),   _T("IRL"),            // IRELAND
    _T("IL"),   _T("ISR"),            // ISRAEL
    _T("IT"),   _T("ITA"),            // ITALY
    _T("JM"),   _T("JAM"),            // JAMAICA
    _T("JP"),   _T("JPN"),            // JAPAN
    _T("JO"),   _T("JOR"),            // JORDAN
    _T("KZ"),   _T("KAZ"),            // KAZAKHSTAN
    _T("KE"),   _T("KEN"),            // KENYA
    _T("KI"),   _T("KIR"),            // KIRIBATI
    _T("KP"),   _T("PRK"),            // KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF
    _T("KR"),   _T("KOR"),            // KOREA, REPUBLIC OF
    _T("KW"),   _T("KWT"),            // KUWAIT
    _T("KG"),   _T("KGZ"),            // KYRGYZSTAN
    _T("LA"),   _T("LAO"),            // LAO PEOPLE'S DEMOCRATIC REPUBLIC
    _T("LV"),   _T("LVA"),            // LATVIA
    _T("LB"),   _T("LBN"),            // LEBANON
    _T("LS"),   _T("LSO"),            // LESOTHO
    _T("LR"),   _T("LBR"),            // LIBERIA
    _T("LY"),   _T("LBY"),            // LIBYAN ARAB JAMAHIRIYA
    _T("LI"),   _T("LIE"),            // LIECHTENSTEIN
    _T("LT"),   _T("LTU"),            // LITHUANIA
    _T("LU"),   _T("LUX"),            // LUXEMBOURG
    _T("MO"),   _T("MAC"),            // MACAU
    _T("MK"),   _T("MKD"),            // MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
    _T("MG"),   _T("MDG"),            // MADAGASCAR
    _T("MW"),   _T("MWI"),            // MALAWI
    _T("MY"),   _T("MYS"),            // MALAYSIA
    _T("MV"),   _T("MDV"),            // MALDIVES
    _T("ML"),   _T("MLI"),            // MALI
    _T("MT"),   _T("MLT"),            // MALTA
    _T("MH"),   _T("MHL"),            // MARSHALL ISLANDS
    _T("MQ"),   _T("MTQ"),            // MARTINIQUE
    _T("MR"),   _T("MRT"),            // MAURITANIA
    _T("MU"),   _T("MUS"),            // MAURITIUS
    _T("YT"),   _T("MYT"),            // MAYOTTE
    _T("MX"),   _T("MEX"),            // MEXICO
    _T("FM"),   _T("FSM"),            // MICRONESIA, FEDERATED STATES OF
    _T("MD"),   _T("MDA"),            // MOLDOVA, REPUBLIC OF
    _T("MC"),   _T("MCO"),            // MONACO
    _T("MN"),   _T("MNG"),            // MONGOLIA
    _T("MS"),   _T("MSR"),            // MONTSERRAT
    _T("MA"),   _T("MAR"),            // MOROCCO
    _T("MZ"),   _T("MOZ"),            // MOZAMBIQUE
    _T("MM"),   _T("MMR"),            // MYANMAR
    _T("NA"),   _T("NAM"),            // NAMIBIA
    _T("NR"),   _T("NRU"),            // NAURU
    _T("NP"),   _T("NPL"),            // NEPAL
    _T("NL"),   _T("NLD"),            // NETHERLANDS
    _T("AN"),   _T("ANT"),            // NETHERLANDS ANTILLES
    _T("NC"),   _T("NCL"),            // NEW CALEDONIA
    _T("NZ"),   _T("NZL"),            // NEW ZEALAND
    _T("NI"),   _T("NIC"),            // NICARAGUA
    _T("NE"),   _T("NER"),            // NIGER
    _T("NG"),   _T("NGA"),            // NIGERIA
    _T("NU"),   _T("NIU"),            // NIUE
    _T("NF"),   _T("NFK"),            // NORFOLK ISLAND
    _T("MP"),   _T("MNP"),            // NORTHERN MARIANA ISLANDS
    _T("NO"),   _T("NOR"),            // NORWAY
    _T("OM"),   _T("OMN"),            // OMAN
    _T("PK"),   _T("PAK"),            // PAKISTAN
    _T("PW"),   _T("PLW"),            // PALAU
    _T("PA"),   _T("PAN"),            // PANAMA
    _T("PG"),   _T("PNG"),            // PAPUA NEW GUINEA
    _T("PY"),   _T("PRY"),            // PARAGUAY
    _T("PE"),   _T("PER"),            // PERU
    _T("PH"),   _T("PHL"),            // PHILIPPINES
    _T("PN"),   _T("PCN"),            // PITCAIRN
    _T("PL"),   _T("POL"),            // POLAND
    _T("PT"),   _T("PRT"),            // PORTUGAL
    _T("PR"),   _T("PRI"),            // PUERTO RICO
    _T("QA"),   _T("QAT"),            // QATAR
    _T("RE"),   _T("REU"),            // REUNION
    _T("RO"),   _T("ROM"),            // ROMANIA
    _T("RU"),   _T("RUS"),            // RUSSIAN FEDERATION
    _T("RW"),   _T("RWA"),            // RWANDA
    _T("KN"),   _T("KNA"),            // SAINT KITTS AND NEVIS
    _T("LC"),   _T("LCA"),            // SAINT LUCIA
    _T("VC"),   _T("VCT"),            // SAINT VINCENT AND THE GRENADINES
    _T("WS"),   _T("WSM"),            // SAMOA
    _T("SM"),   _T("SMR"),            // SAN MARINO
    _T("ST"),   _T("STP"),            // SAO TOME AND PRINCIPE
    _T("SA"),   _T("SAU"),            // SAUDI ARABIA
    _T("SN"),   _T("SEN"),            // SENEGAL
    _T("SC"),   _T("SYC"),            // SEYCHELLES
    _T("SL"),   _T("SLE"),            // SIERRA LEONE
    _T("SG"),   _T("SGP"),            // SINGAPORE
    _T("SK"),   _T("SVK"),            // SLOVAKIA (Slovak Republic)
    _T("SI"),   _T("SVN"),            // SLOVENIA
    _T("SB"),   _T("SLB"),            // SOLOMON ISLANDS
    _T("SO"),   _T("SOM"),            // SOMALIA
    _T("ZA"),   _T("ZAF"),            // SOUTH AFRICA
    _T("GS"),   _T("SGS"),            // SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS
    _T("ES"),   _T("ESP"),            // SPAIN
    _T("LK"),   _T("LKA"),            // SRI LANKA
    _T("SH"),   _T("SHN"),            // ST. HELENA
    _T("PM"),   _T("SPM"),            // ST. PIERRE AND MIQUELON
    _T("SD"),   _T("SDN"),            // SUDAN
    _T("SR"),   _T("SUR"),            // SURINAME
    _T("SJ"),   _T("SJM"),            // SVALBARD AND JAN MAYEN ISLANDS
    _T("SZ"),   _T("SWZ"),            // SWAZILAND
    _T("SE"),   _T("SWE"),            // SWEDEN
    _T("CH"),   _T("CHE"),            // SWITZERLAND
    _T("SY"),   _T("SYR"),            // SYRIAN ARAB REPUBLIC
    _T("TW"),   _T("TWN"),            // TAIWAN, PROVINCE OF CHINA
    _T("TJ"),   _T("TJK"),            // TAJIKISTAN
    _T("TZ"),   _T("TZA"),            // TANZANIA, UNITED REPUBLIC OF
    _T("TH"),   _T("THA"),            // THAILAND
    _T("TG"),   _T("TGO"),            // TOGO
    _T("TK"),   _T("TKL"),            // TOKELAU
    _T("TO"),   _T("TON"),            // TONGA
    _T("TT"),   _T("TTO"),            // TRINIDAD AND TOBAGO
    _T("TN"),   _T("TUN"),            // TUNISIA
    _T("TR"),   _T("TUR"),            // TURKEY
    _T("TM"),   _T("TKM"),            // TURKMENISTAN
    _T("TC"),   _T("TCA"),            // TURKS AND CAICOS ISLANDS
    _T("TV"),   _T("TUV"),            // TUVALU
    _T("UG"),   _T("UGA"),            // UGANDA
    _T("UA"),   _T("UKR"),            // UKRAINE
    _T("AE"),   _T("ARE"),            // UNITED ARAB EMIRATES
    _T("GB"),   _T("GBR"),            // UNITED KINGDOM
    _T("US"),   _T("USA"),            // UNITED STATES
    _T("UM"),   _T("UMI"),            // UNITED STATES MINOR OUTLYING ISLANDS
    _T("UY"),   _T("URY"),            // URUGUAY
    _T("UZ"),   _T("UZB"),            // UZBEKISTAN
    _T("VU"),   _T("VUT"),            // VANUATU
    _T("VE"),   _T("VEN"),            // VENEZUELA
    _T("VN"),   _T("VNM"),            // VIET NAM
    _T("VG"),   _T("VGB"),            // VIRGIN ISLANDS (BRITISH)
    _T("VI"),   _T("VIR"),            // VIRGIN ISLANDS (U.S.)
    _T("WF"),   _T("WLF"),            // WALLIS AND FUTUNA ISLANDS
    _T("EH"),   _T("ESH"),            // WESTERN SAHARA
    _T("YE"),   _T("YEM"),            // YEMEN
    _T("YU"),   _T("YUG"),            // YUGOSLAVIA
    _T("ZM"),   _T("ZMB"),            // ZAMBIA
    _T("ZW"),   _T("ZWE"),            // ZIMBABWE

    _T(""),     _T("")                // end of table
};

bool A3toA2( LPCTSTR a3, TCHAR a2[3] )
{
	bool bRet = false;
	LPTSTR s = 0;

	for ( int i = 0; *(s = g_countryCodeTable[i].a3); i++ ) {
		if ( *s == *a3 && *(s+1) == *(a3+1) && *(s+2) == *(a3+2) ) {
			_tcscpy( a2, g_countryCodeTable[i].a2 );
			bRet = true;
			break;
		}
	}

	// countries come and go; are we up to date?
	assert( *a2 != (TCHAR)0 );
	return bRet;
}


///////////////////////////////////////////////////////////////////////////////
// language info

// some languages come in flavors [e.g. "Italian (Swiss)"] and we keep a list
// of associated lang ids to match against. this is less cumbersome than
// matching against the sub-lang ids, as the list is much smaller and we only
// need to know that there is more than one flavor.
bool HasSubLang( WORD lid )
{
	bool bRet = false;

	switch ( PRIMARYLANGID(lid) ) {
		case LANG_ARABIC:
		case LANG_CHINESE:
		case LANG_DUTCH:
		case LANG_ENGLISH:
		case LANG_FRENCH:
		case LANG_GERMAN:
		case LANG_ITALIAN:
		case LANG_KOREAN:
		case LANG_NORWEGIAN:
		case LANG_PORTUGUESE:
		case LANG_SERBIAN:
		case LANG_SPANISH:
		case LANG_SWEDISH:
			bRet = true;
			break;
	}

	return bRet;
}

// everything we know about the user's mother tongue
struct LInfo
{
	LPTSTR iso639Full;
	TCHAR  iso639Abbrev[3];
	TCHAR  iso3166A2[3];
	TCHAR  iso3166A3[4];
	TCHAR  rfc1766[6];
	LPTSTR localName;

	LInfo(){initialized = false;}
	~LInfo();

private:
	bool initialized;
	bool init_linfo();

	friend LNG_GetLanguageInfo( int, LPTSTR );
};

// Frankenstein
bool LInfo::init_linfo()
{
	if ( initialized )
		return true;

	memset( this, 0, sizeof(LInfo) );

	// get the full iso 639 language name
	LCTYPE LCType = LOCALE_SENGLANGUAGE;
	int nBytes = GetLocaleInfo( LOCALE_USER_DEFAULT, LCType, 0, 0 );
	LPTSTR buf = DEBUG_NEW TCHAR[nBytes + 1];
	GetLocaleInfo( LOCALE_USER_DEFAULT, LCType, buf, nBytes );
	iso639Full = buf;

	// the iso 639 two character abbreviation
	LCType = LOCALE_SABBREVLANGNAME;
	nBytes = sizeof(iso639Abbrev);
	GetLocaleInfo( LOCALE_USER_DEFAULT, LCType, iso639Abbrev, nBytes );
	iso639Abbrev[2] = 0;

	// lower case by popular convention
	_tcslwr( iso639Abbrev );

	// the iso 3166 country codes (Alpha-2 and 3)
	LCType = LOCALE_SABBREVCTRYNAME;
	nBytes = sizeof(iso3166A3);
	GetLocaleInfo( LOCALE_USER_DEFAULT, LCType, iso3166A3, nBytes );
	A3toA2( iso3166A3, iso3166A2 );

	// if this is a sub-language [e.g. "English (United States)"] we append the
	// two character country code to create a fully qualified RFC 1766 code.
	_tcscpy( rfc1766, iso639Abbrev );

	if ( HasSubLang(GetUserDefaultLangID()) ) {
		if ( *(iso3166A2) ) {
			_tcscat( rfc1766, _T("-") );
			_tcscat( rfc1766, iso3166A2 );
		}
	}

	// the full localized language name
	LCType = LOCALE_SLANGUAGE;
	nBytes = GetLocaleInfo( LOCALE_USER_DEFAULT, LCType, 0, 0 );
	buf = DEBUG_NEW TCHAR[nBytes + 1];
	GetLocaleInfo( LOCALE_USER_DEFAULT, LCType, buf, nBytes );
	localName = buf;

	initialized = true;
	return true;   // TODO: error handling would be nice!
}

// Kevorkian
LInfo::~LInfo()
{
	delete iso639Full;
	delete localName;
}

// our one and only instance
LInfo g_langInfo;


///////////////////////////////////////////////////////////////////////////////
// public api

int LNG_GetLanguageInfo( int what, LPTSTR out )
{
	int iRet = 0;   // return data length

	// performs one-time init
	g_langInfo.init_linfo();

	switch ( what ) {
		case LANG_INFO_ISO639FULL:
			if ( out )
				_tcscpy( out, g_langInfo.iso639Full );

			iRet = _tcslen( g_langInfo.iso639Full );
			break;
		case LANG_INFO_ISO639ABBREV:
			if ( out )
				_tcscpy( out, g_langInfo.iso639Abbrev );

			iRet = _tcslen( g_langInfo.iso639Abbrev );
			break;
		case LANG_INFO_ISO3166A2:
			if ( out )
				_tcscpy( out, g_langInfo.iso3166A2 );

			iRet = _tcslen( g_langInfo.iso3166A2 );
			break;
		case LANG_INFO_ISO3166A3:
			if ( out )
				_tcscpy( out, g_langInfo.iso3166A3 );

			iRet = _tcslen( g_langInfo.iso3166A3 );
			break;
		case LANG_INFO_RFC1766:
			if ( out )
				_tcscpy( out, g_langInfo.rfc1766 );

			iRet = _tcslen( g_langInfo.rfc1766 );
			break;
		case LANG_INFO_LOCALNAME:
			if ( out )
				_tcscpy( out, g_langInfo.localName );

			iRet = _tcslen( g_langInfo.localName );
			break;

		default:
			assert(0);   // the caller is confused
	}

	return iRet;
}


///////////////////////////////////////////////////////////////////////////////
// test stub

#if defined(STUB)
void printline( LPCTSTR s )
{
	printf( "%s\n", s );
}

int main( void )
{
	TCHAR buf[128];   // way too big for anything

	for ( int i = 0; i < LANG_INFO_MAX; i++ ) {
		LNG_GetLanguageInfo( i, buf );
		printline( buf );
	}

	return 0;
}
#endif
