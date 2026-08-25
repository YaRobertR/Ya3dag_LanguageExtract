/*
  File: Test-Source.cpp

  Test file for 'Ya3dag_LanguageExtract' examples.

  NOTE: We have no C source here, but language scanning works
        also in multi line type comments.

  Usage inside code:

    printf( "%s\n", LangStringLookup( "&Tag-printf=Hello world."));

  Usage for string constants:

    char *pString = "&Tag-String=Any text for a string";

    printf( "%s\n", LangStringLookup( pString);

  Strings after '//' comment are skipped.

    // "&Tag-Comment=String after //"

  Language strings with over multiple lines.

    strcat( Temp, "&Help-Text-123="
                  "Bla bla bla line 1\n"
                  "Line 2\n"
                  "Last line");

  25.08.2036 RR: first edition.

*/

