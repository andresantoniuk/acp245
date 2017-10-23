!ifndef VERSION
  !define VERSION 'anonymous-build'
!endif

!ifdef OUTFILE
  OutFile "${OUTFILE}"
!else
  OutFile acp245-${VERSION}-setup.exe
!endif

; Location of Python
var PythonPath

InstallDir $PROGRAMFILES\Edantech\ACP245
InstallDirRegKey HKLM "Software\Edantech\ACP245" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

!include "MUI.nsh"
!include "env.nsh"

Name "ACP 245 Library"
Caption "ACP 245 Library ${VERSION} Setup"
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

Section "ACP245 Library" ACP245Lib
    SetOutPath "$INSTDIR"

    File "..\src\acp245.dll"
SectionEnd

Section "ACP245 Development" ACP245Devel
    SetDetailsPrint textonly
    DetailPrint "Installing headers..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR"
    File "..\src\acp245.lib"

    SetOutPath "$INSTDIR\include"
    File "..\src\acp245.h"
    File "..\src\acp_types.h"
    File "..\src\acp_init.h"
    File "..\src\acp_el.h"
    File "..\src\acp_ie.h"
    File "..\src\acp_msg.h"
    File "..\src\acp_key.h"
    File "..\src\acp_err.h"

SectionEnd

Section "ACP245 Examples" ACP245Examples
    SetDetailsPrint textonly
    DetailPrint "Installing examples..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\examples\acp245_example\acp245_example\"
	File "examples\acp245_example\acp245_example\acp245_example.vcproj"
	File "examples\acp245_example\acp245_example\acp245_example.c"
	File "examples\acp245_example\acp245_example\ReadMe.txt"

    SetOutPath "$INSTDIR\examples\acp245_example\"
	File "examples\acp245_example\acp245_example.sln"

SectionEnd

Section "ACP245 Documentation" ACP245Docs
    SetDetailsPrint textonly
    DetailPrint "Installing documentation..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\docs"
	File /oname=ACP245_API_Reference.pdf "..\doxygen\latex\refman.pdf"
	File /r "..\doxygen\html"

SectionEnd

Section "ACP245 Library - Python Bindings" ACP245LibPython
    SetDetailsPrint textonly
    DetailPrint "Installing ACP245 Library Python Bindings..."
    SetDetailsPrint listonly

    SetOutPath "$PythonPath\Lib\site-packages\acp245"
    File "..\src\acp245.dll"
    File "..\python\src\pdu_gen.pyd"
    File "..\python\src\log.pyd"
    File "..\python\build\lib\acp245\__init__.pyc"
    File "..\python\build\lib\acp245\pdu.pyc"
    File "..\python\build\lib\acp245\stdmsg.pyc"

    WriteRegStr HKLM "Software\Edantech\ACP245_Python" "Install_Dir" $PythonPath

SectionEnd

Section -post # hidden required section
    SetDetailsPrint textonly
    DetailPrint "Configuring environment..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR"
    File /oname=LICENSE.txt "..\LICENSE"

    Push "E_ACP245_PATH"
    Push $INSTDIR
    Call WriteEnvStr

    Push "E_ACP245_LICENSE"
    Push "$INSTDIR\license.sig"
    Call WriteEnvStr

    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

