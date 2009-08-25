%token TAGSTART TAGEND

%start file

%%

file: comments userdoc comments
    | userdoc comments
    | comments userdoc
    | userdoc
    ;

comments : comment
         | comments
         ;

userdoc: TAGSTART "userdoc" TAGEND lines TAGSTART "euserdoc" TAGEND

lines: { writeln('-'); };

%%