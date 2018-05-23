// PgGlobals.h
// Wrapper classes for the Paige API global data structures. Provides namespace
// protection while automatically handling startup/shutdown sequence.

#ifndef _PG_GLOBALS_H_
#define _PG_GLOBALS_H_

#include "Paige.h"
#include "Machine.h"
#include "assert.h"


// PgGlobals:
// Wrapper class for Paige APIs "pg_globals".

class PgGlobals : public pg_globals
{
public:
    PgGlobals(){};
    ~PgGlobals(){};

    // two stage initialization
    bool init();
};


// PgMemGlobals:
// Wrapper class for Paige APIs "pgm_globals".

class PgMemGlobals : public pgm_globals
{
public:
    PgMemGlobals(){};
    ~PgMemGlobals(){};

    // two stage initialization. In the case of pgm_globals,
    // there will probably never be a need for this.
    bool init(){return true;}
};


// PgSharedAccess:
// Provides decoupled instantiation/destruction of the shared wrapper classes
// for pg_globals and pg_mem_globals.

class PgSharedAccess
{
    // da goods
    static PgGlobals pgGlobals;
    static PgMemGlobals pgMemGlobals;

    // access control & instantiation/destruction
    static unsigned ucRef;
    static bool bInitialized;
    static bool bLocked;

    void Startup(){
	assert(ucRef == 0); assert(bInitialized == false); bLocked = false;}

    void Shutdown();

public:
    PgSharedAccess();
    ~PgSharedAccess();

    // because I haired-out
    static void SillyShutdown();

protected:
    // references
    PgGlobals* PgGlobalsPtr(){assert(!bLocked); return &pgGlobals;}
    PgMemGlobals* PgMemGlobalsPtr(){assert(!bLocked); return &pgMemGlobals;}
};


#endif		// _PG_GLOBALS_H_
