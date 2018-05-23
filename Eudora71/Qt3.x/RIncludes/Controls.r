/*
 	File:		Controls.r
 
 	Contains:	Control Manager interfaces
 
 	Version:	Technology:	Mac OS 8.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __CONTROLS_R__
#define __CONTROLS_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define kControlSupportsNewMessages 	' ok '				/*  CDEF should return as result of kControlMsgTestNewMsgSupport */
#define kControlFocusNoPart 			0					/*  tells control to clear its focus */
#define kControlFocusNextPart 			(-1)				/*  tells control to focus on the next part */
#define kControlFocusPrevPart 			(-2)				/*  tells control to focus on the previous part */

#define kControlKeyFilterBlockKey 		0
#define kControlKeyFilterPassKey 		1

															/*  Meta-font numbering - see not above  */
#define kControlFontBigSystemFont 		(-1)				/*  force to big system font */
#define kControlFontSmallSystemFont 	(-2)				/*  force to small system font */
#define kControlFontSmallBoldSystemFont  (-3)				/*  force to small bold system font */

#define kControlUseFontMask 			0x0001
#define kControlUseFaceMask 			0x0002
#define kControlUseSizeMask 			0x0004
#define kControlUseForeColorMask 		0x0008
#define kControlUseBackColorMask 		0x0010
#define kControlUseModeMask 			0x0020
#define kControlUseJustMask 			0x0040
#define kControlUseAllMask 				0x00FF
#define kControlAddFontSizeMask 		0x0100

#define kControlFontStyleTag 			'font'
#define kControlKeyFilterTag 			'fltr'

															/*  Control feature bits - returned by GetControlFeatures  */
#define kControlSupportsGhosting 		0x01
#define kControlSupportsEmbedding 		0x02
#define kControlSupportsFocus 			0x04
#define kControlWantsIdle 				0x08
#define kControlWantsActivate 			0x10
#define kControlHandlesTracking 		0x20
#define kControlSupportsDataAccess 		0x40
#define kControlHasSpecialBackground 	0x80
#define kControlGetsFocusOnClick 		0x0100
#define kControlSupportsCalcBestRect 	0x0200
#define kControlSupportsLiveFeedback 	0x0400

#define kControlHasRadioBehavior 		0x0800
#define kControlMsgDrawGhost 			13
#define kControlMsgCalcBestRect 		14					/*  Calculate best fitting rectangle for control */
#define kControlMsgHandleTracking 		15
#define kControlMsgFocus 				16					/*  param indicates action. */
#define kControlMsgKeyDown 				17
#define kControlMsgIdle 				18
#define kControlMsgGetFeatures 			19
#define kControlMsgSetData 				20
#define kControlMsgGetData 				21
#define kControlMsgActivate 			22
#define kControlMsgSetUpBackground 		23
#define kControlMsgCalcValueFromPos 	26
#define kControlMsgTestNewMsgSupport 	27					/*  See if this control supports new messaging */

#define kControlMsgSubValueChanged 		25
#define kControlMsgSubControlAdded 		28
#define kControlMsgSubControlRemoved 	29

															/*  Bevel Button Proc IDs  */
#define kControlBevelButtonSmallBevelProc  32
#define kControlBevelButtonNormalBevelProc  33
#define kControlBevelButtonLargeBevelProc  34

															/*  Bevel button graphic alignment values  */
#define kControlBevelButtonAlignSysDirection  (-1)			/*  only left or right */
#define kControlBevelButtonAlignCenter 	0
#define kControlBevelButtonAlignLeft 	1
#define kControlBevelButtonAlignRight 	2
#define kControlBevelButtonAlignTop 	3
#define kControlBevelButtonAlignBottom 	4
#define kControlBevelButtonAlignTopLeft  5
#define kControlBevelButtonAlignBottomLeft  6
#define kControlBevelButtonAlignTopRight  7
#define kControlBevelButtonAlignBottomRight  8

															/*  Bevel button text alignment values  */
