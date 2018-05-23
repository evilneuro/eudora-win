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
	TBarSendButton():m_bBPWarning(false) {};
	~TBarSendButton() {};

	virtual void AdjustSize();

	bool IsBPWarning(){return m_bBPWarning;}
	void SetBPWarning(bool bBPWarning){m_bBPWarning = bBPWarning;}

protected:
	virtual void DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						  int& nWidth, int& nHeight, int nImgWidth = -1);
	virtual int	OnToolHitTest(CPoint point, TOOLINFO * pTI) const;

private:
	bool m_bBPWarning;
};

#endif // __TBARSENDBUTTON_H__


