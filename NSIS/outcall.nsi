Var AppName
Var ExeName
Var RegistryKey
Var DLLName
Var LanguageFileName
Var OSExtension
;Var OutputSetupFile

Function .onInit
  ;StrCpy $OutputSetupFile "outcall-1.60-setup.exe"
  StrCpy $AppName "OutCALL"
  StrCpy $ExeName "outcall.exe"
  StrCpy $DLLName "outcall.dll"
  StrCpy $LanguageFileName "outcall.po"
  StrCpy $RegistryKey "Software\BicomSystems\OutCALL"
  StrCpy $OSExtension "occfg" 
FunctionEnd


Function un.onInit
  StrCpy $AppName "OutCALL"
  StrCpy $ExeName "outcall.exe"
  StrCpy $DLLName "outcall.dll"
  StrCpy $LanguageFileName "outcall.po"
  StrCpy $RegistryKey "Software\BicomSystems\OutCALL"
  StrCpy $OSExtension "occfg" 
FunctionEnd


;********* kill outcall process *******
Function CloseOutCALL
Push $5

loop:
  push $ExeName
  ;processwork::existsprocess
  ProcessWork::ExistsProcess
  pop $5
  IntCmp $5 0 done

  MessageBox MB_OKCANCEL|MB_ICONINFORMATION "Please close $AppName and press OK to proceed, or press Cancel to abort." IDCANCEL BailOut
  push $ExeName
Goto loop

BailOut:
  Abort

done:
Pop $5
FunctionEnd

;********* kill outlook process *******
Function CloseOutlook
Push $5

loop:
  push "outlook.exe"
  processwork::existsprocess
  pop $5
  IntCmp $5 0 done

  MessageBox MB_OKCANCEL|MB_ICONINFORMATION "Please close Microsoft Outlook and press OK to proceed, or press Cancel to abort." IDCANCEL BailOut

  push "outlook.exe"
Goto loop

BailOut:
  Abort

done:
Pop $5
FunctionEnd



;****************************** UNINSTALL FUNCTIONS ***************************

;********* kill outcall process *******
Function un.CloseOutCALL
Push $5

loop:
  push $ExeName
  processwork::existsprocess
  pop $5
  IntCmp $5 0 done

  MessageBox MB_OKCANCEL|MB_ICONINFORMATION "Please close $AppName and press OK to proceed, or press Cancel to abort." IDCANCEL BailOut
  push $ExeName
Goto loop

BailOut:
  Abort

done:
Pop $5
FunctionEnd

;********* kill outlook process *******
Function un.CloseOutlook
Push $5

loop:
  push "outlook.exe"
  processwork::existsprocess
  pop $5
  IntCmp $5 0 done

  MessageBox MB_OKCANCEL|MB_ICONINFORMATION "Please close Microsoft Outlook and press OK to proceed, or press Cancel to abort." IDCANCEL BailOut

  push "outlook.exe"
Goto loop

BailOut:
  Abort

done:
Pop $5
FunctionEnd


;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------