#define kControlBevelButtonAlignTextSysDirection  0
#define kControlBevelButtonAlignTextCenter  1
#define kControlBevelButtonAlignTextFlushRight  (-1)
#define kControlBevelButtonAlignTextFlushLeft  (-2)

															/*  Bevel button text placement values  */
#define kControlBevelButtonPlaceSysDirection  (-1)			/*  if graphic on right, then on left */
#define kControlBevelButtonPlaceNormally  0
#define kControlBevelButtonPlaceToRightOfGraphic  1
#define kControlBevelButtonPlaceToLeftOfGraphic  2
#define kControlBevelButtonPlaceBelowGraphic  3
#define kControlBevelButtonPlaceAboveGraphic  4

#define kControlBevelButtonSmallBevelVariant  0
#define kControlBevelButtonNormalBevelVariant  0x01
#define kControlBevelButtonLargeBevelVariant  0x02
#define kControlBevelButtonMenuOnRight 	0x04

#define kControlBehaviorPushbutton 		0
#define kControlBehaviorToggles 		0x0100
#define kControlBehaviorSticky 			0x0200
#define kControlBehaviorMultiValueMenu 	0x4000				/*  only makes sense when a menu is attached. */
#define kControlBehaviorOffsetContents 	0x8000

#define kControlBehaviorCommandMenu 	0x2000				/*  menu holds commands, not choices. Overrides multi-value bit. */
#define kControlContentTextOnly 		0
#define kControlContentIconSuiteRes 	1
#define kControlContentCIconRes 		2
#define kControlContentPictRes 			3
#define kControlContentIconSuiteHandle 	129
#define kControlContentCIconHandle 		130
#define kControlContentPictHandle 		131
#define kControlContentIconRef 			132

#define kControlBevelButtonContentTag 	'cont'				/*  ButtonContentInfo */
#define kControlBevelButtonTransformTag  'tran'				/*  IconTransformType */
#define kControlBevelButtonTextAlignTag  'tali'				/*  ButtonTextAlignment */
#define kControlBevelButtonTextOffsetTag  'toff'			/*  SInt16 */
#define kControlBevelButtonGraphicAlignTag  'gali'			/*  ButtonGraphicAlignment */
#define kControlBevelButtonGraphicOffsetTag  'goff'			/*  Point */
#define kControlBevelButtonTextPlaceTag  'tplc'				/*  ButtonTextPlacement */
#define kControlBevelButtonMenuValueTag  'mval'				/*  SInt16 */
#define kControlBevelButtonMenuHandleTag  'mhnd'			/*  MenuHandle */
#define kControlBevelButtonCenterPopupGlyphTag  'pglc'		/*  Boolean: true = center, false = bottom right */

#define kControlBevelButtonLastMenuTag 	'lmnu'				/*  SInt16: menuID of last menu item selected from */
#define kControlBevelButtonMenuDelayTag  'mdly'				/*  SInt32: ticks to delay before menu appears */

															/*  Slider proc IDs  */
#define kControlSliderProc 				48
#define kControlSliderLiveFeedback 		0x01
#define kControlSliderHasTickMarks 		0x02
#define kControlSliderReverseDirection 	0x04
#define kControlSliderNonDirectional 	0x08

															/*  Triangle proc IDs  */
#define kControlTriangleProc 			64
#define kControlTriangleLeftFacingProc 	65
#define kControlTriangleAutoToggleProc 	66
#define kControlTriangleLeftFacingAutoToggleProc  67

															/*  Tagged data supported by disclosure triangles  */
#define kControlTriangleLastValueTag 	'last'				/*  SInt16 */
															/*  Progress Bar proc IDs  */
#define kControlProgressBarProc 		80
															/*  Tagged data supported by progress bars  */
#define kControlProgressBarIndeterminateTag  'inde'			/*  Boolean */
															/*  Little Arrows proc IDs  */
#define kControlLittleArrowsProc 		96
															/*  Chasing Arrows proc IDs  */
#define kControlChasingArrowsProc 		112
															/*  Tabs proc IDs  */
