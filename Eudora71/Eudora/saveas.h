// SAVEAS.H
//
#ifndef SAVEAS_H
#define SAVEAS_H

class JJFile;

class CSaveAs
{
public:
	CSaveAs();
	~CSaveAs();
	
	BOOL Start(const char* Filename = NULL, BOOL IsMessage = FALSE, CWnd* pParent = NULL, BOOL stationary = FALSE);
	BOOL CreateFile(const char* Filename);
	BOOL PutText(const char* Text);
	BOOL IsStationery() {return m_Stationery;}
	CString GetPathName() { return m_PathName; }
	JJFile* GetFilePtr() { return m_File; }
	
protected:
	BOOL			m_Stationery;
	JJFile*			m_File;
	CString			m_PathName;
};

#endif   
