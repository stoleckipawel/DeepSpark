@echo off
REM Preview drafts locally with Hugo (double-click this file)
cd /d "%~dp0"
echo Starting Hugo dev server WITH drafts...
start "Hugo" cmd /k "hugo server --buildDrafts --bind 127.0.0.1 --port 1313 --baseURL http://localhost:1313/ --appendPort=false"
echo Waiting for server to start...
timeout /t 3 /nobreak >nul
echo Opening article preview in Chrome...
start "" chrome "http://localhost:1313/posts/frame-graph-plan-preview/" 2>nul || start "" "http://localhost:1313/posts/frame-graph-plan-preview/"
echo Hugo is running in the new window. Close the Hugo window to stop the server.
pause