#define kControlTabLargeProc 			128					/*  Large tab size, north facing	 */
#define kControlTabSmallProc 			129					/*  Small tab size, north facing	 */
#define kControlTabLargeNorthProc 		128					/*  Large tab size, north facing	 */
#define kControlTabSmallNorthProc 		129					/*  Small tab size, north facing	 */
#define kControlTabLargeSouthProc 		130					/*  Large tab size, south facing	 */
#define kControlTabSmallSouthProc 		131					/*  Small tab size, south facing	 */
#define kControlTabLargeEastProc 		132					/*  Large tab size, east facing	 */
#define kControlTabSmallEastProc 		133					/*  Small tab size, east facing	 */
#define kControlTabLargeWestProc 		134					/*  Large tab size, west facing	 */
#define kControlTabSmallWestProc 		135					/*  Small tab size, west facing	 */

															/*  Tagged data supported by progress bars  */
#define kControlTabContentRectTag 		'rect'				/*  Rect */
#define kControlTabEnabledFlagTag 		'enab'				/*  Boolean */
#define kControlTabFontStyleTag 		'font'				/*  ControlFontStyleRec */

#define kControlTabInfoTag 				'tabi'				/*  ControlTabInfoRec */
#define kControlTabInfoVersionZero 		0
															/*  Visual separator proc IDs  */
#define kControlSeparatorLineProc 		144
															/*  Group Box proc IDs  */
#define kControlGroupBoxTextTitleProc 	160
#define kControlGroupBoxCheckBoxProc 	161
#define kControlGroupBoxPopupButtonProc  162
#define kControlGroupBoxSecondaryTextTitleProc  164
#define kControlGroupBoxSecondaryCheckBoxProc  165
#define kControlGroupBoxSecondaryPopupButtonProc  166

															/*  Tagged data supported by group box  */
#define kControlGroupBoxMenuHandleTag 	'mhan'				/*  MenuHandle (popup title only) */
#define kControlGroupBoxFontStyleTag 	'font'				/*  ControlFontStyleRec */

															/*  Image Well proc IDs  */
#define kControlImageWellProc 			176
															/*  Tagged data supported by image wells  */
#define kControlImageWellContentTag 	'cont'				/*  ButtonContentInfo */
#define kControlImageWellTransformTag 	'tran'				/*  IconTransformType */

															/*  Popup Arrow proc IDs  */
#define kControlPopupArrowEastProc 		192
#define kControlPopupArrowWestProc 		193
#define kControlPopupArrowNorthProc 	194
#define kControlPopupArrowSouthProc 	195
#define kControlPopupArrowSmallEastProc  196
#define kControlPopupArrowSmallWestProc  197
#define kControlPopupArrowSmallNorthProc  198
#define kControlPopupArrowSmallSouthProc  199

															/*  Placard proc IDs  */
#define kControlPlacardProc 			224
															/*  Clock proc IDs  */
#define kControlClockTimeProc 			240
#define kControlClockTimeSecondsProc 	241
#define kControlClockDateProc 			242
#define kControlClockMonthYearProc 		243

#define kControlClockNoFlags 			0
#define kControlClockIsDisplayOnly 		1
#define kControlClockIsLive 			2

															/*  Tagged data supported by clocks  */
#define kControlClockLongDateTag 		'date'				/*  LongDateRec */
#define kControlClockFontStyleTag 		'font'				/*  ControlFontStyleRec */

															/*  User Pane proc IDs  */
#define kControlUserPaneProc 			256
#define kControlUserItemDrawProcTag 	'uidp'				/*  UserItemUPP */
#define kControlUserPaneDrawProcTag 	'draw'				/*  ControlUserPaneDrawingUPP */
#define kControlUserPaneHitTestProcTag 	'hitt'				/*  ControlUserPaneHitTestUPP */
#define kControlUserPaneTrackingProcTag  'trak'				/*  ControlUserPaneTrackingUPP */
#define kControlUserPaneIdleProcTag 	'idle'				/*  ControlUserPaneIdleUPP */
#define kControlUserPaneKeyDownProcTag 	'keyd'				/*  ControlUserPaneKeyDownUPP */
#define kControlUserPaneActivateProcTag  'acti'				/*  ControlUserPaneActivateUPP */
#define kControlUserPaneFocusProcTag 	'foci'				/*  ControlUserPaneFocusUPP */
#define kControlUserPaneBackgroundProcTag  'back'			/*  ControlUserPaneBackgroundUPP */

															/*  Edit Text proc IDs  */
