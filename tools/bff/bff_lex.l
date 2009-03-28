%{
#include <bff.h>
%}


%%

DEFINITION { return(DEFINITION); }
FORMAT      { return(FORMAT); }
END             { return(END); }
ADDRESS { return(ADDRESS); }
FILEHEADER { return(FILEHEADER); }
IMAGESIZE { return(IMAGESIZE); }
IMAGEADDRESS { return(IMAGEADDRESS); }
SIZE   { return(SIZE); }
ARRAY { return(ARRAY); }
[A-Za-z][A-Za-z0-9_]* { yylval=strdup(yytext);return(IDENT); }
[0-9]+   {yylval=strdup(yytext); return(DIGIT);}
[ \t\n]+ { ; }
"("   {return (LB);}
")"    {return(RB);}
"+"   {return(PLUS);}
"-"   {return(MINUS);}
"*"   {return(MUL);}
"/"   {return(DIV);}

%% 

int yywrap() { return 1; /* Конец файла - завершение */ } 
