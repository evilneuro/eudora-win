///////////////////////////////////////////////////////////////////////////////
// AutoSerializedRegionIface.h
//
// Created: 11/01/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __AUTO_SERIALIZED_REGION_IFACE_H_
#define __AUTO_SERIALIZED_REGION_IFACE_H_

template <class T>
class AutoSerializedRegionMT {

public:

     AutoSerializedRegionMT() { _serialized_region.EnterMT(); }
    ~AutoSerializedRegionMT() { _serialized_region.LeaveMT(); }

private:

    T _serialized_region;

};

#endif //  __AUTO_SERIALIZED_REGION_IFACE_H_