#define kControlEditTextProc 			272
#define kControlEditTextDialogProc 		273
#define kControlEditTextPasswordProc 	274
#define kControlEditTextDialogPasswordProc  275

															/*  Tagged data supported by edit text  */
#define kControlEditTextStyleTag 		'font'				/*  ControlFontStyleRec */
#define kControlEditTextTextTag 		'text'				/*  Buffer of chars - you supply the buffer */
#define kControlEditTextTEHandleTag 	'than'				/*  The TEHandle of the text edit record */
#define kControlEditTextKeyFilterTag 	'fltr'
#define kControlEditTextSelectionTag 	'sele'				/*  EditTextSelectionRec */
#define kControlEditTextPasswordTag 	'pass'				/*  The clear text password text */

#define kControlStaticTextProc 			288
#define kControlStaticTextStyleTag 		'font'				/*  ControlFontStyleRec */
#define kControlStaticTextTextTag 		'text'				/*  Copy of text */
#define kControlStaticTextTextHeightTag  'thei'				/*  SInt16 */

															/*  Picture control proc IDs  */
#define kControlPictureProc 			304
#define kControlPictureNoTrackProc 		305					/*  immediately returns kControlPicturePart */

#define kControlIconProc 				320
#define kControlIconNoTrackProc 		321					/*  immediately returns kControlIconPart */
#define kControlIconSuiteProc 			322
#define kControlIconSuiteNoTrackProc 	323					/*  immediately returns kControlIconPart */

#define kControlIconTransformTag 		'trfm'				/*  IconTransformType */
#define kControlIconAlignmentTag 		'algn'				/*  IconAlignmentType */

															/*  Window Header proc IDs  */
#define kControlWindowHeaderProc 		336					/*  normal header */
#define kControlWindowListViewHeaderProc  337				/*  variant for list views - no bottom line */

															/*  List Box proc IDs  */
#define kControlListBoxProc 			352
#define kControlListBoxAutoSizeProc 	353

															/*  Tagged data supported by list box  */
#define kControlListBoxListHandleTag 	'lhan'				/*  ListHandle */
#define kControlListBoxKeyFilterTag 	'fltr'				/*  ControlKeyFilterUPP */
#define kControlListBoxFontStyleTag 	'font'				/*  ControlFontStyleRec */

#define kControlListBoxDoubleClickTag 	'dblc'				/*  Boolean. Was last click a double-click? */
#define kControlListBoxLDEFTag 			'ldef'				/*  SInt16. ID of LDEF to use.  */

															/*  Theme Push Button/Check Box/Radio Button proc IDs  */
#define kControlPushButtonProc 			368
#define kControlCheckBoxProc 			369
#define kControlRadioButtonProc 		370
#define kControlPushButLeftIconProc 	374					/*  Standard pushbutton with left-side icon */
#define kControlPushButRightIconProc 	375					/*  Standard pushbutton with right-side icon */

															/*  Tagged data supported by standard buttons  */
#define kControlPushButtonDefaultTag 	'dflt'				/*  default ring flag */
															/*  Theme Scroll Bar proc IDs  */
#define kControlScrollBarProc 			384					/*  normal scroll bar */
#define kControlScrollBarLiveProc 		386					/*  live scrolling variant */

															/*  Theme Popup Button proc IDs  */
#define kControlPopupButtonProc 		400
#define kControlPopupFixedWidthVariant 	0x01
#define kControlPopupVariableWidthVariant  0x02
#define kControlPopupUseAddResMenuVariant  0x04
#define kControlPopupUseWFontVariant 	0x08				/*  kControlUsesOwningWindowsFontVariant */

#define kControlPopupButtonMenuHandleTag  'mhan'			/*  MenuHandle */
#define kControlPopupButtonMenuIDTag 	'mnid'				/*  SInt16 */