;--------------------------------
;Descriptions

    ;Language strings
    LangString DESC_ACP245Lib ${LANG_ENGLISH} "Installs the required DLL to run ACP 245 applications."
    LangString DESC_ACP245Devel ${LANG_ENGLISH} "Installs the header and library files to compile and link new ACP 245 applications."
    LangString DESC_ACP245Examples ${LANG_ENGLISH} "Installs a Visual C++ 2008 project example."
    LangString DESC_ACP245Docs ${LANG_ENGLISH} "Installs the library documentation."
    LangString DESC_ACP245LibPython ${LANG_ENGLISH} "Installs the required Python libraries to use the ACP245 library from Python"

    LangString DESC_ACP245Lib ${LANG_PORTUGUESEBR} "Instala o DLL exigido para correr novas aplicações do ACP 245."
    LangString DESC_ACP245Devel ${LANG_PORTUGUESEBR} "Instala o cabeçalho e arquivos de biblioteca para compilar e ligar novas aplicações do ACP 245."
    LangString DESC_ACP245Examples ${LANG_PORTUGUESEBR} "Instala o exemplo du projecto para Visual C++ 2008."
    LangString DESC_ACP245Docs ${LANG_PORTUGUESEBR} "Instala a documentação da biblioteca."
    LangString DESC_ACP245LibPython ${LANG_PORTUGUESEBR} "Instala as bibliotecas exigida para usar a biblioteca ACP245 no Python."

    LangString DESC_ACP245Lib ${LANG_SPANISH} "Instala los DLL necesarios para ejecutar aplicaciones ACP 245."
    LangString DESC_ACP245Devel ${LANG_SPANISH} "Instala los cabezales y librerías necesarias para compilar y enlazar nuevas aplicaciones ACP 245."
    LangString DESC_ACP245Examples ${LANG_SPANISH} "Instala un ejemplo de proyecto para Visual C++ 2008."
    LangString DESC_ACP245Docs ${LANG_SPANISH} "Instala la documentación de la biblioteca."
    LangString DESC_ACP245LibPython ${LANG_SPANISH} "Instala las bibliotecas necesarias para usar la biblioteca ACP245 desde Python."

    ;Assign language strings to sections
    !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
        !insertmacro MUI_DESCRIPTION_TEXT ${ACP245Lib} $(DESC_ACP245Lib)
        !insertmacro MUI_DESCRIPTION_TEXT ${ACP245Devel} $(DESC_ACP245Devel)
        !insertmacro MUI_DESCRIPTION_TEXT ${ACP245Examples} $(DESC_ACP245Examples)
        !insertmacro MUI_DESCRIPTION_TEXT ${ACP245Docs} $(DESC_ACP245Docs)
        !insertmacro MUI_DESCRIPTION_TEXT ${ACP245LibPython} $(DESC_ACP245LibPython)
    !insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
    IfFileExists $INSTDIR\LICENSE.txt package_installed
        MessageBox MB_YESNO "It does not appear that ACP 245 Library is installed in the directory '$INSTDIR'.$\r$\nContinue anyway (not recommended)?" IDYES package_installed
        Abort "Uninstall aborted by user"
    package_installed:

    Push "E_ACP245_PATH"
    Call un.DeleteEnvStr

    Push "E_ACP245_LICENSE"
    Call un.DeleteEnvStr

    Delete "$INSTDIR\LICENSE.txt"
    Delete "$INSTDIR\acp245.lib"
    Delete "$INSTDIR\acp245.dll"
    Delete "$INSTDIR\include\acp245.h"
    Delete "$INSTDIR\include\acp_types.h"
    Delete "$INSTDIR\include\acp_init.h"
    Delete "$INSTDIR\include\acp_el.h"
    Delete "$INSTDIR\include\acp_ie.h"
    Delete "$INSTDIR\include\acp_msg.h"
    Delete "$INSTDIR\include\acp_key.h"
    Delete "$INSTDIR\include\acp_err.h"
    RMDir "$INSTDIR\include\"

	Delete "$INSTDIR\examples\acp245_example\acp245_example\acp245_example.vcproj"
	Delete "$INSTDIR\examples\acp245_example\acp245_example\acp245_example.c"
	Delete "$INSTDIR\examples\acp245_example\acp245_example\ReadMe.txt"
    RMDir "$INSTDIR\examples\acp245_example\acp245_example"

	Delete "$INSTDIR\examples\acp245_example\acp245_example.sln"
    RMDir "$INSTDIR\examples\acp245_example"
    RMDir "$INSTDIR\examples\"

	Delete "$INSTDIR\docs\ACP245_API_Reference.pdf"

    RMDir /r "$INSTDIR\docs\html\"
    RMDir /r "$INSTDIR\docs\"

StrCmp $PythonPath "" oops ok
oops:
    Goto done
ok:
    Delete "$PythonPath\Lib\site-packages\acp245\acp245.dll"
    Delete "$PythonPath\Lib\site-packages\acp245\pdu_gen.pyd"
    Delete "$PythonPath\Lib\site-packages\acp245\log.pyd"
    Delete "$PythonPath\Lib\site-packages\acp245\__init__.pyc"
    Delete "$PythonPath\Lib\site-packages\acp245\pdu.pyc"
    Delete "$PythonPath\Lib\site-packages\acp245\stdmsg.pyc"
    RMDir "$PythonPath\Lib\site-packages\acp245\"
done:

    DeleteRegKey HKLM "Software\Edantech\ACP245_Python"
    DeleteRegKey /ifempty HKLM "Software\Edantech\ACP245"

    Delete "$INSTDIR\Uninstall.exe"

    RMDir "$INSTDIR"
    RMDir "$PROGRAMFILES\Edantech\"

SectionEnd

Function .onInit
System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
Pop $R0

StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort

ReadRegStr $9 HKLM "SOFTWARE\Python\PythonCore\2.6\InstallPath" ""
StrCmp $9 "" oops ok
oops:
    ; Guess where Python 2.6 is.
    StrCpy $PythonPath "c:\Python26\"
    ; Fallback installation directory is "Program Files"
    Goto done
ok:
    StrCpy $PythonPath "$9"
done:
FunctionEnd

Function un.onInit
ReadRegStr $9 HKLM "Software\Edantech\ACP245_Python" "Install_Dir"
StrCpy $PythonPath "$9"
FunctionEnd
