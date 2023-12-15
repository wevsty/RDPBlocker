#define ProjectName "RDPBlocker"
#define ServiceWrapperAppName "RDPBlocker_service.exe"
#define ServiceWrapperConfigName "RDPBlocker_service.xml"
#define ReleaseVersion "1.2.6.5"
#define ProjectURL "https://github.com/wevsty/RDPBlocker"
#define GUID "B476FB3F-F5A2-4F34-A0A6-E74EA3962FAD"
#define OutputFileName "rdpblocker_setup"

[Setup]
AppId={{{#GUID}}
AppName={#ProjectName}
AppVersion={#ReleaseVersion}
AppSupportURL={#ProjectURL}

VersionInfoOriginalFileName={#OutputFileName}.exe
VersionInfoVersion={#ReleaseVersion}
VersionInfoDescription={#ProjectName} installer

DefaultDirName={commonpf}\{#ProjectName}
DefaultGroupName={#ProjectName}
AllowNoIcons=yes
LicenseFile=.\Release\LICENSE
OutputBaseFilename={#OutputFileName}
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
var
    SecurityConfigPage: TWizardPage;
    ChkForceNetworkUserAuth: TCheckBox;
    ChkDisableNTLMV1: TCheckBox;
    ChkForceSecurityLayer: TCheckBox;
    ChkMinEncryptionLevel: TCheckBox;
    ChkMinTLSVersion: TCheckBox;

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
    Result:= true;
end;

// 获取卸载程序路径
function GetUninstallerPath(): String;
var
    UninstallRegisterPath: String;
begin
    Result := '';
    UninstallRegisterPath := 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{{#GUID}}_is1';
    if RegQueryStringValue(HKLM, UninstallRegisterPath, 'UninstallString', Result) then
    begin
        Result := RemoveQuotes(Result);
    end;
end;

// 卸载旧版本
function UninstallOldVersion(): Boolean;
var
    ResultCode: Integer;
    UninstallPath: String;
begin
    Result := false;
    UninstallPath := GetUninstallerPath();
    if UninstallPath <> '' then
    begin
        Exec(RemoveQuotes(UninstallPath), '', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);
    end;
    Result:= true;
end;

// 安全配置页面
procedure CreateSecurityConfigPage();
begin
    SecurityConfigPage := CreateCustomPage(
    wpInfoBefore,
    'RDPService Security Config',
    'Configure security options'
    );
    ChkForceNetworkUserAuth := TCheckBox.Create(SecurityConfigPage);
    ChkForceNetworkUserAuth.Left := ScaleX(40);
    ChkForceNetworkUserAuth.Top := ScaleY(40);
    ChkForceNetworkUserAuth.Width := SecurityConfigPage.SurfaceWidth;
    ChkForceNetworkUserAuth.Height := ScaleY(20);
    ChkForceNetworkUserAuth.Caption := 'Force enable NetworkUserAuth';
    ChkForceNetworkUserAuth.Parent := SecurityConfigPage.Surface;
    ChkForceNetworkUserAuth.Enabled := true;
    ChkForceNetworkUserAuth.Checked := true;
    
    ChkDisableNTLMV1 := TCheckBox.Create(SecurityConfigPage);
    ChkDisableNTLMV1.Left := ScaleX(40);
    ChkDisableNTLMV1.Top := ChkForceNetworkUserAuth.Top + ScaleY(40);
    ChkDisableNTLMV1.Width := SecurityConfigPage.SurfaceWidth;
    ChkDisableNTLMV1.Height := ScaleY(20);
    ChkDisableNTLMV1.Caption := 'Disable NTLMV1 authentication';
    ChkDisableNTLMV1.Parent := SecurityConfigPage.Surface;
    ChkDisableNTLMV1.Enabled := true;
    ChkDisableNTLMV1.Checked := true;
    
    ChkForceSecurityLayer := TCheckBox.Create(SecurityConfigPage);
    ChkForceSecurityLayer.Left := ScaleX(40);
    ChkForceSecurityLayer.Top := ChkDisableNTLMV1.Top + ScaleY(40);
    ChkForceSecurityLayer.Width := SecurityConfigPage.SurfaceWidth;
    ChkForceSecurityLayer.Height := ScaleY(20);
    ChkForceSecurityLayer.Caption := 'Force enable SecurityLayer';
    ChkForceSecurityLayer.Parent := SecurityConfigPage.Surface;
    ChkForceSecurityLayer.Enabled := true;
    ChkForceSecurityLayer.Checked := true;

    ChkMinEncryptionLevel := TCheckBox.Create(SecurityConfigPage);
    ChkMinEncryptionLevel.Left := ScaleX(40);
    ChkMinEncryptionLevel.Top := ChkForceSecurityLayer.Top + ScaleY(40);
    ChkMinEncryptionLevel.Width := SecurityConfigPage.SurfaceWidth;
    ChkMinEncryptionLevel.Height := ScaleY(20);
    ChkMinEncryptionLevel.Caption := 'Set MinEncryptionLevel';
    ChkMinEncryptionLevel.Parent := SecurityConfigPage.Surface;
    ChkMinEncryptionLevel.Enabled := true;
    ChkMinEncryptionLevel.Checked := true;

    ChkMinTLSVersion := TCheckBox.Create(SecurityConfigPage);
    ChkMinTLSVersion.Left := ScaleX(40);
    ChkMinTLSVersion.Top := ChkMinEncryptionLevel.Top + ScaleY(40);
    ChkMinTLSVersion.Width := SecurityConfigPage.SurfaceWidth;
    ChkMinTLSVersion.Height := ScaleY(20);
    ChkMinTLSVersion.Caption := 'Set MinTLSVersion';
    ChkMinTLSVersion.Parent := SecurityConfigPage.Surface;
    ChkMinTLSVersion.Enabled := true;
    ChkMinTLSVersion.Checked := true;

end;

// 安装向导初始化
procedure InitializeWizard();
begin
  CreateSecurityConfigPage;
end;

// 安装程序初始化
function InitializeSetup(): Boolean;
begin
    UninstallOldVersion()
    Result := true;
end;

// 安装步骤
procedure CurStepChanged(CurStep: TSetupStep);
begin
    if CurStep = ssInstall then 
    begin
        Log('ssInstall');
        ServiceWrapperCommand('stop {#ServiceWrapperConfigName}');
        if (ChkForceNetworkUserAuth.Checked) then
        begin
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Policies\Microsoft\Windows NT\Terminal Services', 'UserAuthentication', 1);
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp', 'UserAuthentication', 1);
        end;
        if (ChkDisableNTLMV1.Checked) then
        begin
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Lsa', 'LmCompatibilityLevel', 5);
        end;
        if (ChkForceSecurityLayer.Checked) then
        begin
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Policies\Microsoft\Windows NT\Terminal Services', 'SecurityLayer', 2);
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp', 'SecurityLayer', 2);
        end;
        if (ChkMinEncryptionLevel.Checked) then
        begin
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Policies\Microsoft\Windows NT\Terminal Services', 'MinEncryptionLevel', 3);
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp', 'MinEncryptionLevel', 3);
        end;
        if (ChkMinTLSVersion.Checked) then
        begin
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\SecurityProviders\SCHANNEL\Protocols\TLS 1.0\Server', 'Enabled', 0);
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\SecurityProviders\SCHANNEL\Protocols\TLS 1.0\Client', 'Enabled', 0);
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\SecurityProviders\SCHANNEL\Protocols\TLS 1.1\Server', 'Enabled', 0);
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\SecurityProviders\SCHANNEL\Protocols\TLS 1.1\Client', 'Enabled', 0);
        end;
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
        ServiceWrapperCommand('install {#ServiceWrapperConfigName}');
        ServiceWrapperCommand('start {#ServiceWrapperConfigName}');
    end;
end;

// 卸载步骤
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
    if CurUninstallStep = usUninstall then 
    begin
        // Uninstall phase
        Log('usUninstall');
        ServiceWrapperCommand('stop {#ServiceWrapperConfigName}');
        ServiceWrapperCommand('uninstall {#ServiceWrapperConfigName}');
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