#define kControlRadioGroupProc 			416
#define popupFixedWidth 				0x01
#define popupVariableWidth 				0x02
#define popupUseAddResMenu 				0x04
#define popupUseWFont 					0x08

#define popupTitleBold 					0x0100
#define popupTitleItalic 				0x0200
#define popupTitleUnderline 			0x0400
#define popupTitleOutline 				0x0800
#define popupTitleShadow 				0x1000
#define popupTitleCondense 				0x2000
#define popupTitleExtend 				0x4000
#define popupTitleNoStyle 				0x8000

#define popupTitleLeftJust 				0x00000000
#define popupTitleCenterJust 			0x00000001
#define popupTitleRightJust 			0x000000FF


/*--------------------------cctb ¥ Control Color old Lookup Table----------------------*/
#ifdef oldTemp
	type 'cctb' {
			unsigned hex longint;									/* CCSeed				*/
			integer;												/* ccReserved			*/
			integer = $$Countof(ColorSpec) - 1;						/* ctSize				*/
			wide array ColorSpec {
					integer		cFrameColor,						/* partcode				*/
								cBodyColor,
								cTextColor,
								cElevatorColor;
					unsigned integer;								/* RGB:	red				*/
					unsigned integer;								/*		green			*/
					unsigned integer;								/*		blue			*/
			};
	};
#else
	type 'cctb' {
			unsigned hex longint = 0;								/* CCSeed				*/
			integer = 0;											/* ccReserved			*/
			integer = $$Countof(ColorSpec) - 1;						/* ctSize				*/
			wide array ColorSpec {
					integer		cFrameColor,						/* partcode				*/
								cBodyColor,
								cTextColor,
								cElevatorColor,
								cFillPatColor,
								cArrowsLight,
								cArrowsDark,
								cThumbLight,
								cThumbDark,
								cHiliteLight,
								cHiliteDark,
								cTitleBarLight,
								cTitleBarDark,
								cTingeLight,
								cTingeDark;
					unsigned integer;								/* RGB:	red				*/
					unsigned integer;								/*		green			*/
					unsigned integer;								/*		blue			*/
			};
	};
#endif


/*----------------------------CNTL ¥ Control Template-----------------------------------*/
type 'CNTL' {
		rect;													/* Bounds				*/
		integer;												/* Value				*/
		byte			invisible, visible; 					/* visible				*/
		fill byte;
		integer;												/* Max					*/
		integer;												/* Min					*/
		integer 		pushButProc,							/* ProcID				*/
						checkBoxProc,
						radioButProc,
						pushButProcUseWFont = 8,
						checkBoxProcUseWFont,
						radioButProcUseWFont,
						scrollBarProc = 16;
		longint;												/* RefCon				*/
		pstring;												/* Title				*/
};

#define	popupMenuCDEFproc		1008							/* ProcID 1008 = 16 * 63		*/


/*--------------------------ldes ¥ List Box Description Template------------------------*/
/*	Used in conjunction with the list box control.									  */

type 'ldes'
{
	switch
	{
		case versionZero:
			key integer = 0;	/* version */

			integer;												/* Rows					*/
			integer;												/* Columns				*/
			integer; 												/* Cell Height			*/
			integer;												/* Cell Width			*/
			byte			noVertScroll, hasVertScroll;			/* Vert Scroll			*/
			fill byte;												/* Filler Byte			*/
			byte			noHorizScroll, hasHorizScroll;			/* Horiz Scroll			*/
			fill byte;												/* Filler Byte			*/
			integer;												/* LDEF Res ID			*/
			byte			noGrowSpace, hasGrowSpace;				/* HasGrow?				*/
			fill byte;
	};
};


/*-------------------------------tab# ¥ Tab Control Template-----------------------------*/
type 'tab#'
{
	switch
	{
		case versionZero:
			key integer = 0;	/* version */

			integer = $$Countof(TabInfo);
			array TabInfo
			{
				integer;											/* Icon Suite ID		*/
				pstring;											/* Tab Name				*/
				fill long;											/* Reserved				*/
				fill word;											/* Reserved				*/
			};
	};
};


#endif /* __CONTROLS_R__ */

