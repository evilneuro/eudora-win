// ProfileNag.h -- Simple, modeless, unscheduled nag window

#if !defined( _SIMPLE_NAG_H_ )
#define _SIMPLE_NAG_H_

void DoProfileNag( CWnd* pParent, int level, LPCTSTR pText );
bool DoDeadbeatNag( CWnd* pParent = NULL, bool bUpgradeNag = false );

#endif   // _SIMPLE_NAG_H_
