#define ProjectName "RDPBlocker"
#define ServiceWrapperAppName "RDPBlocker_service.exe"
#define ServiceWrapperConfigName "RDPBlocker_service.xml"
#define ReleaseVersion "1.2.5.4"
#define ProjectURL "https://github.com/wevsty/RDPBlocker"
#define GUID "B476FB3F-F5A2-4F34-A0A6-E74EA3962FAD"

[Setup]
AppId={{{#GUID}}
AppName={#ProjectName}
AppVersion={#ReleaseVersion}
AppSupportURL={#ProjectURL}

DefaultDirName={commonpf}\{#ProjectName}
DefaultGroupName={#ProjectName}
AllowNoIcons=yes
LicenseFile=.\Release\LICENSE
OutputBaseFilename={#ProjectName}_setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
WizardStyle=Modern
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: ".\Release\*.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\Release\*.yaml"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\Release\*.xml"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\Release\*.bat"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#ProjectName}"; Filename: "{app}\RDPBlocker.exe"
Name: "{group}\config.yaml"; Filename: "{app}\config.yaml"
Name: "{group}\base_directory"; Filename: "{app}"
Name: "{group}\{cm:UninstallProgram,{#ProjectName}}"; Filename: "{uninstallexe}"

[ThirdParty]
UseRelativePaths=True

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
// 服务管理器命令
function ServiceWrapperCommand(Command: String): Boolean;
var
    ResultCode: Integer;
    WrapperPath: String;
    FileExit: Boolean;
begin
    WrapperPath := ExpandConstant('{app}/{#ServiceWrapperAppName}')
    FileExit := FileExists(WrapperPath);
    if FileExit = true then
    begin
        Exec(WrapperPath, Command, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
end;

// 安装向导初始化
function InitializeSetup(): Boolean;
var
    ResultCode: Integer;
    uicmd: String;
begin
    Result:= false
    if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{{#GUID}}_is1', 'UninstallString', uicmd) then
    begin
    Exec(RemoveQuotes(uicmd), '', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);
    end;
    Result:= true;
end;

// 安装步骤
procedure CurStepChanged(CurStep: TSetupStep);
var
    ResultCode: Integer;
begin
    if CurStep = ssInstall then 
    begin
        Log('ssInstall');
        ServiceWrapperCommand('stop {#ServiceWrapperConfigName}')
    end 
    else if CurStep = ssPostInstall then 
    begin
        // Post-install phase
        Log('ssPostInstall');
    end 
    else if CurStep = ssDone then 
    begin
        // Before setup terminates after a successful install.
        Log('ssDone');
        ServiceWrapperCommand('install {#ServiceWrapperConfigName}')
        ServiceWrapperCommand('start {#ServiceWrapperConfigName}')
    end;
end;

// 卸载步骤
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
    ResultCode: Integer;
begin
    if CurUninstallStep = usUninstall then 
    begin
        // Uninstall phase
        Log('usUninstall');
        ServiceWrapperCommand('stop {#ServiceWrapperConfigName}')
        ServiceWrapperCommand('uninstall {#ServiceWrapperConfigName}')
    end
    else if CurUninstallStep = usPostUninstall then
    begin
        // Post-uninstall phase
        Log('usPostUninstall');
    end
    else if CurUninstallStep = usDone then
    begin
        // Uninstallation done phase
        Log('usDone');
    end;
end;
