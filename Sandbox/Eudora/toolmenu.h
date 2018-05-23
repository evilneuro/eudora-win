// toolmenu.h
//
// The toobar buttons that trigger menus
//

#ifndef _TOOLMENU_H_
#define _TOOLMENU_H_
#endif


class CTranslator;
class CTranslatorMenuItem;
//===================================================================
//
//===================================================================
class CTranslatorMenuItem : public CUserMenuItem	
{
public:	
	CTranslatorMenuItem(CTranslator *t, const char *desc, UINT menuID);
	virtual ~CTranslatorMenuItem(){};

	CTranslator *GetTranslator(){return m_Translator;}
		
// Attributes
public:
	CTranslator *m_Translator;
	HICON		m_HIcon;
	CString		m_Properties;
};

//===================================================================
//
//===================================================================
class CTranslatorMenu : public CUserMenu
{
// Implementation
public:
	virtual ~CTranslatorMenu(){};
	virtual BOOL InitMenu(const long type,const long context, BOOL Check = FALSE, BOOL Popup = FALSE);
	virtual BOOL AddMenuItem(CTranslator* translator);
	
	virtual CTranslatorMenuItem *Find(UINT id);
	virtual CTranslatorMenuItem *Find(long ModuleID, long TransID);
	

	virtual BOOL Toggle(UINT id, BOOL on = FALSE); // On really means turn on no matter what
	virtual void SelectTranslators(const char *);
	virtual CString GetSelectedTranslators();
	virtual void EnableItems(BOOL enable = TRUE);

	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );

private:
	BOOL m_ShowCheck;
};


#ifdef OLDSTUFF
class CAttacher;
class CAttacherMenuItem;
//===================================================================
//
//===================================================================
class CAttacherMenuItem : public CUserMenuItem	
{
public:	
	CAttacherMenuItem(CAttacher *pAtt, const char *desc, UINT menuID);
	virtual ~CAttacherMenuItem(){};
	CAttacher *GetAttacher(){return m_Attacher;}
		
// Attributes
private:
	CAttacher  *m_Attacher;
};

//===================================================================
//
//===================================================================
class CAttacherMenu : public CUserMenu
{
// Implementation
public:
	virtual ~CAttacherMenu(){};
	virtual BOOL InitMenu();
	virtual void EnableItems(BOOL enable = TRUE);
};

class CSpecial;
class CSpecialMenuItem;

//===================================================================
class CSpecialMenuItem : public CUserMenuItem	
{
public:	
	CSpecialMenuItem(CSpecial *pAtt, const char *desc, UINT menuID);
	virtual ~CSpecialMenuItem(){};
	CSpecial *GetSpecial(){return m_Special;}
		
// Attributes
private:
	CSpecial  *m_Special;
};

//===================================================================
//
//===================================================================
class CSpecialMenu : public CAttacherMenu
{
public:
	virtual ~CSpecialMenu(){};
	virtual BOOL InitMenu(CUserMenu& shadowMenu);
	virtual BOOL RemoveItems(CUserMenu& shadowMenu);
};


#endif