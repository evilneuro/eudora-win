plgen.exe -- generates a boilerplate playlist from a specified directory.

Description:
	Plgen is used to create playlists for image files that are contained in a
	local directory. Default values for the various playlist fields can be
	specified in the initialization file "plgen.ini". The output of Plgen can be
	used as input to Dbgen (dbgen.exe) to create the playlist database used
	by the AdWare version of Eudora.


Instructions:
	- Copy plgen.exe, and plgen.ini into your Eudora executable directory.
		Dbgen will also need to be copied into this directory if a playlist
		database is to generated. XML generation does not depend on Dbgen.

	- Put all image files in a common directory.
		The best location for this directory is within the folder where Eudora
		has been installed.

	- Run Plgen at the command line, providing the name of the image directory.

		example:  plgen graphics playlist.xml

		The example above will create a playlist entry for every file found in
		the subdirectory "graphics", and write its output to the file
		"playlist.xml". In this case, the "graphics" directory is a child of the
		Eudora directory where Plgen is located.

		example:  plgen graphics | dbgen

		In this example the output from Plgen is being used as input to its
		sister program Dbgen (dbgen.exe), which generates the final database
		format used by Eudora, without creating an intermediate XML file. This
		option is useful when editing of the generated XML is not required.


Gotchas:
	- Complicated directory path specifications.
		Plgen has limited ability to deal with directory path specifications.
		Wildcards are not needed, and should not be used---all files contained
		in the specified directory will be added to the playlist.

		When specifying subdirectories, do not use relative paths that are not
		subdirectories of the current working directory. If you must use a
		directory that is not a child of the current one, then the only option
		is to use a fully qualified path specification.

		example:  ..\..\graphics       <--- This is bad!
		example:  \Eudora\graphics     <--- So is this.

		example:  c:\Eudora\graphics   <--- This works, but why would you?

