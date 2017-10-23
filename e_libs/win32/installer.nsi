!ifndef VERSION
  !define VERSION 'anonymous-build'
!endif

!ifdef OUTFILE
  OutFile "${OUTFILE}"
!else
  OutFile e_libs-${VERSION}-setup.exe
!endif

InstallDir $PROGRAMFILES\Edantech\e_libs
InstallDirRegKey HKLM "Software\Edantech\e_libs" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

!include "MUI.nsh"
!include "env.nsh"

Name "Edantech e_libs"
Caption "Edantech e_libs ${VERSION} Setup"
BrandingText " "

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "edantech.bmp" ; optional

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the Edantech site for the latest news and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.edantech.com/"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES


!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Spanish"

Section "e_libs Development" e_libs_devel

    SetOutPath "$INSTDIR"
    File "..\src\e_libs_static.lib"
    File "..\src\e_libs_conf_static.lib"
    File "..\src\e_libs_crc_static.lib"
    File "..\src\e_libs_net_static.lib"
    File "..\tests\e_libs_check_static.lib"

    SetDetailsPrint textonly
    DetailPrint "Installing headers..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\include"
    File "..\src\win32\e_libs.h"
	File "..\src\linux-gnu\e_port.h"
	File "..\src\linux-gnu\e_types.h"
	File "..\src\e_buff.h"
	File "..\src\e_client.h"
	File "..\src\e_conf.h"
	File "..\src\e_crc.h"
	File "..\src\e_gprs.h"
	File "..\src\e_gps.h"
	File "..\src\e_io.h"
	File "..\src\e_log.h"
	File "..\src\e_mem.h"
	File "..\src\e_queue.h"
	File "..\src\e_splint_macros.h"
	File "..\src\e_syslog.h"
	File "..\src\e_time.h"
	File "..\src\e_timer.h"
	File "..\src\e_units.h"
	File "..\src\e_util.h"
	File "..\tests\e_check_extra.h"
	File "..\tests\e_check.h"
	File "..\tests\e_minicheck.h"

SectionEnd

Section "e_libs documentation" e_libs_docs
    SetDetailsPrint textonly
    DetailPrint "Installing documentation..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\docs"
	File /oname=e_libs_API_Reference.pdf "..\doxygen\latex\refman.pdf"
	File /r "..\doxygen\html"

SectionEnd

Section -post # hidden required section
    SetDetailsPrint textonly
    DetailPrint "Configuring environment..."
    SetDetailsPrint listonly

    WriteRegStr HKLM "Software\Edantech\e_libs" "" $INSTDIR

    SetOutPath "$INSTDIR"
    File /oname=LICENSE.txt "..\LICENSE"

    Push "E_LIBS_PATH"
    Push $INSTDIR
    Call WriteEnvStr

    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

;--------------------------------
;Descriptions

    ;Language strings
    LangString DESC_e_libs_devel ${LANG_ENGLISH} "Installs the required libraries to develop applications using e_libs."
    LangString DESC_e_libs_devel ${LANG_PORTUGUESEBR} "Instala as bibliotecas exigidas para desenvolver aplicações com e_libs."
    LangString DESC_e_libs_devel ${LANG_SPANISH} "Instala las bibliotecas requeridas para desarrollar aplicaciones con e_libs."

    LangString DESC_e_libs_docs ${LANG_ENGLISH} "Installs the e_libs documentation."
    LangString DESC_e_libs_docs ${LANG_PORTUGUESEBR} "Instala a documentação da e_libs."
    LangString DESC_e_libs_docs ${LANG_SPANISH} "Instala la documentación de e_libs."

    ;Assign language strings to sections
    !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
        !insertmacro MUI_DESCRIPTION_TEXT ${e_libs_devel} $(DESC_e_libs_devel)
        !insertmacro MUI_DESCRIPTION_TEXT ${e_libs_docs} $(DESC_e_libs_docs)
    !insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
    IfFileExists $INSTDIR\LICENSE.txt package_installed
        MessageBox MB_YESNO "It does not appear that e_libs Library is installed in the directory '$INSTDIR'.$\r$\nContinue anyway (not recommended)?" IDYES package_installed
        Abort "Uninstall aborted by user"
    package_installed:

    Push "E_LIBS_PATH"
    Call un.DeleteEnvStr

    Delete "$INSTDIR\e_libs_static.lib"
    Delete "$INSTDIR\e_libs_conf_static.lib"
    Delete "$INSTDIR\e_libs_crc_static.lib"
    Delete "$INSTDIR\e_libs_net_static.lib"
    Delete "$INSTDIR\e_libs_check_static.lib"

    Delete "$INSTDIR\include\e_libs.h"
    Delete "$INSTDIR\include\e_port.h"
    Delete "$INSTDIR\include\e_types.h"
	Delete "$INSTDIR\include\e_buff.h"
	Delete "$INSTDIR\include\e_client.h"
	Delete "$INSTDIR\include\e_conf.h"
	Delete "$INSTDIR\include\e_crc.h"
	Delete "$INSTDIR\include\e_gprs.h"
	Delete "$INSTDIR\include\e_gps.h"
	Delete "$INSTDIR\include\e_io.h"
	Delete "$INSTDIR\include\e_log.h"
	Delete "$INSTDIR\include\e_mem.h"
	Delete "$INSTDIR\include\e_queue.h"
	Delete "$INSTDIR\include\e_splint_macros.h"
	Delete "$INSTDIR\include\e_syslog.h"
	Delete "$INSTDIR\include\e_time.h"
	Delete "$INSTDIR\include\e_timer.h"
	Delete "$INSTDIR\include\e_units.h"
	Delete "$INSTDIR\include\e_util.h"
	Delete "$INSTDIR\include\e_check_extra.h"
	Delete "$INSTDIR\include\e_check.h"
	Delete "$INSTDIR\include\e_minicheck.h"
    RMDir "$INSTDIR\include\"

    Delete "$INSTDIR\LICENSE.txt"

	Delete "$INSTDIR\docs\e_libs_API_Reference.pdf"

    RMDir /r "$INSTDIR\docs\html\"
    RMDir "$INSTDIR\docs\"

    DeleteRegKey /ifempty HKLM "Software\Edantech\e_libs"

    Delete "$INSTDIR\Uninstall.exe"

    RMDir "$INSTDIR"

    RMDir $PROGRAMFILES\Edantech\

SectionEnd

Function .onInit
System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
Pop $R0

StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort
FunctionEnd
