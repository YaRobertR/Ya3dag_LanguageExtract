/****************************************************************************

  Language-Translate.h

  Example handling header.

  25.08.2026 RR: First edition of this file.

*****************************************************************************
*/

#ifndef LANGUAGE_TRANSLATE_H_
#define LANGUAGE_TRANSLATE_H_

/*===========================================================================

  Lang_FreeData()

  Free up all the memory allocated for the most
  recently loaded language data.

  Return:

    None

=============================================================================
*/

void Lang_FreeData();

/*===========================================================================

  LangLoadTranslations()

  Load the language data for a specific language

  Arguments:

    char *pLanguage        Name of the directory containing the language data

    int NewLineCharForMultipleStringLines
                           If set, insert new line characters at end of multiple string lines.
                                Example: tag_xyz  "Line 1"
                                "Line 2"
                                "Line 3"

  Return:

    0      OK
    != 0   Error

=============================================================================
*/

int LangLoadTranslations( char *pLanguage, int NewLineCharForMultipleStringLines = false);

/*===========================================================================

  Lang_Init()

  Load the language files for the last selected language.
  Use this after starting your application.

  Arguments:

    int NewLineCharForMultipleStringLines
                           If set, insert new line characters at end of multiple string lines.
                                Example: tag_xyz  "Line 1"
                                "Line 2"
                                "Line 3"

  NOTE: The last language used is stored in a global variable

  Return:

    None

=============================================================================
*/

void Lang_Init( int NewLineCharForMultipleStringLines = false);

/*===========================================================================

  LangStringLookup()

  Look up a language definition.

  Arguments:

    char *pString       Look up this language definition.
                        It should be in the format "&tag=string".

  Return:

    NULL                This only happens if the 'pString' argument is 'NULL'.

    pointer to string   * If the first character does not start with '&',
                          the 'pString' argument string will be returned.
                        * If a translation is found, this will be returned.
                        * If no translation is found, the text following the '='
                          character in the 'pString' argument will be returned.

  NOTE: The translations remain valid as long as no language data is loaded.
        This means that after loading language data, the application must be
        closed and restarted. For example, after selecting a new language.

=============================================================================
*/

char * LangStringLookup( char *pString);
char * LangStringLookup( const char *pString);

#endif // LANGUAGE_TRANSLATE_H_

/****************************** End Of File ******************************/
