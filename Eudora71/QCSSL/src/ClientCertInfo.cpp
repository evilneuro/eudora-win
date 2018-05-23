#include "stdafx.h"

#include "clientcertinfo.h"



CStringList* ClientCertInfo::GetHeirarchy()
{
	return &m_Heirarchy;
}

void ClientCertInfo::SetHeirarchy(CStringList&  heir)
{
	m_Heirarchy.RemoveAll();
  	for(POSITION pos = heir.GetHeadPosition(); pos != NULL; )
   {
    CString csTemp = heir.GetNext( pos );
	heir.AddTail(csTemp);
   }
}


bool ClientCertInfo::Read(const CString& filename)
{
	m_Heirarchy.RemoveAll();
	CStdioFile thisFile;
	bool bSuccess = true;
	if( !thisFile.Open( filename, CFile::modeRead|CFile::typeText ) )
		return false;
	CString csTemp;
	while(thisFile.ReadString(csTemp))
	{
		if(csTemp[0] == '#') //this is a comment
			continue;
		int pos = csTemp.Find("=",0);
		if(pos<=0)//invalid line so ignore
			continue;
		CString part1, part2;
		part1 = csTemp.Left(pos);
		part1.TrimLeft();
		part1.TrimRight();
		part2 = csTemp.Mid(pos+1);
		part2.TrimLeft();
		part2.TrimRight();
		if(part1.CompareNoCase("version") ==0)
		{
			if(part2 !="1.0") bSuccess = false;
		}
		if(part1.CompareNoCase("hash")==0)
			m_Heirarchy.AddTail(part2);
	}
    thisFile.Close();
	return bSuccess;
}


bool ClientCertInfo::Write(const CString& filename)
{

	CStdioFile thisFile;
	bool bSuccess = true;
	if( !thisFile.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeText ) )
		return false;
	CString csTemp;
	thisFile.WriteString("#Please edit this file(version 1.0) to change the heirarchy\r\n");
    thisFile.WriteString("version=1.0\r\n");
	   
	for(POSITION pos = m_Heirarchy.GetHeadPosition(); pos != NULL; )
   {
    CString csTemp = m_Heirarchy.GetNext( pos );
	thisFile.WriteString("hash=");

	thisFile.WriteString(csTemp);
	thisFile.WriteString("\r\n");
   }

    thisFile.Close();
	return bSuccess;


}

