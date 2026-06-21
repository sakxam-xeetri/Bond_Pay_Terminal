@echo off
title BondPay UI Local Server
echo ==============================================
echo   BondPay Terminal UI Local Web Server
echo ==============================================
echo.
echo Trying to start server using Node.js (npx)...
where node >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo Node.js found. Starting server on http://localhost:8080
    echo Please open: http://localhost:8080/bondpay-ui.html
    echo.
    npx -y http-server -p 8080
    goto end
)

echo Node.js not found. Trying Python...
where python >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo Python found. Starting server on http://localhost:8000
    echo Please open: http://localhost:8000/bondpay-ui.html
    echo.
    python -m http.server 8000
    goto end
)

echo.
echo ERROR: Neither Node.js nor Python is installed on your system!
echo Please install Node.js (https://nodejs.org) or Python (https://python.org), 
echo or use the "Live Server" extension in VS Code to run the HTML on localhost.
echo.
pause

:end
