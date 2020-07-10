--------------
Assignment 3
Srishti Singhal
--------------


MAKE FILE
---------
Make
	Using make, will compile all the necessary files, producing 2 executables named:
										-diskinfo
										-disklist
										
Make clean
	Using make clean, will remove all executable files created from the make command, leaving the source code.


FUNCTIONALITY
-------------

Run
	diskinfo
		"diskinfo <Name of image file>"
		This will output to console the following information for the image file in the format:
			"Super block information:
			Block size: 
			Block count:
			FAT starts:
			FAT blocks:
			Root directory start:
			Root directory blocks:

			FAT information:
			Free Blocks:
			Reserved Blocks:
			Allocated Blocks: "
	disklist
		"disklist <name of image file>"
	
		This will output to console the following information for the contents of the specified directory in the format:
		"F <size> <name> <modified date/time>
		 D <size> <name> <modified date/time>"
	
	
END OF README
	
