/****************************************************************************

  Language-Translate.h

  Example handling header.

  25.08.2026 RR: First edition of this file.

*****************************************************************************
*/

#ifndef LANGUAGE_TRANSLATE_H_
#define LANGUAGE_TRANSLATE_H_

//   NewLineCharForMultipleStringLines:
//      If set, insert new line characters at end of multiple string lines.
//        Example: tag_xyz  "Line 1"
//                          "Line 2"
//                          "Line 3"

// Free language data
void Lang_FreeData();

// Load translations for a specific language
int LangLoadTranslations( char *pLanguage, int NewLineCharForMultipleStringLines = false);

// Load translations for last selected language
void Lang_Init( int NewLineCharForMultipleStringLines = false);

// Lookup a language string
char * LangStringLookup( char *pString);
char * LangStringLookup( const char *pString);

#endif // LANGUAGE_TRANSLATE_H_

/****************************** End Of File ******************************/
