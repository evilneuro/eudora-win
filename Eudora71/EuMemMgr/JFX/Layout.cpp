#include "stdafx.h"
#include "JFX.h"

// This macro evaluates to the number of elements in an array.
#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))
// This macro returns TRUE if a number is between two others
#define chINRANGE(low, Num, High) (((low) <= (Num)) && ((Num) <= (High)))
inline void chMB(LPCTSTR s) {
   TCHAR szTMP[256];
   GetModuleFileName(NULL, szTMP, chDIMOF(szTMP));
   HWND hwnd = GetActiveWindow();
   MessageBox(hwnd, s, szTMP, MB_OK);
   //MessageBoxA(hwnd, s, szTMP, MB_OK | ((hwnd == NULL ) ? MB_SERVICE_NOTIFICATION : 0));
}



//////////////////////// ID for Child List Terminator /////////////////////////


#define IDC_LASTCHILD 0x10001     // Special child id for last child in list.
// Guaranteed not to be used because you
// cannot have control ids above 0xffff.


/////////////////////////////////// Limits ////////////////////////////////////


#define NUMPARTS     7            // There are 7 parts: left, top, right,
// bottom, width, height and group
#define NUMMETRICS   6            // There are 6 metrics: left, top, right,
// bottom, width and height.
#define NUMSIDES     4            // There are 4 sides: left, top, right and
// bottom.


/////////////////////// Macros for Asserting Part Types ///////////////////////


#define ISPART(n)   chINRANGE(lpLEFT, (n), lpGROUP)
#define ISMETRIC(n) chINRANGE(lpLEFT, (n), lpHEIGHT)
#define ISSIDE(n)   chINRANGE(lpLEFT, (n), lpBOTTOM)


///////////////////////////////// Metric Flags ////////////////////////////////


typedef enum
{
    UNKNOWN,                       // Metric is unknown
    KNOWN                          // Metric is known
} MFLAG;


///////////////////////////////// Rule States /////////////////////////////////


enum
{
    UNAPPLIED,                     // Rule has not been applied
    APPLIED                        // Rule has been applied
};


//////////////////////// Definition of a Child Control ////////////////////////

// nonstandard extension user : nameless struct/union
#pragma warning(disable:4201)

typedef struct tagCHILD
{
    int idc;                       // Control id (lPARENT represents parent)
    union
    {                        // Child has either a list of metrics:
        int anMetric[NUMMETRICS];   //   left, top, right, bottom, width, height
        struct
        {                    // or a RECT, a width and a height:
            RECT rc;                 //   RECT (left, top, right, bottom)
            int nWidth;              //   width
            int nHeight;             //   height
        };                          // depending on which is more readable in
    };                             // a given context.
    BOOL fFixed;                   // TRUE if child doesn't need to be placed
    MFLAG afMetric[NUMMETRICS];    // Metric flags (KNOWN or UNKNOWN)
} CHILD;
#pragma warning(default:4201)

////////////////////////// Local Function Prototypes //////////////////////////


BOOL   chMapDialogRect(HWND hwndParent, PRECT prc);

int    Layout_GetOppositeSide (int nSide);
int    Layout_GetOtherUnknownMetric (int nUnknownMetric);
BOOL   Layout_MetricIsVertical (int nMetric);
void   Layout_SolveChild (CHILD* pChild);
CHILD* Layout_FindChild (CHILD* pChildList, int idcChild);
CHILD* Layout_CreateChildList (HWND hwndParent, int* pnChildren);
void   Layout_ConvertDlgUnits (HWND hwndParent, RULE* pRules, CHILD* pChildList);
void   Layout_MarkUnknowns (HWND hwndParent, RULE* pRules, CHILD* pChildList);
BOOL   Layout_CheckChild (CHILD* pChild);
BOOL   Layout_ApplyRule (HWND hwndParent, RULE* pRules, CHILD* pChildList, RULE* pRule);
BOOL   Layout_ApplyRules (HWND hwndParent, RULE* pRules, CHILD* pChildList);


///////////////////////////////////////////////////////////////////////////////


