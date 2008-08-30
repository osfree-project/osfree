uses
  Classes, SysUtils, Lexlib, YaccLib;


{$I yacc.pas}
{$I lex.pas}

begin
  Assign(yyinput, paramstr(1));
  Reset(yyinput);
  assign(yyoutput, 'test');
  rewrite(yyoutput);
  yyparse;
//  Close(yyinput);
//  Close(yyoutput);
end.