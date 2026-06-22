[Setup]
AppName=CheapChatContext
AppVersion=1.0.0
DefaultDirName={localappdata}\CheapChatContext
DefaultGroupName=CheapChatContext
PrivilegesRequired=lowest
OutputDir=output
OutputBaseFilename=cccInstaller
Compression=lzma
SolidCompression=yes
; Tells Windows to refresh environment variables after install/uninstall
ChangesEnvironment=yes 

[Files]
Source: "build\Release\ccc.exe"; DestDir: "{app}"

[Registry]
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; Check: NeedsAddPath('{app}')

[Code]
function NeedsAddPath(Dir: string): Boolean;
var
  Path: string;
begin
  if RegQueryStringValue(HKCU, 'Environment', 'Path', Path) then
  begin
    // Pad with semicolons to prevent false positive substring matches
    Result := Pos(';' + LowerCase(Dir) + ';', ';' + LowerCase(Path) + ';') = 0;
  end
  else
    Result := True;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Path, AppDir: string;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    AppDir := ExpandConstant('{app}');
    if RegQueryStringValue(HKCU, 'Environment', 'Path', Path) then
    begin
      // Remove the app directory from the Path
      StringChangeEx(Path, ';' + AppDir, '', True);
      StringChangeEx(Path, AppDir + ';', '', True);
      StringChangeEx(Path, AppDir, '', True);
      RegWriteStringValue(HKCU, 'Environment', 'Path', Path);
    end;
  end;
end;
