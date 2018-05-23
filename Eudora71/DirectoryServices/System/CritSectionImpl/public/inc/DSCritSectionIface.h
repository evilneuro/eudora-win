///////////////////////////////////////////////////////////////////////////////
// DSCritSectionIface.h
//
// Created: 08/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORYSERVICES_CRIT_SECTION_IFACE_H_
#define __DIRECTORYSERVICES_CRIT_SECTION_IFACE_H_


class  DirectoryServicesCritSectionMT {

public:
          DirectoryServicesCritSectionMT();
         ~DirectoryServicesCritSectionMT();

    void  EnterMT();
    void  LeaveMT();

private:

    static UINT nRefCnt;
};

#endif //  __DIRECTORYSERVICES_CRIT_SECTION_IFACE_H_
