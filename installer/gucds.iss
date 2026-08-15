#ifndef MyAppVersion
  #define MyAppVersion "5.7.4"
#endif
#ifndef ChineseLanguageFile
  #define ChineseLanguageFile "..\build\installer-languages\ChineseSimplified.isl"
#endif

#define MyAppName "QL-IOT App"
#define MyAppDisplayName "奇力智造上位机调试系统 QL-IOT App"
#define MyAppPublisher "北京奇力建通工程技术有限公司"
#define MyAppExeName "gucds_app.exe"
#define PortableDir "..\dist\GUCDSQt-portable-win-x64"

[Setup]
AppId={{2EB5B6B9-65E4-4BF8-A828-D17308926358}
AppName={#MyAppDisplayName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppDisplayName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={localappdata}\Programs\QL-IOT App
DefaultGroupName={#MyAppDisplayName}
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=QL-IOT-App-{#MyAppVersion}-win-x64-Setup
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
WizardStyle=modern
SetupIconFile=installer-package-icon-final.ico
SetupLogging=yes
CloseApplications=yes
RestartApplications=no
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppDisplayName} Setup
VersionInfoProductName={#MyAppDisplayName}
VersionInfoProductVersion={#MyAppVersion}

[Languages]
Name: "chinesesimp"; MessagesFile: "{#ChineseLanguageFile}"
Name: "english"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
chinesesimp.DesktopIcon=创建桌面快捷方式
english.DesktopIcon=Create a desktop shortcut
chinesesimp.LaunchProgram=启动 {#MyAppDisplayName}
english.LaunchProgram=Launch {#MyAppDisplayName}

[Tasks]
Name: "desktopicon"; Description: "{cm:DesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#PortableDir}\*"; DestDir: "{app}"; Excludes: "data\gucds.sqlite"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#PortableDir}\data\gucds.sqlite"; DestDir: "{app}\data"; Flags: onlyifdoesntexist uninsneveruninstall

[Icons]
Name: "{group}\{#MyAppDisplayName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\卸载 QL-IOT App (Uninstall)"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppDisplayName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent
