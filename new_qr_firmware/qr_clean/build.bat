@echo off
setlocal enableextensions

REM Update this path to your installed MPLAB C18 root folder if different
set "MCC18=C:\Program Files (x86)\Microchip\mplabc18\v3.47"

if not exist "%MCC18%\bin\mcc18.exe" (
  echo ERROR: mcc18 not found at "%MCC18%\bin\mcc18.exe"
  echo Please install MPLAB C18 v3.47 and/or update MCC18 path in build.bat
  exit /b 1
)

set "PATH=%MCC18%\bin;%PATH%"
pushd "%~dp0"

REM Clean old objects
del /q *.o *.cof *.map *.hex 2>nul

REM Compile
mcc18 -p=18F4550 -I"%MCC18%\h" -I. Functions.c     -fo=Functions.o     || goto :err
mcc18 -p=18F4550 -I"%MCC18%\h" -I. i2c_lcd.c       -fo=i2c_lcd.o       || goto :err
mcc18 -p=18F4550 -I"%MCC18%\h" -I. SBC_Rpi.c       -fo=SBC_Rpi.o       || goto :err
mcc18 -p=18F4550 -I"%MCC18%\h" -I. service.c       -fo=service.o       || goto :err
mcc18 -p=18F4550 -I"%MCC18%\h" -I. Main_QR.c       -fo=Main_QR.o       || goto :err

REM Link (linker script includes c018i.o, clib.lib, p18f4550.lib)
mplink /l"%MCC18%\lib" rm18f4550.lkr Main_QR.o Functions.o SBC_Rpi.o i2c_lcd.o service.o /o=QR_CLEAN.hex || goto :err

echo.
echo SUCCESS: Built QR_CLEAN.hex
popd
exit /b 0

:err
echo.
echo BUILD FAILED
popd
exit /b 1
