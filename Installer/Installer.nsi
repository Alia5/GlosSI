

!define APP_NAME "GlosSI"
!define COMP_NAME "Peter Repukat - Flatspotsoftware"
!define WEB_SITE "https://glossi.flatspot.pictures/"
!define VERSION "0.1.2.0-18-g27056b4"
!define COPYRIGHT "Peter Repukat - FlatspotSoftware  © 2017-2022"
!define DESCRIPTION "SteamInput compatibility tool"
!define INSTALLER_NAME "GlosSI-Installer.exe"
!define MAIN_APP_EXE "GlosSIConfig.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define REG_ROOT "HKLM"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

!define PROD_NAME "GlosSI"
!define PROD_PUBLISHER "Peter Repukat - FLatspotSoftware"
!define PROD_WEB_ADDRESS "https://glossi.flatspot.pictures/"
!define PROD_WEB_ADDRESS_SUPP "https://glossi.flatspot.pictures/"

!define REG_START_MENU "Start Menu Folder"

!define TARGET_APP_EXE "GlosSITarget.exe"

var SM_Folder

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor ZLIB
Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
XPStyle on
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
InstallDir "$PROGRAMFILES64\GlosSI"

######################################################################

Unicode true

SetCompressor lzma
SetDateSave off 

;--------------------------------
;Include Modern UI

!include "MUI2.nsh"


!define MUI_WELCOMEPAGE_TITLE "GlosSI Installer"
!define MUI_WELCOMEPAGE_TEXT "Welcome to the GlosSI Installer!"
;Extra space for the title area
;!insertmacro MUI_WELCOMEPAGE_TITLE_3LINES


!insertmacro MUI_PAGE_WELCOME
 
!insertmacro MUI_PAGE_LICENSE "../LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!ifdef REG_START_MENU
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "GlosSI"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${REG_ROOT}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_PATH}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${REG_START_MENU}"
!insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif



!insertmacro MUI_PAGE_INSTFILES


Function finishpageaction
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
FunctionEnd

!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION finishpageaction

!define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_APP_EXE}"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"


Section "GlosSI" -MainProgram
    SectionIn RO

        ${INSTALL_TYPE}
        SetOverwrite ifnewer
        SetOutPath "$INSTDIR"

        File /r /x "GlosSI*.zip" "..\x64\Release\*"

SectionEnd

Section "Visual Studio Runtime"
  SetOutPath "$INSTDIR"
  File  "..\x64\Release\vc_redist.x64.exe"
  ExecWait '"$INSTDIR\vcredist_x64.exe" /quiet'
SectionEnd

Section "ViGEmBus Driver"
  SetOutPath "$INSTDIR"
  File  "..\x64\Release\ViGEmBusSetup_x64.exe"
  ExecWait '"$INSTDIR\ViGEmBusSetup_x64.exe"'
SectionEnd

Section "HidHide Driver"
  SetOutPath "$INSTDIR"
  File  "..\x64\Release\HidHideSetup.exe"
  ExecWait '"$INSTDIR\HidHideSetup.exe"'
SectionEnd


Section -Icons_Reg


SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\uninstall.exe"


!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
CreateDirectory "$SMPROGRAMS\$SM_Folder"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}-Target.lnk" "$INSTDIR\${TARGET_APP_EXE}"
!insertmacro MUI_STARTMENU_WRITE_END
!endif

!ifndef REG_START_MENU
CreateDirectory "$SMPROGRAMS\GlosSI"
CreateShortCut "$SMPROGRAMS\GlosSI\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
!endif

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

!ifdef WEB_SITE
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "URLInfoAbout" "${WEB_SITE}"
!endif
SectionEnd


Section Uninstall
${INSTALL_TYPE}
RmDir /r "$INSTDIR"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_GETFOLDER "Application" $SM_Folder
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}-Target.lnk"
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\$SM_Folder"
!endif

!ifndef REG_START_MENU
Delete "$SMPROGRAMS\GlosSI\${APP_NAME}.lnk"
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\GlosSI"
!endif

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
SectionEnd

