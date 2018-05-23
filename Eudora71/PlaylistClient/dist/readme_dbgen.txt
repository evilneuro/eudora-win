dbgen.exe -- generates a playlist database from an XML playlist.

Description:
	Dbgen is typically used in concert with Plgen (plgen.exe) to create a
	playlist database for the AdWare version of Eudora. The primary application
	is the creation of local-only databases to support "Demo Mode" without
	the need of a Playlist Server.


Instructions:
	- Copy dbgen.exe into your Eudora executable directory.
		Plgen (plgen.exe, and plgen.ini) will also need to be copied into this
		directory if XML playlist generation is desired.

		** Dbgen cannot be run while Eudora is running.

		** See the Plgen readme file for more about XML playlist generation.

	- Run Dbgen at the command line, providing the name of the XML playlist file
	  to be used for creating the playlist database.

		example:  dbgen playlist.xml
		example:  plgen graphics | dbgen

		The first example creates a database from the XML document contained
		in the file "playlist.xml". This document is easily generated using
		Plgen, but could also come from some alternate source.

		The second example uses Plgen to create a playlist from a subdirectory
		named "graphics" and redirects the output to Dbgen, bypassing the
		creation of an intermediate XML file. This option is useful when editing
		the generated XML is not required.

	- Database generation is complete.
