# Microsoft Developer Studio Project File - Name="adplug" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=adplug - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "adplug.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "adplug.mak" CFG="adplug - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "adplug - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "adplug - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "adplug - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "adplug - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "adplug - Win32 Release"
# Name "adplug - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\players\a2m.cpp
# End Source File
# Begin Source File

SOURCE=.\adplug.cpp
# ADD CPP /I "players"
# End Source File
# Begin Source File

SOURCE=.\players\amd.cpp
# End Source File
# Begin Source File

SOURCE=.\players\bam.cpp
# End Source File
# Begin Source File

SOURCE=.\players\d00.cpp
# End Source File
# Begin Source File

SOURCE=.\players\dfm.cpp
# End Source File
# Begin Source File

SOURCE=.\emuopl.cpp
# End Source File
# Begin Source File

SOURCE=.\fmopl.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\players\hsc.cpp
# End Source File
# Begin Source File

SOURCE=.\players\hsp.cpp
# End Source File
# Begin Source File

SOURCE=.\players\imf.cpp
# End Source File
# Begin Source File

SOURCE=.\players\ksm.cpp
# End Source File
# Begin Source File

SOURCE=.\players\lds.cpp

!IF  "$(CFG)" == "adplug - Win32 Release"

!ELSEIF  "$(CFG)" == "adplug - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\players\mid.cpp
# End Source File
# Begin Source File

SOURCE=.\players\mkj.cpp
# End Source File
# Begin Source File

SOURCE=.\players\mtk.cpp
# End Source File
# Begin Source File

SOURCE=.\players\protrack.cpp
# End Source File
# Begin Source File

SOURCE=.\players\rad.cpp
# End Source File
# Begin Source File

SOURCE=.\players\raw.cpp
# End Source File
# Begin Source File

SOURCE=.\realopl.cpp
# End Source File
# Begin Source File

SOURCE=.\players\rol.cpp
# End Source File
# Begin Source File

SOURCE=.\players\s3m.cpp
# End Source File
# Begin Source File

SOURCE=.\players\sa2.cpp
# End Source File
# Begin Source File

SOURCE=.\players\sng.cpp
# End Source File
# Begin Source File

SOURCE=.\players\u6m.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\players\a2m.h
# End Source File
# Begin Source File

SOURCE=.\adplug.h
# End Source File
# Begin Source File

SOURCE=.\players\amd.h
# End Source File
# Begin Source File

SOURCE=.\players\bam.h
# End Source File
# Begin Source File

SOURCE=.\players\d00.h
# End Source File
# Begin Source File

SOURCE=.\players\dfm.h
# End Source File
# Begin Source File

SOURCE=.\emuopl.h
# End Source File
# Begin Source File

SOURCE=.\fm.h
# End Source File
# Begin Source File

SOURCE=.\fmopl.h
# End Source File
# Begin Source File

SOURCE=.\players\hsc.h
# End Source File
# Begin Source File

SOURCE=.\players\hsp.h
# End Source File
# Begin Source File

SOURCE=.\players\imf.h
# End Source File
# Begin Source File

SOURCE=.\players\imfcrc.h
# End Source File
# Begin Source File

SOURCE=.\players\ksm.h
# End Source File
# Begin Source File

SOURCE=.\players\lds.h

!IF  "$(CFG)" == "adplug - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "adplug - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\players\mid.h
# End Source File
# Begin Source File

SOURCE=.\players\mididata.h
# End Source File
# Begin Source File

SOURCE=.\players\mkj.h
# End Source File
# Begin Source File

SOURCE=.\players\mtk.h
# End Source File
# Begin Source File

SOURCE=.\opl.h
# End Source File
# Begin Source File

SOURCE=.\players\player.h
# End Source File
# Begin Source File

SOURCE=.\players\protrack.h
# End Source File
# Begin Source File

SOURCE=.\players\rad.h
# End Source File
# Begin Source File

SOURCE=.\players\raw.h
# End Source File
# Begin Source File

SOURCE=.\realopl.h
# End Source File
# Begin Source File

SOURCE=.\players\rol.h
# End Source File
# Begin Source File

SOURCE=.\players\s3m.h
# End Source File
# Begin Source File

SOURCE=.\players\sa2.h
# End Source File
# Begin Source File

SOURCE=.\silentopl.h
# End Source File
# Begin Source File

SOURCE=.\players\sng.h
# End Source File
# Begin Source File

SOURCE=.\players\u6m.h
# End Source File
# End Group
# End Target
# End Project
