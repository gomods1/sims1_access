@echo off
rem Builds the mod into mod\out\:
rem   SpeechHost.exe + prism.dll  (x64; owns the screen reader connection)
rem   version.dll                 (x86 proxy/loader; goes next to Sims.exe)
rem   SimsAccess.dll              (x86 mod core; goes into <game>\SimsAccess\)
rem   sounds\*.wav                (the cursors' sound cues, made with genny from sounds\batch.json)
rem   docs\<tag>.html             (the player's manual, from docs\manual\*.md by tools\build_docs.py)
rem Usage: build.bat [deploy]
rem   deploy copies the files into the game folder and creates SimsAccess\dev.txt, which enables
rem   hot reload of SimsAccess.dll while the game runs and verbose logging. Files in use
rem   (version.dll and SpeechHost.exe while the game runs) are skipped with a note.
setlocal
set ROOT=%~dp0
set OUT=%ROOT%out
set PRISM=C:\Users\Nitropc\code\libs\prism\prism-windows-x64
set MH=%ROOT%third_party\minhook
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set GAME=S:\steam\steamapps\common\The Sims Legacy Collection
set CXXFLAGS=/nologo /std:c++17 /O2 /W3 /MT /utf-8 /DNOMINMAX /DWIN32_LEAN_AND_MEAN
for %%d in ("%OUT%" "%OUT%\obj64" "%OUT%\obj32" "%OUT%\objmh") do if not exist %%d mkdir %%d

rem ---- x64: SpeechHost ----
setlocal
call %VCVARS% x64 >nul 2>&1 || exit /b 1
cl %CXXFLAGS% /EHsc /DUNICODE /D_UNICODE /I"%PRISM%\include" /Fo"%OUT%\obj64\\" ^
   "%ROOT%speechhost\speech_host.cpp" /Fe"%OUT%\SpeechHost.exe" ^
   /link /SUBSYSTEM:WINDOWS shell32.lib ole32.lib xaudio2.lib || exit /b 1
endlocal
copy /y "%PRISM%\dynamic\release\bin\prism.dll" "%OUT%\" >nul

rem ---- x86: MinHook, loader, core ----
setlocal
call %VCVARS% x86 >nul 2>&1 || exit /b 1
cl /nologo /c /O2 /MT /W3 /I"%MH%\include" /Fo"%OUT%\objmh\\" ^
   "%MH%\src\buffer.c" "%MH%\src\hook.c" "%MH%\src\trampoline.c" "%MH%\src\hde\hde32.c" || exit /b 1
lib /nologo /out:"%OUT%\objmh\minhook.lib" "%OUT%\objmh\*.obj" || exit /b 1

cl %CXXFLAGS% /EHsc /LD /Fo"%OUT%\obj32\\" "%ROOT%loader\loader.cpp" /Fe"%OUT%\version.dll" ^
   /link /DEF:"%ROOT%loader\version.def" user32.lib bcrypt.lib || exit /b 1

rem /EHa: game calls are wrapped in SEH (game::Guarded) and C++ objects must unwind through it.
set CORE=%ROOT%core
cl %CXXFLAGS% /Zi /EHa /LD /I"%MH%\include" /Fo"%OUT%\obj32\\" /Fd"%OUT%\obj32\core.pdb" ^
   "%CORE%\main.cpp" "%CORE%\util\*.cpp" "%CORE%\lang\*.cpp" "%CORE%\speech\*.cpp" "%CORE%\input\*.cpp" ^
   "%CORE%\game\*.cpp" "%CORE%\nav\*.cpp" /Fe"%OUT%\SimsAccess.dll" ^
   /link /DEBUG /MAP:"%OUT%\SimsAccess.map" /PDB:"%OUT%\SimsAccess.pdb" "%OUT%\objmh\minhook.lib" user32.lib advapi32.lib shell32.lib ole32.lib || exit /b 1
endlocal

rem ---- the manual (pandoc) ----
python "%ROOT%..\tools\build_docs.py" || exit /b 1

if /i not "%1"=="deploy" goto :eof
if not exist "%GAME%\SimsAccess" mkdir "%GAME%\SimsAccess"
copy /y "%OUT%\version.dll" "%GAME%\" >nul 2>&1 || echo note: version.dll in use, not replaced
copy /y "%OUT%\SimsAccess.dll" "%GAME%\SimsAccess\" >nul || exit /b 1
copy /y "%OUT%\SimsAccess.pdb" "%GAME%\SimsAccess\" >nul 2>&1
copy /y "%OUT%\SpeechHost.exe" "%GAME%\SimsAccess\" >nul 2>&1 || echo note: SpeechHost.exe in use, not replaced
copy /y "%OUT%\prism.dll" "%GAME%\SimsAccess\" >nul 2>&1 || echo note: prism.dll in use, not replaced
if not exist "%GAME%\SimsAccess\sounds" mkdir "%GAME%\SimsAccess\sounds"
copy /y "%ROOT%sounds\*.wav" "%GAME%\SimsAccess\sounds\" >nul || exit /b 1
if not exist "%GAME%\SimsAccess\docs" mkdir "%GAME%\SimsAccess\docs"
copy /y "%OUT%\docs\*.html" "%GAME%\SimsAccess\docs\" >nul || exit /b 1
if not exist "%GAME%\SimsAccess\dev.txt" echo hot reload of SimsAccess.dll and verbose logging> "%GAME%\SimsAccess\dev.txt"
echo deployed to %GAME%
endlocal
