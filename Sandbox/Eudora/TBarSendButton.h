#ifndef __TBARSENDBUTTON_H__
#define __TBARSENDBUTTON_H__
////////////////////////////////////////////////////////////////////////
// Filename:  TBarSendButton.h
//
// Handles special sizing and drawing for Send/Queue button on comp
// message toolbar.
////////////////////////////////////////////////////////////////////////


class TBarSendButton : public SECStdBtn
{
	DECLARE_BUTTON(TBarSendButton)
public:
	TBarSendButton() {};
	~TBarSendButton() {};

	virtual void AdjustSize();

protected:
	virtual void DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						  int& nWidth, int& nHeight, int nImgWidth = -1);
};

#endif // __TBARSENDBUTTON_H__


