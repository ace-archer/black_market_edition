; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Black Market Edition"
#define MyAppVersion "1b9"
#define MyAppPublisher "p0358"
#define MyAppURL "https://github.com/p0358/black_market_edition"

#define public Dependency_NoExampleSetup
#include "..\thirdparty\InnoDependencyInstaller\CodeDependencies.iss"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{AB99661E-6887-4F75-8105-A2883F54C935}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}   
;DefaultDirName=C:\Users\p0358\source\repos\bme\installer\test                                                                                   
;DefaultDirName={reg:HKLM\SOFTWARE\WOW6432Node\Respawn\Titanfall, Install Dir}
DefaultDirName={code:GetInstallationPath}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
DisableProgramGroupPage=yes
;InfoBeforeFile=C:\Users\p0358\source\repos\bme\installer\pre_install_info.txt
DisableWelcomePage=yes
InfoAfterFile=C:\Users\p0358\source\repos\bme\installer\post_install_info.txt
OutputDir=C:\Users\p0358\source\repos\bme\installer
OutputBaseFilename=bme_installer
SetupIconFile=C:\Users\p0358\source\repos\bme\installer\Titanfall_red.ico
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest
DisableReadyPage=yes
ShowLanguageDialog=no
CreateUninstallRegKey=no
DirExistsWarning=no
EnableDirDoesntExistWarning=no
CloseApplications=force
CloseApplicationsFilter=*.exe,*.dll,*.chm,*.bik,*.asi,*.log,*.json
;ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64 arm64 ia64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

;[Icons]
;Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Dirs]
Name: "{app}\bin\x64_retail"; Flags: uninsneveruninstall
Name: "{app}\bme"

[Files]
;Source: "source\winmm.dll"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist
;Source: "source\bin\x64_retail\winmm.dll"; DestDir: "{app}\bin\x64_retail"; Flags: ignoreversion onlyifdoesntexist
;Source: "source\bme\bme.asi"; DestDir: "{app}\bme"; Flags: ignoreversion
Source: "source\bme\bme.dll"; DestDir: "{app}\bme"; Flags: ignoreversion
Source: "source\bme\bme.bsp"; DestDir: "{app}\bme"; Flags: ignoreversion
Source: "source\bme\bme.log"; DestDir: "{app}\bme"; Flags: ignoreversion onlyifdoesntexist
Source: "source\bme\bme_channel.txt"; DestDir: "{app}\bme"; Flags: ignoreversion
Source: "source\bme\crashpad_handler.exe"; DestDir: "{app}\bme"; Flags: ignoreversion
Source: "source\bme\crashpad_wer.dll"; DestDir: "{app}\bme"; Flags: ignoreversion
;Source: "source\discord_game_sdk.dll"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist
Source: "source\r1\media\fov_video_15ms_480x400.bik"; DestDir: "{app}\r1\media"; Flags: ignoreversion onlyifdoesntexist
Source: "{app}\bin\x64_retail\launcher.dll"; DestDir: "{app}\bin\x64_retail"; DestName: "launcher.org.dll"; Flags: external skipifsourcedoesntexist onlyifdoesntexist uninsneveruninstall
Source: "source\bin\x64_retail\launcher.dll"; DestDir: "{app}\bin\x64_retail"; Flags: ignoreversion uninsneveruninstall
Source: "source\Titanfall_alt.exe"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist

[InstallDelete]
; old loaders from beta
Type: files; Name: "{app}\winmm.dll"
Type: files; Name: "{app}\bin\x64_retail\winmm.dll"
Type: files; Name: "{app}\bme\bme.asi"
; unused anymore
Type: files; Name: "{app}\source\discord_game_sdk.dll"

[Code]
function InitializeSetup: Boolean;
begin
  Dependency_AddVC2015To2022;
  Result := True;
end;



procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectDir then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall)
  //else
  //  WizardForm.NextButton.Caption := SetupMessage(msgButtonNext);
end;



var
  InstallationPath: string;

function GetInstallationPath(Param: string): string;
begin
  { Detected path is cached, as this gets called multiple times }
  if InstallationPath = '' then
  begin
    if RegQueryStringValue(
         HKLM32, 'SOFTWARE\Respawn\Titanfall',
         'Install Dir', InstallationPath) then
    begin
      Log('Detected Origin installation: ' + InstallationPath);
    end
      else
    if RegQueryStringValue(
         HKLM64, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 1454890',
         'InstallLocation', InstallationPath) then
    begin
      Log('Detected Steam installation: ' + InstallationPath);
    end
      else
    begin
      InstallationPath := 'C:\Program Files (x86)\Origin Games\Titanfall';
      Log('No installation detected, using the default path: ' + InstallationPath);
    end;
  end;
  Result := InstallationPath;
end;



// https://stackoverflow.com/questions/31918706/backup-files-and-restore-them-on-uninstall-with-inno-setup
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  FindRec: TFindRec;
  SourcePath: string;
  DestDir: string;
  DestPath: string;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    DestDir := RemoveBackslash(ExpandConstant('{app}'));
    if FindFirst(DestDir + '\bin\x64_retail\launcher.org.dll', FindRec) then
    begin
      //repeat
        if (FindRec.Name <> '.') and (FindRec.Name <> '..') then
        begin
          SourcePath := DestDir + '\bin\x64_retail\launcher.org.dll';
          DestPath := DestDir + '\bin\x64_retail\launcher.dll';
          Log(Format('Restoring %s to %s', [SourcePath, DestPath]));
          if not DeleteFile(DestPath) then
          begin
            Log('Delete failed');
          end;
          if not RenameFile(SourcePath, DestPath) then
          begin
            Log('Restore failed');
          end;
        end;
      //until not FindNext(FindRec);
    end; 
    if FindFirst(DestDir + '\Titanfall.org.exe', FindRec) then
    begin
      //repeat
        if (FindRec.Name <> '.') and (FindRec.Name <> '..') then
        begin
          SourcePath := DestDir + '\Titanfall.org.exe';
          DestPath := DestDir + '\Titanfall.exe';
          Log(Format('Restoring %s to %s', [SourcePath, DestPath]));
          if not DeleteFile(DestPath) then
          begin
            Log('Delete failed');
          end;
          if not RenameFile(SourcePath, DestPath) then
          begin
            Log('Restore failed');
          end;
        end;
      //until not FindNext(FindRec);
    end; 
    FindClose(FindRec);
  end;
end;