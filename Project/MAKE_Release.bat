set ArchiveName=GameRangerDirectConnectFixer_v1.3

set ReleaseDir=Release
set ReleaseUnpackedDir=ReleaseUnpacked

if not exist %ReleaseDir% mkdir %ReleaseDir%
if not exist %ReleaseUnpackedDir% mkdir %ReleaseUnpackedDir%

xcopy /Y Code\vc9\Release\*.exe %ReleaseUnpackedLauncherDir%\

tar.exe -a -c -C %ReleaseUnpackedDir% -f %ReleaseDir%\%ArchiveName%.zip *.*