;General

  ;Name and file
  Name "$AppName"
  OutFile "outcall-1.60-setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Bicom Systems\OutCALL"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "$RegistryKey" ""

  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE ".\license.txt"
  ;!insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "$RegistryKey" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Installer Section" SecInstaller

  Call CloseOutCALL
  Call CloseOutlook

  SetOutPath "$INSTDIR"
  SetShellVarContext all
  
  ;ADD YOUR OWN FILES HERE...
  ;File /oname=OutCALL.exe outcall-noauth.exe
  File "outcall.exe"
  ;File "outcall.exe.manifest"
  File "outcall.dll"
  File "languages.txt"
  File "outcall.po"

  File "sqlite3.dll"
  File "intl.dll"
  File "iconv.dll"
  File "msvcr71.dll"

  ;this is to make sure OutCALL.exe uses our version of MFC & c/c++ runtime DLLs
  
  ; VS 2010 distributable files

  SetOutPath "$INSTDIR\Microsoft.VC100.CRT"
  File /r "Microsoft.VC100.CRT\*.*"
  SetOutPath "$INSTDIR\Microsoft.VC100.MFC"
  File /r "Microsoft.VC100.MFC\*.*"

  ;we must copy MFC & C/C++ files again so that Outlook is able to load outcall.dll (if they do not exist already)
  SetOverwrite off
  SetOutPath "$SYSDIR"

  ; VS 2005 distributable files needed for OutCall.dll (Outlook plugin)  
  File "Microsoft.VC100.CRT\msvcr100.dll"
  File "Microsoft.VC100.CRT\msvcp100.dll"
  File "Microsoft.VC100.MFC\mfc100.dll"
  File "Microsoft.VC100.MFC\mfc100u.dll"
  
  SetOutPath "$INSTDIR\lang\bs-BA-Latn\LC_MESSAGES"
  File "lang\Bosnian\outcall.mo"
  SetOutPath "$INSTDIR\lang\es\LC_MESSAGES"
  File "lang\Spanish\outcall.mo"

  SetOutPath "$INSTDIR\lang\fr\LC_MESSAGES"
  File "lang\French\outcall.mo"
  SetOutPath "$INSTDIR\lang\sk\LC_MESSAGES"
  File "lang\Slovak\outcall.mo"

  SetOutPath "$INSTDIR\lang\de\LC_MESSAGES"
  File "lang\German\outcall.mo"
  SetOutPath "$INSTDIR\lang\it\LC_MESSAGES"
  File "lang\Italian\outcall.mo"
  SetOutPath "$INSTDIR\lang\ru\LC_MESSAGES"
  File "lang\Russian\outcall.mo"
  
  ;Store installation folder
  WriteRegStr HKLM "$RegistryKey" "" $INSTDIR

  WriteRegStr HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Run" "$AppName" "$INSTDIR\$ExeName background"
  WriteRegStr HKCR ".$OSExtension" "" "$AppName"
  WriteRegStr HKCR "$AppName\DefaultIcon" "" "$INSTDIR\$ExeName"
  WriteRegStr HKCR "$AppName\Shell\Open with $AppName\Command" "" "$INSTDIR\$ExeName %1"

  WriteRegStr HKLM "SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions" "$AppName" "4.0;$INSTDIR\$DLLName;1;11111111111111;1111111"
  Delete "$LOCALAPPDATA\Microsoft\Outlook\extend.dat"
  WriteRegStr HKLM "SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps" "$ExeName" "Microsoft Outlook"

  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    SetOutPath "$INSTDIR"
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$AppName.lnk" "$INSTDIR\$ExeName"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

; //////// CREATE REGISTRY KEYS FOR ADD/REMOVE PROGRAMS IN CONTROL PANEL /////////

WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OutCALL" "DisplayName" "OutCALL (remove only)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OutCALL" "UninstallString" "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecInstaller ${LANG_ENGLISH} "Installer section."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecInstaller} $(DESC_SecInstaller)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...  

  Call un.CloseOutCALL
  Call un.CloseOutlook

  Delete "$INSTDIR\$ExeName"
  Delete "$INSTDIR\$ExeName.manifest"
  Delete "$INSTDIR\$DLLName"
  Delete "$INSTDIR\languages.txt"
  Delete "$INSTDIR\$LanguageFileName"

  Delete "$INSTDIR\sqlite3.dll"
  Delete "$INSTDIR\intl.dll"
  Delete "$INSTDIR\iconv.dll"
  Delete "$INSTDIR\msvcr71.dll"
  
  RMDir "$INSTDIR\Microsoft.VC100.CRT"
  RMDir "$INSTDIR\Microsoft.VC100.CRT"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR\lang"
  RMDir "$INSTDIR"

  SetShellVarContext all
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
    
  Delete "$SMPROGRAMS\$MUI_TEMP\$AppName.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
	ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  
  DeleteRegKey HKCU "$RegistryKey"
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "$AppName"

  DeleteRegValue HKLM "SOFTWARE\Microsoft\Exchange\Client\Extensions" "$AppName"
  ;DeleteRegValue HKLM "SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps" "$ExeName"

  DeleteRegKey HKLM "$RegistryKey"

  DeleteRegKey HKCR ".$OSExtension"
  DeleteRegKey HKCR "$AppName"

  SetShellVarContext current  

  Delete "$LOCALAPPDATA\Microsoft\Outlook\extend.dat"
  RMDir /r "$LOCALAPPDATA\$AppName"

SectionEnd
