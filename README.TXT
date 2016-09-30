DEBUG:

Configuration Properities -> Debugging -> Command Arguments:
------------------------------------------------------------
$(ProjectDir)res\reduced-dict-plastic.yml $(ProjectDir)res\outputboard.yml $(ProjectDir)res\outputimage.png 2 3 1
reduced-dict-plastic.yml outputboard.yml outputimage.png 2 3 1


C/C++ -> General -> Additional Include Directories:
---------------------------------------------------
$(ProjectDir)ext\libharu-2_3_0\include;$(ProjectDir)ext\aruco\include;$(INDE_OPENCV_DIR)\include


C/C++ -> Preprocessor -> Preprocessor Definitions:
--------------------------------------------------
WIN32;_DEBUG;_CONSOLE;_LIB;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)
_CRT_SECURE_NO_WARNINGS


Linker -> General -> Additional Library Directories:
----------------------------------------------------
$(ProjectDir)ext\libharu-2_3_0\lib\debug;$(OCVLIBDIR_D)


Linker -> Input -> Additional Dependencies:
-------------------------------------------
libhpdfsd.lib;$(OCVLIBs_D);%(AdditionalDependencies)



Tools -> Options -> Debugging -> Symbols -> "Microsoft Symbol Servers"





reduced-dict-plastic.yml MUSIM MAT NA 2 MIESTACH:
1. V $(ProjectDir) INAK HO NENACITA TATO FUNKCIA:


	aruco::Dictionary D;
	D.fromFile(dictionaryfile);
	if (D.size() == 0) {
		std::cerr << "Error: Dictionary is empty" << std::endl;
		exit(-1);
	}
    
2. V $(SolutionDir)Debug INAK NEVIEM (RESP VIEM ALE ZLE) S NIM PRACOVAT Z COMMAND LINEU