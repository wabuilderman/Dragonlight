:: File        : Shader Compilation Script
:: Author      : Joel Hemphill
:: Date        : 11-30-18
:: Description : Compiles all HLSL scripts in the current directory, and sends
::               the results to a designated folder.

:: ================================ MAIN ==================================== ::
  @ECHO OFF
  SETLOCAL
  
  CALL :Main
  
  ECHO.
  IF NOT "%0"=="%~nx0" (
    PAUSE
  )
  ENDLOCAL
  ECHO ON
@EXIT /B 0

:: ============================= FUNCTIONS ================================== ::
:Main
  SET compiler="C:/VulkanSDK/1.1.85.0/Bin32/glslangValidator.exe"
  SET outDir="./Build"

  echo Compiling Shader: "shader"
  call :Compile "shader"
@EXIT /B 0

:Compile
  %compiler% -V %1.vert -o %outDir%/%1_vert.spv
  %compiler% -V %1.frag -o %outDir%/%1_frag.spv
@EXIT /B 0