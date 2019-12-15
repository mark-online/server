REM @echo off
REM
REM Build & release ClientNet
REM

set VERSION=%1

pushd .
if "%VERSION%" == "" goto usage

if "%SNE_DIR%" == "" goto error_env
if "%SNE_DEPENDENT_DIR%" == "" goto error_env

:setvar
setlocal
set TARGET_NAME=gideon_clientnet_%VERSION%
call detail\set_properties.bat

set GIDEON_DIR=..\..

set REL_BASE_DIR=release
set REL_DIR=%REL_BASE_DIR%\%TARGET_NAME%
set SNE_INC_DIR=include\sne
set GIDEON_INC_DIR=include\gideon
set RESOURCE_DIR=resource
set SNECLIENT_DIR=src\sne
set CLIENTNET_SRC_DIR=src\clientnet
set BOT_SRC_DIR=test\bot
set ACE_DIR=%SNE_DEPENDENT_DIR%\ACE_wrappers
set XERCESC_DIR=%SNE_DEPENDENT_DIR%\xerces-c

:makedirs
mkdir %REL_BASE_DIR%
mkdir %REL_DIR%
mkdir %REL_DIR%\bin
mkdir %REL_DIR%\include
mkdir %REL_DIR%\%SNE_INC_DIR%
mkdir %REL_DIR%\%SNE_INC_DIR%\base
mkdir %REL_DIR%\%SNE_INC_DIR%\client
mkdir %REL_DIR%\%SNE_INC_DIR%\common
mkdir %REL_DIR%\%SNE_INC_DIR%\core
mkdir %REL_DIR%\%SNE_INC_DIR%\security
mkdir %REL_DIR%\%SNE_INC_DIR%\sgp
mkdir %REL_DIR%\%SNE_INC_DIR%\srpc
mkdir %REL_DIR%\%GIDEON_INC_DIR%
mkdir %REL_DIR%\%GIDEON_INC_DIR%\clientnet
mkdir %REL_DIR%\%GIDEON_INC_DIR%\common
mkdir %REL_DIR%\lib
mkdir %REL_DIR%\src
mkdir %REL_DIR%\src\sne
mkdir %REL_DIR%\%SNECLIENT_DIR%
mkdir %REL_DIR%\src\gideon
mkdir %REL_DIR%\%CLIENTNET_SRC_DIR%
mkdir %REL_DIR%\%BOT_SRC_DIR%
REM mkdir %REL_DIR%\%ACE_DIR%

echo copy_sne_inc
copy /Y %SNE_DIR%\%SNE_INC_DIR%\*.h %REL_DIR%\%SNE_INC_DIR%
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\base\*.h %REL_DIR%\%SNE_INC_DIR%\base
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\client\*.h %REL_DIR%\%SNE_INC_DIR%\client
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\common\*.h %REL_DIR%\%SNE_INC_DIR%\common
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\core\*.h %REL_DIR%\%SNE_INC_DIR%\core
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\security\*.h %REL_DIR%\%SNE_INC_DIR%\security
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\sgp\*.h %REL_DIR%\%SNE_INC_DIR%\sgp
xcopy /S /Y %SNE_DIR%\%SNE_INC_DIR%\srpc\*.h %REL_DIR%\%SNE_INC_DIR%\srpc

echo copy_gideon_inc
copy /Y %GIDEON_DIR%\%GIDEON_INC_DIR%\*.h %REL_DIR%\%GIDEON_INC_DIR%
xcopy /S /Y %GIDEON_DIR%\%GIDEON_INC_DIR%\clientnet\*.h %REL_DIR%\%GIDEON_INC_DIR%\clientnet
xcopy /S /Y %GIDEON_DIR%\%GIDEON_INC_DIR%\common\*.h %REL_DIR%\%GIDEON_INC_DIR%\common

echo copy_sne_dll
copy /Y %SNE_DIR%\bin\Win32\sne.base.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.base.debug.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.client.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.client.debug.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.core.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.core.debug.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.security.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.security.debug.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.sgp.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.sgp.debug.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.srpc.dll %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.srpc.debug.dll %REL_DIR%\bin

echo copy_sne_pdb
copy /Y %SNE_DIR%\bin\Win32\sne.base.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.base.debug.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.client.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.client.debug.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.core.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.core.debug.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.security.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.security.debug.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.sgp.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.sgp.debug.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.srpc.pdb %REL_DIR%\bin
copy /Y %SNE_DIR%\bin\Win32\sne.srpc.debug.pdb %REL_DIR%\bin

