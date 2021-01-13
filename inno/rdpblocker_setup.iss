; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "RDPBlocker"
#define MyAppVersion "1.1.0.0"
#define MyAppURL "https://github.com/wevsty/RDPBlocker"
#define MyAppExeName "RDPBlocker.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B476FB3F-F5A2-4F34-A0A6-E74EA3962FAD}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=.\Release\LICENSE
OutputBaseFilename=rdpblocker_setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl";

[Files]
Source: ".\Release\RDPBlocker.exe"; DestDir: "{app}"; Flags: ignoreversion;BeforeInstall: ServiceStop('{#MyAppName}')
Source: ".\Release\config.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\Release\nssm.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Run]
Filename: {app}\nssm.exe; Parameters: "stop {#MyAppName}" ; Flags: runhidden
Filename: {app}\nssm.exe; Parameters: "install {#MyAppName} ""{app}\{#MyAppExeName}""" ; Flags: runhidden
Filename: {app}\nssm.exe; Parameters: "start {#MyAppName}" ; Flags: runhidden
;Filename: "{cmd}"; Parameters: "sc create RDPBlocker start= auto DisplayName= RDPBlocker binPath= ""{app}\RDPBlocker.exe"""; Flags: runhidden

[UninstallRun]
Filename: "{app}\nssm.exe"; Parameters: "stop {#MyAppName}"; Flags: runhidden waituntilterminated; RunOnceId: "StopServices"
Filename: "{app}\nssm.exe"; Parameters: "remove {#MyAppName} confirm"; Flags: runhidden waituntilterminated; RunOnceId: "RemoveServices"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[ThirdParty]
UseRelativePaths=True

[UninstallDelete]
Type: files; Name: "{app}\*.*"

[Code]
procedure ServiceStop(SvcName: String);
var
  ResultCode: Integer;
begin
    //Exec('cmd.exe', '/c sc stop SvcName', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);
    //ShellExec('', 'cmd.exe', 'cmd /c net stop SvcName& sc delete SvcName', '', SW_HIDE, ewWaitUntilTerminated, ResultCode); 
    Exec('sc.exe', 'stop ' + SvcName , '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;