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

[Files]
Source: "build\Release\ccc.exe"; DestDir: "{app}"

[Registry]
Root: HKCU; \
Subkey: "Environment"; \
ValueType: expandsz; \
ValueName: "Path"; \
ValueData: "{olddata};{app}"; \
Check: NeedsAddPath('{app}')

[Code]

function NeedsAddPath(Dir: string): Boolean;
var
  Path: string;
begin
  Result := True;

  if RegQueryStringValue(HKCU,
                         'Environment',
                         'Path',
                         Path) then
    Result := Pos(LowerCase(Dir),
                  LowerCase(Path)) = 0;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    SendMessageTimeout(
      HWND_BROADCAST,
      WM_SETTINGCHANGE,
      0,
      Integer(PChar('Environment')),
      SMTO_ABORTIFHUNG,
      5000,
      0);
  end;
end;
