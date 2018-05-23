// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG:
//
//    AAB	9/3/95	Started
//    MSW       9/30/95  Code Review
//
//

#ifndef __SECRAND_H__
#define __SECRAND_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


#define SECRANDOM_MIN	0
#define SECRANDOM_MAX	32767

class SECRandom : public CObject
{
    DECLARE_SERIAL(SECRandom)
        
// Construction                                 
public:
  SECRandom(unsigned int nSeed=0);
    
// Attributes
public:
    BOOL SetLBound(unsigned int);
    BOOL SetUBound(unsigned int);
    BOOL SetBounds(unsigned int, unsigned int);
    BOOL AddWeight(unsigned int, unsigned int);
    BOOL InitWeights(unsigned int);
    // retrieve current seed value
    unsigned int GetSeed();
    unsigned int GetLBound();
    unsigned int GetUBound();
    
    
// Operations
public:
    unsigned int GetRandom();
    unsigned int GetRandomWeighted();
    unsigned int GetRange();	// Retrieves random value between current min/max
    unsigned int GetRange(unsigned int nMin, unsigned int nMax); // reassigns min/max
    void SetMultiplier();

// Implementation
protected:
	unsigned int m_nMin;	// Min value
	unsigned int m_nMax;	// Max val
	unsigned int m_nSeed;	// Seed used for Knuth ops
	double m_dMultiplier;	// Bounds multiplier
	unsigned int m_nWeights;	// Number of weight values
	unsigned int *m_pWeights;	// pointer to weight array itself

public:
    virtual ~SECRandom();
#ifdef _DEBUG
	void AssertValid() const;
#endif
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECRAND_H__

