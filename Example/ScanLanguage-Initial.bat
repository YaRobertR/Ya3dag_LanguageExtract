@echo off
rem File: ScanLanguage-Initial.bat
rem
rem Example to extract language strings.
rem Extract language transtlations to folder Ya3dagInitial.
rem This are initial translation files for the game Ya3dag.
rem
rem
rem 25.08.2036 RR: first edition.
rem
rem
@echo on

rem make subdirecory for language scan

mkdir TempDir

rem Usecase 1: Scan output into one output file

Ya3dag_LanguageExtract -f -n TempDir\OneOutputFile.txt Test-Src\*.txt Test-Src\*.h Test-Src\*.cpp

rem Usecase 1: An output file for each scanned file

Ya3dag_LanguageExtract -F -n TempDir\Translation.txt Test-Src\*.txt Test-Src\*.h Test-Src\*.cpp

rem User can inspect output of the tools

pause
