@ECHO OFF
SET CHERE_INVOKING=1
IF EXIST C:\msys64\usr\bin\bash.exe (
    echo Starting msys2 bash
    C:\msys64\usr\bin\bash.exe --login
    GOTO :eof
)
IF EXIST C:\cygwin64\bin\bash.exe (
     echo Starting cygwin bash
     C:\cygwin64\bin\bash.exe --login
     GOTO :eof
) ELSE (
    echo Starting git windows bash
    "C:\Program Files\Git\bin\bash.exe"
)
