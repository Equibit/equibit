set OLDCD=%CD%
cd ..\..\src
set XGETTEXT=C:\MinGW\bin\gettext.exe
python ..\share\qt\extract_strings_qt.py qt\intro.cpp
cd %OLDCD%
