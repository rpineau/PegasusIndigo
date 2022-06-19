; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Pegasus Astro Indigo filter wheel X2 Plugin"
#define MyAppVersion "1.0"
#define MyAppPublisher "RTI-Zone"
#define MyAppURL "https://rti-zone.org"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{9B420BF7-AC54-4243-9A8E-05BF5AC90F0F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={code:TSXInstallDir}\Resources\Common
DefaultGroupName={#MyAppName}

; Need to customise these
; First is where you want the installer to end up
OutputDir=installer
; Next is the name of the installer
OutputBaseFilename=PegasusIndigo_X2_Installer
; Final one is the icon you would like on the installer. Comment out if not needed.
SetupIconFile=rti_zone_logo.ico
Compression=lzma
SolidCompression=yes
; We don't want users to be able to select the drectory since read by TSXInstallDir below
DisableDirPage=yes
; Uncomment this if you don't want an uninstaller.
;Uninstallable=no
CloseApplications=yes
DirExistsWarning=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Dirs]
Name: "{app}\Plugins\FilterWheelPlugIns";
Name: "{app}\Plugins64\FilterWheelPlugIns";

[Files]
Source: "filterwheellist PegasusIndigo.txt";                                  DestDir: "{app}\Miscellaneous Files"; Flags: ignoreversion
Source: "filterwheellist PegasusIndigo.txt";                                  DestDir: "{app}\Miscellaneous Files"; Flags: ignoreversion; DestName: "filterwheellist64 PegasusIndigo.txt"
; 32 bit
Source: "libPegasusIndigo\Win32\Release\libPegasusIndigo.dll";    DestDir: "{app}\Plugins\FilterWheelPlugIns"; Flags: ignoreversion; Check: DirExists(ExpandConstant('{app}\Plugins64\FilterWheelPlugIns'))
; 64 bit
Source: "libPegasusIndigo\x64\Release\libPegasusIndigo.dll";      DestDir: "{app}\Plugins64\FilterWheelPlugIns"; Flags: ignoreversion; Check: DirExists(ExpandConstant('{app}\Plugins64\FilterWheelPlugIns'))


[Code]
{* Below is a function to read TheSkyXInstallPath.txt and confirm that the directory does exist
   This is then used in the DefaultDirName above
   *}
var
  Location: String;
  LoadResult: Boolean;

function TSXInstallDir(Param: String) : String;
begin
  LoadResult := LoadStringFromFile(ExpandConstant('{userdocs}') + '\Software Bisque\TheSkyX Professional Edition\TheSkyXInstallPath.txt', Location);
  if not LoadResult then
    LoadResult := LoadStringFromFile(ExpandConstant('{userdocs}') + '\Software Bisque\TheSky Professional Edition 64\TheSkyXInstallPath.txt', Location);
    if not LoadResult then
      LoadResult := BrowseForFolder('Please locate the installation path for TheSkyX', Location, False);
      if not LoadResult then
        RaiseException('Unable to find the installation path for TheSkyX');
  if not DirExists(Location) then
    RaiseException('TheSkyX installation directory ' + Location + ' does not exist');
  Result := Location;
end;