// A MapDialogRect function that works for non-dialog windows.
BOOL chMapDialogRect(HWND hwndParent, PRECT prc)
{

    HDC hdc;
    SIZE size;
    int cxChar, cyChar;
    HFONT hfont = NULL, hfontOriginal = NULL;

    // This is the set of characters that Windows uses to compute the average
    // character width.
    static TCHAR szChars[] =
    TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(prc);

    // If the window is a dialog, just use MapDialogRect.
    if ( GetClassWord(hwndParent, GCW_ATOM) == 32770 )
        return(MapDialogRect(hwndParent, prc));

    // Get a device context and select the window's font into it.
    hdc = GetDC(hwndParent);
    hfont = GetWindowFont(hwndParent);
    if ( hfont != NULL )
        hfontOriginal = SelectFont(hdc, hfont);

    // Unfortunately, we cannot use GetTextMetrics to get the average character
    // width because the TEXTMETRIC structure's tmAveCharWidth member is
    // incorrect for proportional fonts. So, instead we compute the average
    // character width ourselves using the same technique employed by Windows
    // itself:  We pass "a-zA-Z" to GetTextExtentPoint and average, rounding up.
    // (NOTE: We do not call GetTextExtentPoint32 because this function corrects
    // an error that Windows relies on)
    GetTextExtentPoint(hdc, szChars, chDIMOF(szChars), &size);
    cyChar = size.cy;
    cxChar = ((size.cx / (chDIMOF(szChars) / 2)) + 1) / 2;

    // Restore any original font and then release the device context.
    if ( hfont != NULL )
        SelectFont(hdc, hfontOriginal);
    ReleaseDC(hwndParent, hdc);

    // Map rectangle prc based on the font dimensions (cxChar by cyChar).
    SetRect(prc,
            prc->left  * cxChar / 4, prc->top    * cyChar / 8,
            prc->right * cxChar / 4, prc->bottom * cyChar / 8);
    return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


int Layout_GetOppositeSide (int nSide)
{

    int nOppositeSide = 0;

    // Check assumptions.
    ASSERT(ISSIDE(nSide));

    switch ( nSide )
    {
        case lpLEFT:
            nOppositeSide = lpRIGHT;
            break;

        case lpRIGHT:
            nOppositeSide = lpLEFT;
            break;

        case lpTOP:
            nOppositeSide = lpBOTTOM;
            break;

        case lpBOTTOM:
            nOppositeSide = lpTOP;
            break;

        default:    // Invalid side
            ASSERT ( !"Invalid side" ) ;
            break;
    }
    return(nOppositeSide);
}


///////////////////////////////////////////////////////////////////////////////


int Layout_GetOtherUnknownMetric (int nUnknownMetric)
{

    int nOtherUnknownMetric = 0;

    // Check assumptions.
    ASSERT(ISMETRIC(nUnknownMetric));

    switch ( nUnknownMetric )
    {
        case lpLEFT:
        case lpRIGHT:
            nOtherUnknownMetric = lpWIDTH;
            break;

        case lpTOP:
        case lpBOTTOM:
            nOtherUnknownMetric = lpHEIGHT;
            break;

        case lpWIDTH:
            nOtherUnknownMetric = lpRIGHT;
            break;

        case lpHEIGHT:
            nOtherUnknownMetric = lpBOTTOM;
            break;

        default:
            ASSERT ( !"Unknown side" ) ;
            break;
    }
    return(nOtherUnknownMetric);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Layout_MetricIsVertical (int nMetric)
{

    BOOL fMetricIsVertical = FALSE;

    // Check assumptions.
    ASSERT(ISMETRIC(nMetric));

    switch ( nMetric )
    {
        case lpLEFT:
        case lpRIGHT:
        case lpWIDTH:
            fMetricIsVertical = FALSE;
            break;

        case lpTOP:
        case lpBOTTOM:
        case lpHEIGHT:
            fMetricIsVertical = TRUE;
            break;

        default:
            ASSERT ( !"Unknown side" ) ;
            break;
    }
    return(fMetricIsVertical);
}


///////////////////////////////////////////////////////////////////////////////


void Layout_SolveChild (CHILD* pChild)
{

    int i;

    // Check assumptions.
    ASSERT(pChild);

    // Loop through all six metrics of a child, computing values for unknown
    // metrics from values of known metrics (if any).
    for ( i = 0; i < NUMMETRICS; i++ )
    {

        // If this metric of the child is unknown, see if it can be computed in
        // terms of other metrics which we do know.
        if ( pChild->afMetric[i] == UNKNOWN )
        {

            // Compute left/top as right/bottom - width/height
            if ( i < 2 )
            {
                if ( (pChild->afMetric[i + 2] == KNOWN) &&
                     (pChild->afMetric[i + 4] == KNOWN) )
                {
                    pChild->anMetric[i] = pChild->anMetric[i + 2] -
                                          pChild->anMetric[i + 4];
                    pChild->afMetric[i] = KNOWN;
                }
            }
            // Compute right/bottom as left/top + width/height
            else if ( i < 4 )
            {
                if ( (pChild->afMetric[i - 2] == KNOWN) &&
                     (pChild->afMetric[i + 2] == KNOWN) )
                {
                    pChild->anMetric[i] = pChild->anMetric[i - 2] +
                                          pChild->anMetric[i + 2];
                    pChild->afMetric[i] = KNOWN;
                }
            }
            // Compute width/height as right/bottom - left/top
            else
            {
                if ( (pChild->afMetric[i - 2] == KNOWN) &&
                     (pChild->afMetric[i - 4] == KNOWN) )
                {
                    pChild->anMetric[i] = pChild->anMetric[i - 2] -
                                          pChild->anMetric[i - 4];
                    pChild->afMetric[i] = KNOWN;
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////


CHILD* Layout_FindChild (CHILD* pChildList, int idcChild)
{

    CHILD *pChild;

    // Check assumptions.
    ASSERT(pChildList);
    ASSERT(chINRANGE(0, idcChild, IDC_LASTCHILD));

    // Traverse the child list looking for an id
    for ( pChild = pChildList; pChild->idc != IDC_LASTCHILD; pChild++ )
    {

        // If we find idcChild, we solve the child for unknowns and return it
        if ( pChild->idc == idcChild )
        {
            Layout_SolveChild(pChild);
            return(pChild);
        }
    }
    ASSERT ( !"Child not found in child list" ) ;
    return(NULL);
}


///////////////////////////////////////////////////////////////////////////////


CHILD* Layout_CreateChildList (HWND hwndParent, int* pnChildren)
{

    int i;
    HWND hwnd, hwndFirst;
    CHILD* pChild, *pChildList;

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(pnChildren);

    // Count the number of child windows in hwndParent.
    hwndFirst = hwnd = GetFirstChild(hwndParent);
    for ( *pnChildren = 0; IsWindow(hwnd); hwnd = GetNextSibling(hwnd) )
        (*pnChildren)++;
    if ( *pnChildren == 0 )
        return(NULL);

    // Allocate memory for the CHILD list. This list will have an entry for
    // each child of the dialog, plus a CHILD structure for the parent window
    // (lPARENT) and one which acts as a list terminator (IDC_LASTCHILD).
    pChildList = (CHILD*) malloc((*pnChildren + 2) * sizeof(CHILD));
    ASSERT(pChildList);
    if ( !pChildList )
        return(NULL);
    pChild = pChildList;

    // Add the special-case parent 'CHILD' structure
    pChild->idc = lPARENT;
    GetClientRect(hwndParent, &pChild->rc);
    for ( i = 0; i < NUMSIDES; i++ )
        pChild->afMetric[i] = KNOWN;
    pChild->afMetric[lpWIDTH] = pChild->afMetric[lpHEIGHT] = UNKNOWN;
    Layout_SolveChild(pChild);
    pChild++;

    // Add all the real children of the dialog to the list
    hwnd = hwndFirst;
    for ( ; IsWindow(hwnd); hwnd = GetNextSibling(hwnd) )
    {

        // Get child's id and bounding rectangle in client coordinates
        pChild->idc = GetWindowID(hwnd);
        GetWindowRect(hwnd, &pChild->rc);
        MapWindowRect(HWND_DESKTOP, hwndParent, &pChild->rc);
        for ( i = 0; i < NUMSIDES; i++ )
            pChild->afMetric[i] = KNOWN;

        // Solve for the width and height.
        pChild->afMetric[lpWIDTH] = pChild->afMetric[lpHEIGHT] = UNKNOWN;
        Layout_SolveChild(pChild);

        // All children are fixed, initially.
        pChild->fFixed = TRUE;
        pChild++;
    }

    // Terminate and return the list.
    pChild->idc = IDC_LASTCHILD;
    return(pChildList);
}


///////////////////////////////////////////////////////////////////////////////


void Layout_ConvertDlgUnits (HWND hwndParent, RULE* pRules, CHILD* pChildList)
{

    RECT rc = { 0, 0, 0, 0};
    BOOL fVertical = FALSE;
    RULE* pRule;

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(pRules);
    ASSERT(pChildList);
    pChildList = pChildList ;

    // Traverse the rules list
    for ( pRule = pRules; pRule->Action != lEND; pRule++ )
    {

        // Simultaneously map the rule's offset value, vertically and
        // horizontally, from dialog units to pixels.
        rc.right = rc.bottom = pRule->nOffset;
        chMapDialogRect(hwndParent, &rc);

        // Determine if the current rule affects horizontal or vertical
        // coordinates. We need to know this because dialog unit space is not
        // isometric (horizontal and vertical dialog units are not equivalent).
        switch ( pRule->Action )
        {

            case lVCENTER:
                fVertical = TRUE;
                break;

            case lHCENTER:
                fVertical = FALSE;
                break;

            case lMOVE:
            case lSTRETCH:
                fVertical = Layout_MetricIsVertical(pRule->ActOn.nMetric);
                break;

            default:
                ASSERT ( !"Unknown operation!" ) ;
                break;
        }

        // Take the correct mapped value based on the rule being applied
        pRule->nPixelOffset = fVertical ? rc.bottom : rc.right;
    }
}


///////////////////////////////////////////////////////////////////////////////


void Layout_MarkUnknowns (HWND hwndParent, RULE* pRules, CHILD* pChildList)
{

    RULE* pRule;
    CHILD* pChildActOn;
    HWND hwnd;
    int nOtherUnknown, nOppositeSide, idc, idcFirst, idcLast;

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(pRules);
    ASSERT(pChildList);

    // Traverse the rule list, marking unknowns in the child list.
    for ( pRule = pRules; pRule->Action != lEND; pRule++ )
    {

        // Set metric flags based on the rule's proposed action
        switch ( pRule->Action )
        {

            case lSTRETCH:           // Metric should be stretched

                // Find the child to be acted upon
                pChildActOn = Layout_FindChild(pChildList, pRule->ActOn.idc);
                ASSERT(pChildActOn);

                // Since the child is going to be acted upon, it is no longer fixed.
                pChildActOn->fFixed = FALSE;

                // The part being acted on must be a metric.
                ASSERT(ISMETRIC(pRule->ActOn.nMetric));

                // The metric being stretched must be unknown.
                pChildActOn->afMetric[pRule->ActOn.nMetric] = UNKNOWN;

                // If the left/top or right/bottom is unknown, so is the
                // width/height. If the width/height is unknown, then the
                // right/bottom is also unknown.
                nOtherUnknown = Layout_GetOtherUnknownMetric(pRule->ActOn.nMetric);
                pChildActOn->afMetric[nOtherUnknown] = UNKNOWN;
                break;

            case lMOVE:              // Control should be moved

                // Find the child to be acted upon
                pChildActOn = Layout_FindChild(pChildList, pRule->ActOn.idc);
                ASSERT(pChildActOn);

                // Since the child is going to be acted upon, it is no longer fixed.
                pChildActOn->fFixed = FALSE;

                // The part being acted upon must be a side.
                ASSERT(ISSIDE(pRule->ActOn.nSide));

                // The side being moved is unknown.
                pChildActOn->afMetric[pRule->ActOn.nSide] = UNKNOWN;

                // So is the opposite side. But, the width/height remains known.
                // (Actually, this is the primary reason for having six metrics).
                nOppositeSide = Layout_GetOppositeSide(pRule->ActOn.nSide);
                pChildActOn->afMetric[nOppositeSide] = UNKNOWN;
                break;

            case lVCENTER:           // Vertically center control/group
            case lHCENTER:           // Horizontally center control/group

                // We must be centering a group of one or more controls.
                ASSERT(pRule->ActOn.nPart == lpGROUP);

                // Go through the group of one or more controls
                idcFirst = pRule->ActOn.idcFirst;
                idcLast = pRule->ActOn.idcLast;
                ASSERT(idcFirst <= idcLast);
                hwnd = GetFirstChild(hwndParent);
                for ( ; IsWindow(hwnd); hwnd = GetNextSibling(hwnd) )
                {
                    idc = GetWindowID(hwnd);
                    if ( chINRANGE(idcFirst, idc, idcLast) )
                    {

                        // Find the child to be acted upon and set the appropriate
                        // sides to unknown. Width is still known.
                        pChildActOn = Layout_FindChild(pChildList, idc);
                        if ( pRule->Action == lHCENTER )
                        {
                            pChildActOn->afMetric[lpLEFT] = UNKNOWN;
                            pChildActOn->afMetric[lpRIGHT] = UNKNOWN;
                        }
                        else
                        {
                            pChildActOn->afMetric[lpTOP] = UNKNOWN;
                            pChildActOn->afMetric[lpBOTTOM] = UNKNOWN;
                        }

                        // Child acted upon is no longer fixed.
                        pChildActOn->fFixed = FALSE;
                    }
                }
                break;

            default:
                ASSERT ( !"Unknown operation" ) ;
                break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////


BOOL Layout_CheckChild (CHILD* pChild)
{

    static TCHAR* pszMetric[] = {
        TEXT("left"),   TEXT("top"),   TEXT("right"),
        TEXT("bottom"), TEXT("width"), TEXT("height")
    };

    int i;
    BOOL fOK = TRUE;
    TCHAR sz[80];

    // Check assumptions.
    ASSERT(pChild);

    // Any unknown metric indicates a problem with the rules, so we 'assert'.
    for ( i = 0; i < NUMMETRICS; i++ )
    {
        if ( pChild->afMetric[i] == UNKNOWN )
        {
            wsprintf(sz, TEXT("Layout couldn't find %s of id=%d"),
                     pszMetric[i], pChild->idc);
            chMB(sz);
            fOK = FALSE;
        }
    }
    return(fOK);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Layout_ApplyRule (HWND hwndParent, RULE* pRules,
                       CHILD* pChildList, RULE* pRule)
{

    CHILD* pChildRelTo, *pChildActOn, ChildRelTo;
    CHILD* pChild, *pChildListNew, *pSrc, *pDest;
    int nRules, nMetric, nChildren;
    int idcFirst, idcLast, nOffset, nCentered;
    RECT rcBounds;
    HWND hwnd, hwndFirst;
    RULE *pr, *prn, *prNew;

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(pRules);
    ASSERT(pChildList);
    ASSERT(pRule);

    // Find the child and part(s) that we are going to act relative to
    pChildRelTo = Layout_FindChild(pChildList, pRule->RelTo.idc);
    ASSERT(pChildRelTo);

    switch ( pRule->RelTo.nPart )
    {

        case lpLEFT:
        case lpTOP:
        case lpRIGHT:
        case lpBOTTOM:
        case lpWIDTH:
        case lpHEIGHT:

            // We can't apply a rule relative to a metric that is unknown.
            if ( pChildRelTo->afMetric[pRule->RelTo.nMetric] == UNKNOWN )
                return(FALSE);
            break;

        case lpGROUP:

            // We can't apply a rule relative to a control unless we know its
            // left/top and right/bottom sides (for centering).
            ASSERT(pRule->RelTo.idcFirst == pRule->RelTo.idcLast);
            ASSERT((pRule->Action == lHCENTER) || (pRule->Action == lVCENTER));
            if ( pRule->Action == lHCENTER )
            {
                if ( (pChildRelTo->afMetric[lpLEFT] == UNKNOWN) ||
                     (pChildRelTo->afMetric[lpRIGHT] == UNKNOWN) )
                    return(FALSE);
            }
            else
            {
                if ( (pChildRelTo->afMetric[lpTOP] == UNKNOWN) ||
                     (pChildRelTo->afMetric[lpBOTTOM] == UNKNOWN) )
                    return(FALSE);
            }
            break;
    }

    // Make a local copy of the child we are relative to. We need to do this
    // because we may need to apply a percentage to the width/height metrics
    // and we don't want to modify the actual child list.
    ChildRelTo = *pChildRelTo;

    // Use percentage to modify the width/height of the child we are relative to
    if ( (pRule->RelTo.nMetric == lpWIDTH) || (pRule->RelTo.nMetric == lpHEIGHT) )
    {
        ChildRelTo.anMetric[pRule->RelTo.nMetric] *= pRule->RelTo.nPercent;
        ChildRelTo.anMetric[pRule->RelTo.nMetric] /= 100;
        Layout_SolveChild(&ChildRelTo);
    }

    // Apply our rule based on the action field
    switch ( pRule->Action )
    {

        case lSTRETCH:              // Metric should be stretched

            // The part being acted on must be a metric. If it is a width/height
            // metric, it must be 100% of the width/height.
            ASSERT(ISMETRIC(pRule->ActOn.nMetric));
            ASSERT(ISSIDE(pRule->ActOn.nSide) ||
                     (pRule->ActOn.nPercent == 100));

            // The part being acted relative to must be a metric.
            ASSERT(ISMETRIC(pRule->RelTo.nMetric));

            // Find the child being acted on and stretch the specified metric.
            pChildActOn = Layout_FindChild(pChildList, pRule->ActOn.idc);
            ASSERT(pChildActOn);
            pChildActOn->anMetric[pRule->ActOn.nMetric] =
            ChildRelTo.anMetric[pRule->RelTo.nMetric] + pRule->nPixelOffset;
            pChildActOn->afMetric[pRule->ActOn.nMetric] = KNOWN;
            Layout_SolveChild(pChildActOn);
            pRule->fState = APPLIED;
            return(TRUE);

        case lMOVE:                 // Whole control should be moved

            // The part being moved must be a side.
            ASSERT(ISSIDE(pRule->ActOn.nSide));

            // The part that is being acted relative to must be a metric.
            ASSERT(ISMETRIC(pRule->RelTo.nMetric));

            // Find the child being acted on and move the specified side.
            pChildActOn = Layout_FindChild(pChildList, pRule->ActOn.idc);
            ASSERT(pChildActOn);
            pChildActOn->anMetric[pRule->ActOn.nSide] =
            ChildRelTo.anMetric[pRule->RelTo.nMetric] + pRule->nPixelOffset;
            pChildActOn->afMetric[pRule->ActOn.nSide] = KNOWN;
            Layout_SolveChild(pChildActOn);
            pRule->fState = APPLIED;
            return(TRUE);

        case lVCENTER:              // Vertically center a control/group
        case lHCENTER:              // Horizontally center a control/group

            // We can only center a group of one or more controls relative to
            // another control (a single control is a 'group' of one control).
            ASSERT(pRule->ActOn.nPart == lpGROUP);
            ASSERT(pRule->RelTo.nPart == lpGROUP);
            ASSERT(pRule->RelTo.idcFirst == pRule->RelTo.idcLast);

            // First id in group must be less than or equal to the last id
            idcFirst = pRule->ActOn.idcFirst;
            idcLast = pRule->ActOn.idcLast;
            ASSERT(idcFirst <= idcLast);

            // Ensure that the width/height is known for each control in the
            // group before proceeding with any centering.
            hwndFirst = GetFirstChild(hwndParent);
            for ( hwnd = hwndFirst; IsWindow(hwnd); hwnd = GetNextSibling(hwnd) )
            {
                int idc = GetWindowID(hwnd);
                if ( chINRANGE(idcFirst, idc, idcLast) )
                {
                    pChildActOn = Layout_FindChild(pChildList, idc);
                    if ( pRule->Action == lHCENTER )
                    {
                        if ( pChildActOn->afMetric[lpWIDTH] == UNKNOWN )
                            return(FALSE);
                    }
                    else
                    {
                        if ( pChildActOn->afMetric[lpHEIGHT] == UNKNOWN )
                            return(FALSE);
                    }
                }
            }

            // Create a new list of rules which contains the subset of rules
            // which act on controls in the centered group.
            for ( nRules = 0, pr = pRules; pr->Action != lEND; pr++ )
                nRules++;
            nRules++;
            prNew = (RULE*) _alloca(nRules * sizeof(RULE));
            prn = prNew;
            for ( pr = pRules; pr->Action != lEND; pr++ )
            {
                if ( chINRANGE(idcFirst, pr->ActOn.idc, idcLast) )
                {
                    if ( pRule->Action == lHCENTER )
                    {
                        if ( (pr->ActOn.nPart == lpLEFT) ||
                             (pr->ActOn.nPart == lpRIGHT) )
                        {
                            *prn++ = *pr;
                        }
                    }
                    else
                    {
                        if ( (pr->ActOn.nPart == lpTOP) ||
                             (pr->ActOn.nPart == lpBOTTOM) )
                        {
                            *prn++ = *pr;
                        }
                    }
                }
            }
            prn->Action = lEND;

            // Make a local copy of the child list and set everything to KNOWN.
            nChildren = 0;
            for ( pChild = pChildList; pChild->idc != IDC_LASTCHILD; pChild++ )
                nChildren++;
            nChildren++;
            pChildListNew = (CHILD*) _alloca(nChildren * sizeof(CHILD));
            MoveMemory(pChildListNew, pChildList, nChildren * sizeof(CHILD));
            for ( pChild = pChildListNew; pChild->idc != IDC_LASTCHILD; pChild++ )
                for ( nMetric = 0; nMetric < NUMMETRICS; nMetric++ )
                    pChild->afMetric[nMetric] = KNOWN;

            // Solve for the children being centered as a sub-problem.
            if ( !Layout_ApplyRules (hwndParent, prNew, pChildListNew) )
            {
                ASSERT ( !"Unable to apply rules to centered children" ) ;
                return(FALSE);
            }

            // Compute the bounding rectangle of the group
            SetRectEmpty(&rcBounds);
            hwndFirst = GetFirstChild(hwndParent);
            for ( hwnd = hwndFirst; IsWindow(hwnd); hwnd = GetNextSibling(hwnd) )
            {
                int idc = GetWindowID(hwnd);
                if ( chINRANGE(idcFirst, idc, idcLast) )
                {
                    pChildActOn = Layout_FindChild(pChildListNew, idc);
                    UnionRect(&rcBounds, &rcBounds, &pChildActOn->rc);
                }
            }

            // Find the offset required to center the group's bounding rectangle
            // against the control we are relative to.
            if ( pRule->Action == lHCENTER )
            {
                nCentered = ChildRelTo.anMetric[lpLEFT] +
                            ((ChildRelTo.anMetric[lpWIDTH] -
                              (rcBounds.right - rcBounds.left)) / 2);
                nOffset = nCentered - rcBounds.left;
            }
            else
            {
                nCentered = ChildRelTo.anMetric[lpTOP] +
                            ((ChildRelTo.anMetric[lpHEIGHT] -
                              (rcBounds.bottom - rcBounds.top)) / 2);
                nOffset = nCentered - rcBounds.top;
            }

            // Add in any additional offset from the rule.
            nOffset += pRule->nPixelOffset;

            // Go through the new child list, moving each control.
            ASSERT(pRule->ActOn.idcFirst <= pRule->ActOn.idcLast);
            for ( hwnd = hwndFirst; IsWindow(hwnd); hwnd = GetNextSibling(hwnd) )
            {
                int idc = GetWindowID(hwnd);
                if ( chINRANGE(idcFirst, idc, idcLast) )
                {
                    pChildActOn = Layout_FindChild(pChildListNew, idc);
                    if ( pRule->Action == lHCENTER )
                    {
                        pChildActOn->anMetric[lpLEFT]  += nOffset;
                        pChildActOn->anMetric[lpRIGHT] += nOffset;
                    }
                    else
                    {
                        pChildActOn->anMetric[lpTOP]    += nOffset;
                        pChildActOn->anMetric[lpBOTTOM] += nOffset;
                    }
                }
            }

            // Now modify the real child list based on pChildListNew.
            for ( pSrc = pChildListNew, pDest = pChildList;
                pSrc->idc != IDC_LASTCHILD; pSrc++, pDest++ )
            {

                if ( chINRANGE(idcFirst, pSrc->idc, idcLast) )
                {
                    if ( pRule->Action == lHCENTER )
                    {
                        pDest->anMetric[lpLEFT]  = pSrc->anMetric[lpLEFT];
                        pDest->anMetric[lpRIGHT] = pSrc->anMetric[lpRIGHT];
                        pDest->afMetric[lpLEFT]  = KNOWN;
                        pDest->afMetric[lpRIGHT] = KNOWN;
                    }
                    else
                    {
                        pDest->anMetric[lpTOP]    = pSrc->anMetric[lpTOP];
                        pDest->anMetric[lpBOTTOM] = pSrc->anMetric[lpBOTTOM];
                        pDest->afMetric[lpTOP]    = KNOWN;
                        pDest->afMetric[lpBOTTOM] = KNOWN;
                    }
                }
            }

            pRule->fState = APPLIED;
            return(TRUE);

        default:
            ASSERT ( FALSE ) ;
            return(FALSE);
    }
}


///////////////////////////////////////////////////////////////////////////////


BOOL Layout_ApplyRules (HWND hwndParent, RULE* pRules, CHILD* pChildList)
{

    RULE* pRule;
    BOOL fAppliedAtLeastOneRule, fOK = TRUE;

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(pRules);
    ASSERT(pChildList);

    // Based on the list of rules, mark all unknown child metrics as UNKNOWN.
    Layout_MarkUnknowns(hwndParent, pRules, pChildList);

    // Traverse the rule list, converting offsets from dialog units to pixels.
    Layout_ConvertDlgUnits(hwndParent, pRules, pChildList);

    // Mark all the rules as unapplied before attempting to apply them.
    for ( pRule = pRules; pRule->Action != lEND; pRule++ )
        pRule->fState = UNAPPLIED;

    // Loop through the rule list for as long as we are able to apply at least
    // one rule (if we make a pass through the entire list, and we are unable
    // to apply any rule, we are finished).
    do
    {
        fAppliedAtLeastOneRule = FALSE;
        for ( pRule = pRules; pRule->Action != lEND; pRule++ )
        {
            if ( pRule->fState != APPLIED )
            {
                if ( Layout_ApplyRule(hwndParent, pRules, pChildList, pRule) )
                    fAppliedAtLeastOneRule = TRUE;
            }
        }
    } while ( fAppliedAtLeastOneRule );

    // Verify that all rules have been successfully applied.
    for ( pRule = pRules; pRule->Action != lEND; pRule++ )
    {
        ASSERT(pRule->fState == APPLIED);
        if ( pRule->fState != APPLIED )
            fOK = FALSE;
    }
    return(fOK);
}

///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI Layout_ComputeLayout(HWND hwndParent, RULE* pRules)
{

    HDWP hdwp;
    BOOL fOK = TRUE;
    CHILD* pChild, *pChildList;
    int nChildren;

    // Check assumptions.
    ASSERT(IsWindow(hwndParent));
    ASSERT(pRules);

    // Don't do anything to a minimized window.
    if ( IsIconic(hwndParent) )
        return(TRUE);

    // Enumerate all child windows of the dialog, allocating a CHILD structure
    // for each child, with all six metric flags set to KNOWN. To simplify
    // coding, we also add a special CHILD structure for the parent window with
    // the id lPARENT (defined in layout.h). If there are no children, or
    // memory cannot be allocated for the child list, we do nothing.
    pChildList = Layout_CreateChildList(hwndParent, &nChildren);
    if ( pChildList == NULL )
        return(FALSE);
    if ( nChildren == 0 )
        return(TRUE);

    // Apply the rules from the rule list to solve for the locations of all the
    // child controls.
    if ( !Layout_ApplyRules(hwndParent, pRules, pChildList) )
    {
        ASSERT ( !"Unable to apply rules" ) ;
        return(FALSE);
    }

    // Simultaneously relocate all the children using DeferWindowPos.
    hdwp = BeginDeferWindowPos(0);
    ASSERT(hdwp);

    // Move each child in the CHILD list.  We enumerate the child list starting
    // at pChildList + 1, because the first CHILD is lPARENT.
    for ( pChild = pChildList + 1; pChild->idc != IDC_LASTCHILD; pChild++ )
    {

        // Check child for any still-unsolved metrics. You may want to remove or
        // #ifdef out this check once your rules are known to be working.
        if ( !Layout_CheckChild(pChild) )
            fOK = FALSE;

        // Add child to DeferWindowPos list if it is not fixed.
        if ( !pChild->fFixed )
        {
            HWND hwndChild = GetDlgItem(hwndParent, pChild->idc);
            ASSERT(pChild->anMetric[lpWIDTH] >= 0);
            ASSERT(pChild->anMetric[lpHEIGHT] >= 0);
            hdwp = DeferWindowPos(hdwp, hwndChild, NULL,
                                  pChild->anMetric[lpLEFT],  pChild->anMetric[lpTOP],
                                  pChild->anMetric[lpWIDTH], pChild->anMetric[lpHEIGHT],
                                  SWP_NOZORDER);
            ASSERT(hdwp);

            // Invalidation is necessary here because some controls (edit
            // controls in particular) don't repaint correctly under Windows NT
            // when they are moved with DeferWindowPos.
            InvalidateRect(hwndChild, NULL, TRUE);
        }
    }

    // It is this function call which actually moves all the windows.
    EndDeferWindowPos(hdwp);

    // Free the allocated list of CHILD structures
    free(pChildList);
    return(fOK);
}


///////////////////////////////////////////////////////////////////////////////


