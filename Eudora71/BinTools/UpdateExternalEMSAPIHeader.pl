# Converts internal QC only "emsapi-win.h" into the external/public EMSAPI header "emsapi-win.h"
#
# Assumes following convention in emsapi-win.h:
# #ifdef INTERNAL_FOR_QC
# marks the start of definitions that are internal only - i.e. only for use by Eudora and QUALCOMM plugins
#
# #else // EXTERNAL_FOR_PUBLIC
# closes definitions that are internal only and starts definitions that are external only - i.e. for public use
#
# #endif // INTERNAL_FOR_QC
# closes definitions that are internal only - i.e. only for use by Eudora and QUALCOMM plugins
#
# #endif // EXTERNAL_FOR_PUBLIC
# closes definitions that are external only - i.e. for public use
#
# AND
#
# #ifdef INTERNAL_FOR_EUDORA
# marks the start of definitions that are internal only for Eudora (not for any plugin use, including QUALCOMM's)
#
# #else // EXTERNAL_FOR_PLUGINS
# closes definitions that are internal only for Eudora and starts definitions that are external for plugin use (including QUALCOMM's)
#
# #endif // INTERNAL_FOR_EUDORA
# closes definitions that are internal only for Eudora (not for any plugin use, including QUALCOMM's)
#
# #endif // EXTERNAL_FOR_PLUGINS
# closes definitions that are external for plugin use (including QUALCOMM's)
#
# Note that in each case above, EXTERNAL_FOR_... is only supported with #else - it is meant to provide
# alternative content for a preceding #ifdef INTERNAL_FOR_... section.
#
# This script does not distinguish between the two types of INTERNAL_FOR #ifdefs - the separate #defines
# are used so that the same internal file can be used to build both Eudora and QC plugins by defining
# the appropriate #define.
#
# Some attempt is made to handle nested #ifdef's, but it may not be robust particularly
# when it comes any nested use of "#else // EXTERNAL_ONLY".

# Make sure the directory ..\EMSAPI\EMSSDK exists before trying to open the output
# public header file ..\EMSAPI\EMSSDK\emsapi-win.h
if (!-d '..\EMSAPI') {
   mkdir('..\EMSAPI', 0777) || die "Failed to create directory \"EMSAPI\"";
}
if (!-d '..\EMSAPI\EMSSDK') {
   mkdir('..\EMSAPI\EMSSDK', 0777) || die "Failed to create directory \"EMSAPI\\EMSSDK\"";
}

open (INTERNALHEADER, '..\Eudora\emsapi-win.h') || die "Failed to open internal version of \"emsapi-win.h\": $!";
open (PUBLICHEADER, '>..\EMSAPI\EMSSDK\emsapi-win.h') || die "Failed to open public version of \"emsapi-win.h\": $!";

while (<INTERNALHEADER>) {
   if (m:^\s*#ifdef\s+INTERNAL_FOR:) {
      # Started an #ifdef INTERNAL_ONLY
      $internal++;
      $skipBlankLine = 0;
   }
   elsif ( (m:^\s*#endif\s*//\s*INTERNAL_FOR:) || (m:^\s*#endif\s*//\s*EXTERNAL_FOR:) ) {
      # Ended either an external section or an internal section
      if ($external > 0) {
         $external--;
      }
      else {
         if ($internal == 0) { die "Internal section closed when none was started!"; }
         $internal--;
      }

      if (m:<-SBL->:) {
         $skipBlankLine = 1;
      }
   }
   elsif (m:^\s*#else\s+//\s*EXTERNAL_FOR:) {
      # Internal section ended, external section begun
      if ($internal == 0) { die "$_ used when no internal section was started!"; }
      $internal--;
      $external++;
      $skipBlankLine = 0;
   }
   elsif (m:INTERNAL_ONLY:) {
      die "Possible stray reference to INTERNAL_FOR...";
   }
   elsif (m:EXTERNAL_ONLY:) {
      die "Possible stray reference to EXTERNAL_FOR...";
   }
   elsif ($internal == 0) {
      # We're not in an internal section and the line wasn't
      # a #ifdef, #else, or #endif that controls the internal/external sections.
      if (m:^PLUGIN_FUNCTION:) {
         # Translate PLUGIN_FUNCTION macro to 'extern "C"'
         print PUBLICHEADER 'extern "C"';
         # Remove the PLUGIN_FUNCTION from the line
         s/PLUGIN_FUNCTION//;
         # Print out the rest of the line
         print PUBLICHEADER;
      }
      elsif (m:^//:) {
         # Line is C++ comment only - skip it, only C comments are allowed
         # in the public version of emsapi-win.h. This allows us to use C++
         # style comments as private comments or line spacers.
      }
      elsif ( (m:^$:) && $skipBlankLine ) {
         # Skip blank line when indicated.
         # Allows white space in source file so that readability of #if's etc. is
         # improved without adding blank lines to the public version of emsapi-win.h.
      }
      else {
         # Nothing special - just print the line
         print PUBLICHEADER;
      }
      $skipBlankLine = 0;
   }
}

if ($internal > 0) { die "Detection of internal only sections of EMSAPI header still positive after EOF!"; }
if ($external > 0) { die "Detection of external only sections of EMSAPI header still positive after EOF!"; }

close (INTERNALHEADER) || die "Can't close \"emsapi-win.h\": $!";
close (PUBLICHEADER) || die "Can't close \"emsapi-win.h\": $!";

