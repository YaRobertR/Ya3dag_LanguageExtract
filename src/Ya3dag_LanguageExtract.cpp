/*
  File: Ya3dag_LanguageExtract.cpp

  Ya3dag_LanguageExtract is a command line tool to extract initial language usage
  definitions out of source files.

  Language definition: "&tag=string"
    * Must always be enclosed in double quotation marks
    * The first character must be a '&'
    * After that the tag follows. Use alphanumerics, _ or -. NO blanks.
    * A '=' the string follows.

  Longer language definitions can be spread over multiple lines:
     "&tag-long="
     "First line\n"
     "Next line"
     "Last line"

  The tag is used in the code for language translation to retrieve the translation.

  Language definitions are made in a source file by, for example, inserting the
  following language definition in place of the string “Loading ...”:

      "&GameLoading=Loading ..."

  In the code, the string above must be routed through a piece of code which
  looks up the tag 'GameLoading' and get a text "Loading ..." for it.
  This text strings are loaded language dependent and therefore it is different
  in every language.

  Strings after the characters // are skipped. For files with the extension
  '.txt' or '.cfg' also strings after the character ';' are skipped.

  Ya3dag_LanguageExtract scans all source files and collect the language definitions
  in a file. With the option '-F' for each scanned file a separate output file is created.
  This file is used as an initial language file and/or can be modified
  for translations.


    Usage: Ya3dag_LanguageExtract [-i string] [-f] [-n] output_file [files_to_add]
             -i  ignore if string is part of filename to add
             -f  output language tags after each file
             -F  output language tags after each file to a separate output file
             -n  no sorting of the language tags
             -o  print language tag optimization hints to console
             -t  print test prints to console

    Ya3dag_LanguageExtract recursively scans all subdirectories in 'files_to_add'.

    Example: Ya3dag_LanguageExtract Ya3dagInitial\Translation-Q2Tgamex86_DLL.txt D:\Robert\Quake2\SrcRRGame\*.c


  14.06.2008 RR: first edition.

  25.04.2009 RR: Added option -t for test prints.

  22.07.2025 RR: Added option -F to have one translation file per source file.

  25.07.2025 RR: Bugfix.
                 pLangTagsTextSpace[] used wrong define for size of tags and text strings.
                 Replaced LANEXTRACT_MAX_TRANSLATIONS by LANEXTRACT_MAX_TAGS_TEXT_SPACE.

  26.07.2025 RR: Added option -o to print language tag optimization hints to console

  25.08.2026 RR: Moved file to an Eclipse IDE environment and compiled there.
                 Executable is 64 bit code.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef unix
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#else
# include <direct.h>
# include <io.h>
#endif

#include "windows.h"

//----------------------------

#ifndef true
#define true 1
#define false 0
#endif // true

#define MAXFILENAME (256)

typedef struct {
  char *pTag;                 // point to tag
  char *pText;                // point to buffer
  int  DoubleTextCount;       // Count double occurrence of texts
} T_LangTranslations;

#define LANEXTRACT_MAX_TRANSLATIONS       (1024 * 16)                          // max number of translations
#define LANEXTRACT_MAX_TAGS_TEXT_SPACE    (LANEXTRACT_MAX_TRANSLATIONS * 128)  // max number of characters for tags and text strings

static T_LangTranslations pLangTranslations[ LANEXTRACT_MAX_TRANSLATIONS];     // buffer for language translations
static char pLangTagsTextSpace[ LANEXTRACT_MAX_TAGS_TEXT_SPACE];               // buffer for tags and text strings

// Size of preallocated data
static int nLangTranslationsAlloced  = LANEXTRACT_MAX_TRANSLATIONS;
static int nLangTagsTextSpaceAlloced = LANEXTRACT_MAX_TAGS_TEXT_SPACE;

// Number of used data
static int nLangTranslationsUsed;
static int nLangTagsTextSpaceUsed;

//----------------------------

#define MAX_IGNOREFILES 64

static int nIgnoreFiles;
static char *IgnoreFiles[ MAX_IGNOREFILES];

static char OutputFileName[ 512];  // Output file name

static int OpionOutputAfterEachFile = false; // 0 = off, 1 output after each file, 2 = output to a separate output file
static int OpionNoSorting           = false;
static int OpionOptimize            = false;
static int OpionTestprints          = false;

// ...

//----------------------------
// Print banner to console
//----------------------------

void do_banner()
{
  printf("Ya3dag_LanguageExtract\n");
}

//----------------------------
// Print help to console
//----------------------------

void do_help()
{
  printf( "Usage: Ya3dag_LanguageExtract [-i string] [-f] [-n] output_file [files_to_add]\n"
          "         -i  ignore if string is part of filename to add\n"
          "         -f  output language tags after each file\n"
          "         -F  output language tags after each file to a separate output file\n"
          "         -n  no sorting of the language tags\n"
          "         -o  print language tag optimization hints to console\n"
          "         -t  print test prints to console\n"
          "\n");
}

//----------------------------
// Dump language entries to a file
//----------------------------

void dump_language_entires( FILE * fout, int *pGotMultipleLines, char *pFileName,
                            int BaseThisFile, int UsedThisFile)
{
  char TempFileName[ 1024];
  int i;
  char *p, *pEndOfLine;

  if( OpionOutputAfterEachFile == 2) {     // Output to separate output file

    memset( TempFileName, 0, sizeof( TempFileName)); // Zero all

    p = strchr( OutputFileName, '.');      // Test for last point

    if( p != NULL) {                       // Have a last point

      strncpy( TempFileName, OutputFileName, p - OutputFileName);

      strcat( TempFileName, "-");          // Add delimiter
      strcat( TempFileName, pFileName);    // Add filename
      strcat( TempFileName, p);            // Add point with extension
    }

    fout = fopen( TempFileName, "wt");

    if( fout == NULL) {

      printf("Ya3dag_LanguageExtract: can't create output file '%s'\n", TempFileName);
      return;
    }
  }

  if( pFileName != NULL) {

    fprintf( fout, ";-------------------------------------------------------\n");
    fprintf( fout, "; %s\n", pFileName);
    fprintf( fout, ";-------------------------------------------------------\n");
  }

  fprintf( fout, ";\n");
  fprintf( fout, "; %d entries\n", UsedThisFile);
  fprintf( fout, ";\n");

  for( i = 0; i < UsedThisFile; i++) {

    fprintf( fout, "%-34s ", pLangTranslations[ BaseThisFile + i].pTag);

    p = pLangTranslations[ BaseThisFile + i].pText;

    for( ; ; ) {

      pEndOfLine = strchr( p, '\n');   // Test for multi line strings

      if( pEndOfLine != NULL) {

        *pEndOfLine = '\0';            // Set end of string

        fprintf( fout, "\"%s\"\n", p);

        *pEndOfLine = '\n';            // Restore end of line

        p = pEndOfLine + 1;

        fprintf( fout, "%-34s ", "");
      } else {

        fprintf( fout, "\"%s\"\n", p);
        break;
      }
    }

    if( *pGotMultipleLines > 0) {     // if we have any multiple lines
      fprintf( fout, "\n");           // make an empty line between definitions
    }
  }

  fprintf( fout, ";\n");

  if( OpionOutputAfterEachFile == 2) {     // Output to separate output file

    fprintf( fout, ";\n");
    fprintf( fout, "; End Of File\n");

    fclose( fout);
  }
}

//----------------------------
// Sort language strings by tags
//----------------------------

static int LangTranslationCompareByTags( const void *arg1, const void *arg2 )
{
  T_LangTranslations *p1;
  T_LangTranslations *p2;

  p1 = (T_LangTranslations *)arg1;
  p2 = (T_LangTranslations *)arg2;

  return strcmp( p1->pTag, p2->pTag);
}

//----------------------------
// Sort language strings by text
//----------------------------

static int LangTranslationCompareByText( const void *arg1, const void *arg2 )
{
  T_LangTranslations *p1;
  T_LangTranslations *p2;

  p1 = (T_LangTranslations *)arg1;
  p2 = (T_LangTranslations *)arg2;

  return strcmp( p1->pText, p2->pText);
}

//----------------------------
// Sort language strings by DoubleTextCount and by text
//----------------------------

static int LangTranslationCompareByDoubleTextCount( const void *arg1, const void *arg2 )
{
  T_LangTranslations *p1;
  T_LangTranslations *p2;

  p1 = (T_LangTranslations *)arg1;
  p2 = (T_LangTranslations *)arg2;

  if( p1->DoubleTextCount == p2->DoubleTextCount) {   // Double text count is same

    return strcmp( p1->pText, p2->pText);             // sort by text
  }

  return( p2->DoubleTextCount - p1->DoubleTextCount); // Sort by DoubleTextCount
}

//----------------------------
// Scan a file for language strings
//----------------------------

static int AddFile( char *pFilename, char *pBaseFileName, int *pnFiles,
                    int *pnLangStrings, int *pnWarnings, int *pGotMultipleLines)
{
  FILE * fin;
  int i, LineNr, IsScriptSource, nTempText;
  char filename_try[MAXFILENAME+16];
  char TempString[ 1024];
  char TempTag[ 1024];
  char TempText[ 1024 * 5];
  char *pSrc, *pTag, *pText, *pDstText;

  // test ignore files

  strcpy( filename_try, pFilename);
  _strlwr( filename_try);

  for( i = 0; i < nIgnoreFiles; i++) {

    if( strstr( filename_try, IgnoreFiles[ i]) != NULL) {   // if is in ignore list
      return( 0);                                           // nothing to do, return OK
    }
  }

  // test for script file

  IsScriptSource = false;                     // default

  i = strlen( pFilename);

  if( i > 4 && pFilename[ i - 4]  == '.' &&

      ((tolower( pFilename[ i - 3]) == tolower( 't') &&
       tolower( pFilename[ i - 2]) == tolower( 'x') &&
       tolower( pFilename[ i - 1]) == tolower( 't')) ||
      (tolower( pFilename[ i - 3]) == tolower( 'c') &&
       tolower( pFilename[ i - 2]) == tolower( 'f') &&
       tolower( pFilename[ i - 1]) == tolower( 'g')))) {

    IsScriptSource = true;                   // a text file is always a script file
  }

  //printf("AddFile: %s...\n", pFilename);

  fin = fopen( pFilename, "rt");
  if (fin==NULL)
  {

    printf("ERROR in opening %s for reading\n", pFilename);
    return( -1);;
  }

  *pnFiles += 1;

  // loop over text lines

  LineNr = 0;

  for( ; ; ) {

TryNextLine:

    if( fgets( TempString, sizeof( TempString), fin) == NULL) {
      break; // end of file
    }

    LineNr += 1;

    pSrc = TempString;

    // find begin of language string

TryNextInLine:

    for( ; ; ) {

      if( *pSrc == '\0' || *pSrc == '\n') {          // end of line
        goto TryNextLine;
      }

      if( pSrc[ 0] == '/' && pSrc[ 1] == '/') {      // c style commend line
        goto TryNextLine;
      }

      if( IsScriptSource) {                          // is script source

        if( *pSrc == ';') {                          // actor script commend line
          goto TryNextLine;
        }
      }

      if( pSrc[ 0] == '"' && pSrc[ 1] == '&') {      // begin of language string

        pSrc++;
        pSrc++;
        break;
      }

      pSrc++;
    }

    // begin of language string, pSrc points to tag
    // find begin of text

    pTag = pSrc;

    for( ; ; ) {

      if( *pSrc == '\0' || *pSrc == '\n') {          // end of line
        goto TryNextLine;
      }

      if( *pSrc == '"') {                            // double quote is end of string (Thats not OK here)
        goto TryNextLine;
      }

      if( pSrc[ 0] == '=') {                         // assing is begin of tag

        *pSrc = '\0';                                // end of tag

        pSrc++;                                      // point after
        break;
      }

      pSrc++;
    }

    // begin of language string, pSrc points to text
    // find end of text

    pText = pSrc;

    for( ; ; ) {

      if( *pSrc == '\0' || *pSrc == '\n') {          // end of line
        goto TryNextLine;
      }

      if( *pSrc == '"') {                            // double quote is end of string

        *pSrc = '\0';                                // end of tag

        pSrc++;                                      // point after
        break;
      }

      pSrc++;
    }

    // If the assign part is empty, get all following quoted strings.
    // NOTE: There may be no comments here

    if( strlen( pText) == 0) {

      strcpy( TempTag, pTag);
      pTag = TempTag;

      memset( TempText, 0, sizeof( TempText));

      pText     = TempText;            // data is stored here now
      pDstText  = TempText;
      nTempText = 0;

      for( ; ; ) {

        if( fgets( TempString, sizeof( TempString), fin) == NULL) {
          goto EndOfFile; // end of file
        }

        LineNr += 1;

        pSrc = TempString;

        // find beginning quote

        for( ; ; ) {

          if( *pSrc == '\0' || *pSrc == '\n') {         // end of line
            goto GotTextLines;                          // something not OK, continue elsewhere
          }

          if( *pSrc == '"') {                           // begin of quoted string

            pSrc++;
            break;
          }

          if( *pSrc > ' ') {                            // none blank
            goto GotTextLines;                          // something not OK, continue elsewhere
          }

          pSrc++;
        }

        // find closing quote

        if( pDstText != TempText) {                     // if not first line
          if( nTempText < (int)sizeof( TempString) - 2) {
            *pDstText++ = '\n';                           // enter linefeed
            nTempText += 1;
          }
        }

        for( ; ; ) {

          if( *pSrc == '\0' || *pSrc == '\n') {         // end of line
            goto GotTextLines;                          // something not OK, continue elsewhere
          }

          if( *pSrc == '"') {                           // end of quoted string

            pSrc++;
            break;
          }

          if( nTempText < (int)sizeof( TempString) - 2) {
            *pDstText++ = *pSrc;                        // store this character
            nTempText += 1;
          }

          pSrc++;
        }

        *pGotMultipleLines += 1;
      }

GotTextLines:
      *pDstText++ = '\0';                                // enter of string

    }

    // got it

    if( strlen( pTag) > 0 && strlen( pText) > 0) {


      // test for already existing

      for( i = 0; i < nLangTranslationsUsed; i++) {

        if( strcmp( pTag, pLangTranslations[ i].pTag) == 0) {        // already have it in the database


          if( strcmp( pText, pLangTranslations[ i].pText) != 0) {    // same tag, but tag is different, something is not OK

            *pnWarnings += 1;         // count warnings

            printf( "**** Tag defined elsewhere with different text '%s' Line %4d\n", pBaseFileName, LineNr);
            printf( "     \"&%s=%s\"\n", pTag, pText);
          }

          goto TryNextInLine;
        }
      }


      // store it

      *pnLangStrings += 1;


      if( nLangTranslationsUsed + 1 >= nLangTranslationsAlloced) {

        printf("ERROR overflow of max number of translations (%d)\n", LANEXTRACT_MAX_TRANSLATIONS);
        return( -1);;
      }


      if( nLangTagsTextSpaceUsed + (int)strlen( pTag) + 1 + (int)strlen( pText) + 1 >= nLangTagsTextSpaceAlloced) {

        printf("ERROR overflow of space for tag/text string (%d)\n", LANEXTRACT_MAX_TAGS_TEXT_SPACE);
        return( -1);;
      }


      pLangTranslations[ nLangTranslationsUsed].pTag = pLangTagsTextSpace + nLangTagsTextSpaceUsed;

      strcpy( pLangTagsTextSpace + nLangTagsTextSpaceUsed, pTag);
      nLangTagsTextSpaceUsed += strlen( pTag) + 1;

      pLangTranslations[ nLangTranslationsUsed].pText = pLangTagsTextSpace + nLangTagsTextSpaceUsed;

      strcpy( pLangTagsTextSpace + nLangTagsTextSpaceUsed, pText);
      nLangTagsTextSpaceUsed += strlen( pText) + 1;

      nLangTranslationsUsed += 1;
    }

    // try next in line

    goto TryNextInLine;

  }


  // ...
EndOfFile:

  fclose( fin);

  return( 0);      // return OK
}

//----------------------------
// Scan a directory for files to inspect
//----------------------------

static int AddFileRecursive( char *pDir, char *pFilemask, int *pnFiles,
                             FILE * fout, int *pnLangStrings, int *pnWarnings, int *pGotMultipleLines)
{
  HANDLE hFind;
  WIN32_FIND_DATA  ff32;
  char Filename[ 512];
  int ierr, LangTranslationsBaseThisFile, LangTranslationsUsedThisFile;

  // find with wildchard first

  strcpy( Filename, pDir);                        // merge directory part
  if( pDir[ 0] != (char)'\0') {
    strcat( Filename, "\\");                      // add path delimitter
  }
  strcat( Filename, pFilemask);                   // with filename

  if( OpionTestprints) {

    printf("Scan for files %s ....\n", Filename);
  }

  hFind = FindFirstFile( Filename, &ff32);

  if (hFind != INVALID_HANDLE_VALUE)
  {

    for( ; ; ) {

      if( ff32.cFileName[ 0] == '.') {           // skip current/parent directory
        goto SkipAddFile1;
      }

      if( ff32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {     // a directory

        goto SkipAddFile1;
      }

      strcpy( Filename, pDir);                   // merge directory part
      if( pDir[ 0] != (char)'\0') {
        strcat( Filename, "\\");                 // add path delimitter
      }
      strcat( Filename, ff32.cFileName);         // with filename

      if( OpionTestprints) {

        printf("File %s --> %s\n", ff32.cFileName, Filename);
      }

      LangTranslationsBaseThisFile = nLangTranslationsUsed;

      ierr = AddFile( Filename, ff32.cFileName, pnFiles, pnLangStrings, pnWarnings, pGotMultipleLines);

      LangTranslationsUsedThisFile = nLangTranslationsUsed - LangTranslationsBaseThisFile;

      if( OpionTestprints) {

        printf("  Base %4d   Used %3d\n", LangTranslationsBaseThisFile, LangTranslationsUsedThisFile);
      }

      //

      if( OpionOutputAfterEachFile) {                  // output after each scanned file
        if( LangTranslationsUsedThisFile > 0) {        // does we have any data ?

          // sort the entries by tag

          if( ! OpionNoSorting) {

            qsort( (void *)(pLangTranslations + LangTranslationsBaseThisFile),
                   (size_t)LangTranslationsUsedThisFile, sizeof( T_LangTranslations), LangTranslationCompareByTags);
          }

          // dump data


          dump_language_entires( fout, pGotMultipleLines, ff32.cFileName, LangTranslationsBaseThisFile, LangTranslationsUsedThisFile);

          // reset the data
          *pGotMultipleLines = 0;
        }
      }

      //

      if( ierr != 0) {     // got error
        return( ierr);     // return error
      }


SkipAddFile1:

      if( FindNextFile( hFind, &ff32) == 0) {
        break;
      }
    }
    FindClose(hFind);

  } else {

    if( OpionTestprints) {

      printf("---- no files for %s !!!\n", Filename);
    }

  }

  // scan sup directories

  strcpy( Filename, pDir);                        // merge directory part
  if( pDir[ 0] != (char)'\0') {
    strcat( Filename, "\\");                      // add path delimitter
  }
  strcat( Filename, "*");                         // in subdirectories

  if( OpionTestprints) {

    printf("Scan for directories %s ....\n", Filename);
  }

  hFind = FindFirstFile( Filename, &ff32);

  if (hFind != INVALID_HANDLE_VALUE)
  {

    for( ; ; ) {

      if( ff32.cFileName[ 0] == '.') {           // skip current/parent directory
        goto SkipAddFile2;
      }

      if( ff32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {     // a directory

        char dir[ 512];

        // recurisve walk the directories

        strcpy( dir, pDir);
        if( pDir[ 0] != (char)'\0') {
          strcat( dir, "\\");                    // add path delimitter
        }
        strcat( dir, ff32.cFileName);            // with filename

        if( OpionTestprints) {

          printf("==============================\n");
          printf("Directory %s --> %s\n", ff32.cFileName, dir);
        }

        ierr = AddFileRecursive( dir, pFilemask, pnFiles, fout, pnLangStrings, pnWarnings, pGotMultipleLines);
        if( ierr != 0) {     // got error
          return( ierr);     // return error
        }

        if( OpionTestprints) {

          printf("Done directory %s --> %s\n", ff32.cFileName, dir);
          printf("==============================\n");
        }

        goto SkipAddFile2;
      }


SkipAddFile2:

      if( FindNextFile( hFind, &ff32) == 0) {
        break;
      }
    }
    FindClose(hFind);

  } else {

    if( OpionTestprints) {

      printf("---- no directories for %s !!!\n", Filename);
    }

  }

  return( 0);
}

//----------------------------
// Main function
//----------------------------

int main(int argc, char* argv[])
{
  int OutputFileArg = 0;
  int i, ierr, nWarnings, GotMultipleLines;
  FILE * fout = NULL;
  char *p, *pEndOfLine;
  T_LangTranslations *pEntryFirst, *pEntryThis;

  //----------------------------
  // Parse arguments
  //----------------------------

  if (argc==1)
  {
    do_banner();
    do_help();
    return 0;
  }
  else
  {
    for (i=1;i<argc;i++)
    {
      if ((*argv[i])=='-')
      {
        const char *p=argv[i]+1;

        while ((*p)!='\0')
        {
          char c=*(p++);

          if (c=='f') {                 // output language tags after each file

            OpionOutputAfterEachFile = true;

          } else if (c=='F') {         // output language tags after each file to a separate output file

            OpionOutputAfterEachFile = 2;

          } else if ((c=='n') || (c=='N')) {

            OpionNoSorting = true;


          } else if ((c=='t') || (c=='T')) {

            OpionTestprints = true;

          } else if ((c=='o') || (c=='O')) {

            OpionOptimize = true;

          } else if (((c=='i') || (c=='i')) && (i+1<argc)) {

            if( nIgnoreFiles < MAX_IGNOREFILES) {
              IgnoreFiles[ nIgnoreFiles] = argv[i+1];
              _strlwr( IgnoreFiles[ nIgnoreFiles]);
              nIgnoreFiles++;
            }
            i++;
          }
        }
      }
      else
        if (OutputFileArg == 0)
          OutputFileArg = i ;
    }
  }

  if( OutputFileArg == 0) {         // no arguments
    do_banner();
    do_help();
    return 0;
  }

  //----------------------------
  // Prepare output file
  //----------------------------

  strcpy( OutputFileName, argv[OutputFileArg]);     // Remember output file name

  if( OpionOutputAfterEachFile != 2) {

    fout = fopen( argv[OutputFileArg], "wt");

    if( fout == NULL) {

      printf("Ya3dag_LanguageExtract: can't create output file '%s'\n", argv[OutputFileArg]);
      return( -1);
    }

    // output file header

    fprintf( fout, ";\n");
    p = strrchr( argv[OutputFileArg], '/');
    if( p == NULL) {
      p = strrchr( argv[OutputFileArg], '\\');
    }
    if( p != NULL) {
      fprintf( fout, "; %s\n", p + 1);
    } else {
      fprintf( fout, "; %s\n", argv[OutputFileArg]);
    }
    fprintf( fout, ";\n");
  }

  //----------------------------
  // Scan ...
  //----------------------------

  nLangTranslationsUsed  = 0;
  nLangTagsTextSpaceUsed = 0;

  nWarnings        = 0;
  GotMultipleLines = 0;

  for( i = OutputFileArg + 1; i < argc; i++) {

    if( argv[i][ 0] != '-') {

      char dir[ 512];
      char filemask[ 512];
      char *pPathDelimiter;
      int nFiles, nLangStrings;

      strcpy( dir, argv[i]);
      pPathDelimiter = strrchr( dir, '\\');          // find last backslash
      if( pPathDelimiter == NULL) {
        pPathDelimiter = strrchr( dir, '\\');        // find last slash
      }

      if( pPathDelimiter == NULL) {
        strcpy( filemask, dir);                      // filemask for search
        dir[ 0] = '\0';                              // have no path
      } else {
        strcpy( filemask, pPathDelimiter + 1);       // filemask for search
        *pPathDelimiter = '\0';                      // cat off add end of path
      }

      nFiles = 0;
      nLangStrings = 0;

      printf("add from %s ...\n", argv[i]);

      ierr = AddFileRecursive( dir, filemask, &nFiles, fout, &nLangStrings, &nWarnings, &GotMultipleLines);
      if( ierr != 0) {   // got error
        goto ExitPoint;
      }

      printf("   %d files, %d LangStrings\n", nFiles, nLangStrings);
    }
  }

  if( nWarnings > 0) {

    printf("\n");
    printf("  WARNING, %d Tag's defined elsewhere with different text!\n", nWarnings);
    printf("\n");
  }

  if( OpionOutputAfterEachFile == 0) {

    // sort all entries by tag

    if( ! OpionNoSorting) {

      qsort( (void *)pLangTranslations, (size_t)nLangTranslationsUsed, sizeof( T_LangTranslations), LangTranslationCompareByTags);
    }

    // dump data

    dump_language_entires( fout, &GotMultipleLines, NULL, 0, nLangTranslationsUsed);
  }

  if( OpionOutputAfterEachFile != 2) {

    fprintf( fout, ";\n");
    fprintf( fout, "; End Of File\n");
  }

  // Close output file

  if( fout != NULL) {

    fclose( fout);
    fout = NULL;
  }

  //----------------------------
  // Check for language tag optimization hints
  //----------------------------

  if( ! OpionOptimize) {    // NO optimizations

    goto ExitPoint;
  }

  // Sort all language strings by text

  qsort( (void *)pLangTranslations, (size_t)nLangTranslationsUsed, sizeof( T_LangTranslations), LangTranslationCompareByText);

  // Reset double text count

  pEntryThis = pLangTranslations;

  for( i = 0; i < nLangTranslationsUsed; i++, pEntryThis++) {

    pEntryThis->DoubleTextCount = 0;
  }

  // Check for double occurrence of texts

  pEntryThis = pLangTranslations;
  pEntryFirst = NULL;

  for( i = 0; i < nLangTranslationsUsed; i++, pEntryThis++) {

    if( pEntryFirst == NULL ||                                 // First entry
        strcmp( pEntryFirst->pText, pEntryThis->pText) != 0) { // or different text

      pEntryFirst = pEntryThis;
      pEntryFirst->DoubleTextCount = 1;

    } else {                                                   // Same text

      pEntryFirst->DoubleTextCount += 1;   // Count double
      pEntryThis->DoubleTextCount = -1;    // This is double used
    }
  }

  // Sort all language strings by DoubleTextCount and by text

  qsort( (void *)pLangTranslations, (size_t)nLangTranslationsUsed, sizeof( T_LangTranslations), LangTranslationCompareByDoubleTextCount);

  // Dump double usage to console

  printf("\n");
  printf("-------------------------------------\n");
  printf("Language strings with different tags:\n");
  printf("\n");

  pEntryThis = pLangTranslations;

  for( i = 0; i < nLangTranslationsUsed; i++, pEntryThis++) {

    if( pEntryThis->DoubleTextCount <= 1) {    // No double use

      continue;
    }

    printf("  %3d x =", pEntryThis->DoubleTextCount);

    p = pEntryThis->pText;

    for( ; ; ) {

      pEndOfLine = strchr( p, '\n');   // Test for multi line strings

      if( pEndOfLine != NULL) {        // Is not the last multi line string

        *pEndOfLine = '\0';            // Set end of string

        if( p == pEntryThis->pText) {  // Is first multi line string

          printf( "\n");
        }

        printf( "        \"%s\"\n", p);

        *pEndOfLine = '\n';            // Restore end of line

        p = pEndOfLine + 1;

      } else {

        if( p == pEntryThis->pText) {  // Is a single line string

          printf( "%s\"\n", p);

        } else {                       // Is last multi line string

          printf( "        \"%s\"\n", p);
        }
        break;
      }
    }
  }

  printf( "\n");

  // ...

ExitPoint:

  if( fout != NULL) {

    fclose( fout);
    fout = NULL;
  }

	return 0;
}

// ------------------------ End Of File --------------------------------