echo copy_sne_lib
copy /Y %SNE_DIR%\lib\Win32\sne.base.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.base.debug.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.client.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.client.debug.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.core.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.core.debug.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.security.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.security.debug.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.sgp.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.sgp.debug.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.srpc.lib %REL_DIR%\lib
copy /Y %SNE_DIR%\lib\Win32\sne.srpc.debug.lib %REL_DIR%\lib

echo copy_clientnet_dll
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.dll %REL_DIR%\bin
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.debug.dll %REL_DIR%\bin

echo copy_clientnet_pdb
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.pdb %REL_DIR%\bin
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.debug.pdb %REL_DIR%\bin

echo copy_clientnet_lib
copy /Y %GIDEON_DIR%\lib\Win32\clientnet.lib %REL_DIR%\lib
copy /Y %GIDEON_DIR%\lib\Win32\clientnet.debug.lib %REL_DIR%\lib

REM echo copy_clientnet_static_mt_lib
copy /Y %GIDEON_DIR%\lib\Win32\clientnet.static.mt.lib %REL_DIR%\lib
REM copy /Y %GIDEON_DIR%\lib\Win32\clientnet.static.mt.debug.lib %REL_DIR%\lib

echo copy_ace_dll
copy /Y %ACE_DIR%\bin\Win32\ACE.dll %REL_DIR%\bin
copy /Y %ACE_DIR%\bin\Win32\ACEd.dll %REL_DIR%\bin

echo copy_ace_pdb
copy /Y %ACE_DIR%\bin\Win32\ACE.pdb %REL_DIR%\bin
copy /Y %ACE_DIR%\bin\Win32\ACEd.pdb %REL_DIR%\bin

REM echo copy_ace_lib
REM copy /Y %ACE_DIR%\lib\Win32\ACE.lib %REL_DIR%\lib
REM copy /Y %ACE_DIR%\lib\Win32\ACEd.lib %REL_DIR%\lib

echo copy_xerces-c_dll
copy /Y %XERCESC_DIR%\bin\Win32\xerces-c_3_1.dll %REL_DIR%\bin
copy /Y %XERCESC_DIR%\bin\Win32\xerces-c_3_1D.dll %REL_DIR%\bin

echo copy_xerces-c_pdb
REM copy /Y %XERCESC_DIR%\bin\Win32\xerces-c_3_1.pdb %REL_DIR%\bin
copy /Y %XERCESC_DIR%\bin\Win32\xerces-c_3_1D.pdb %REL_DIR%\bin

echo copy_xerces-c_lib
copy /Y %XERCESC_DIR%\lib\Win32\xerces-c_3.lib %REL_DIR%\lib
copy /Y %XERCESC_DIR%\lib\Win32\xerces-c_3D.lib %REL_DIR%\lib

REM echo copy_clientnet_src
REM copy /Y %GIDEON_DIR%\%CLIENTNET_SRC_DIR% %REL_DIR%\%CLIENTNET_SRC_DIR%

REM echo copy_sne_src
REM copy /Y %SNE_DIR%\%SNECLIENT_DIR%\*.h %REL_DIR%\%SNECLIENT_DIR%

echo copy_bot_src
copy /Y %GIDEON_DIR%\%BOT_SRC_DIR%\*.h %REL_DIR%\%BOT_SRC_DIR%
copy /Y %GIDEON_DIR%\%BOT_SRC_DIR%\*.cpp %REL_DIR%\%BOT_SRC_DIR%
copy /Y %GIDEON_DIR%\%BOT_SRC_DIR%\*.vcproj %REL_DIR%\%BOT_SRC_DIR%
copy /Y %GIDEON_DIR%\%BOT_SRC_DIR%\*.sln %REL_DIR%\%BOT_SRC_DIR%

REM echo copy_ace_lib
REM copy /Y %ACE_ROOT%\lib\aces.lib %REL_DIR%\lib
REM copy /Y %ACE_ROOT%\lib\acesd.lib %REL_DIR%\lib

echo zip
cd %REL_BASE_DIR%
c:\cygwin\bin\zip -9 -r %TARGET_NAME%.zip %TARGET_NAME%
if errorlevel 1 goto done
goto done

echo remove_dirs
c:\cygwin\bin\rmdir /s /q %TARGET_NAME%

goto done

:usage
echo "USAGE: %0 <version>"
goto done

:error_env
echo "SNE_DIR or SNE_DEPENDENT_DIR environment variable is not defined!"

:done
popd

if "%2"=="nowait" goto end
pause

:end
