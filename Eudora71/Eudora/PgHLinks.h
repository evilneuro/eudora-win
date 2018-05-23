// PgHLinks.h -- Support for URLs, Attachments, etc.

#ifndef _PG_HLINKS_H_
#define _PG_HLINKS_H_

///////////////////////////////////////////////////////////////////////////////
// C++ declarations go here


// End of C++

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// "C" stuff goes here

    // callback entry point for Paige Lib
    PG_PASCAL (long) hlCallback( paige_rec_ptr pg, pg_hyperlink_ptr hypertext,
				 short command, short modifiers, long position,
				 pg_char_ptr URL );

    // public interface
    void PgInitHyperlinks( pg_ref pg );
    void PgProcessHyperlinks( pg_ref pg );

#ifdef __cplusplus
}    // end of "C"
#endif

#endif    // _PG_HLINKS_H_
