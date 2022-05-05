#
# text in Portuguese (Brazil) without accents (diacritics)
#
# est  = estah - as opposed to esta (this)
# ‚ = eh - as opposed to e (and)
# m  = mah
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#
# Regina error messages - English
# Written by Mark Hessling <M.Hessling@qut.edu.au>
#
# portuguese - translation by Susana Carpenter,
#              with assist by uni-lingual C>nerd husband (brian)
#            - additional messages by Josie Medieros
#
  0,  1,Erro %s ao executar %s, linha %d:|<value>,<source>,<linenumber>
  0,  2,Erro %s na investigacao interativa:|<value>
  0,  3,investigacao interativa.  "Trace Off" para finalizar depuracao. ENTER para continuar.
  2,  0,Falha durante finalizacao
  2,  1,Falha durante finalizacao: %s|<description>
  3,  0,Falha durante inicializacao
  3,  1,Falha durante inicializacao: %s|<description>
  4,  0,Programa interrompido
  4,  1,Programa interrompido com condicao HALT: %s|<description>
  5,  0,Recursos do sistema esgotados
  5,  1,Recursos do sistema esgotados: %s|<description>
  6,  0,"/*" Nao-correspondente ou aspas
  6,  1,delimitador de comentarios nao-correspondente ("/*")
  6,  2,apostrofe nao-correspondente (')
  6,  3,aspas nao-correspondentes (")
  7,  0,WHEN ou OTHERWISE sao necessarios
  7,  1,SELECT na linha %d requer WHEN; encontrado "%s"|<linenumber>,<token>
  7,  2,SELECT na linha %d requer WHEN, OTHERWISE, ou END; encontrado "%s"|<linenumber>,<token>
  7,  3,Todas as expressoes WHEN de SELECT na linha %d sao falsas; eh necessario OTHERWISE|<linenumber>
  8,  0,THEN ou ELSE inesperados
  8,  1,THEN nao tem clausula IF ou WHEN correspondente
  8,  2,ELSE nao tem clausula THEN correspondente
  9,  0,WHEN ou OTHERWISE imprevistos
  9,  1,WHEN nao tem SELECT correspondente
  9,  2,OTHERWISE nao tem SELECT  correspondente
 10,  0,END imprevisto  ou nao-correspondente
 10,  1,END nao tem DO ou SELECT correspondentes
 10,  2,END correspondente a DO na linha %d deve ter um simbolo em seguida que seja correspondente a variavel de controle (ou nenhum simbolo); encontrado "%s"|<linenumber>,<token>
 10,  3,END correspondente a DO na linha %d nao deve ter um simbolo em seguida porque nao ha variavel de controle; encontrado "%s"|<linenumber>,<token>
 10,  4,END corresponding to SELECT on line %d must not have a symbol following; found "%s"|<linenumber>,<token>
 10,  5,END nao deve ser imediatamente seguido de THEN
 10,  6,END nao deve ser imediatamente seguido de ELSE
 11,  0,[pilha de controle cheia]
 12,  0,[Clausula > 1024 caracteres]
 13,  0,Caractere invalido no programa
 13,  1,Caractere invalido no programa "('%x'X)"|<hex-encoding>
 14,  0,DO/SELECT/IF incompletos
 14,  1,Instrucao DO requer um END compat¡vel
 14,  2,Instrucao SELECT requer um END correspondente
 14,  3,THEN requer uma instrucao subsquente
 14,  4,ELSE requer uma instrucao subsequente
 15,  0,Linha hexadecimal ou binaria invalida
 15,  1,Localizacao invalida do espaço na posicao %d na linha hexadecimal|<position>
 15,  2,Localizacao invalida do espaço na posicao %d na linha binaria|<position>
 15,  3,Apenas 0-9, a-f, A-F, e espaço sao validos numa linha hexadecimal; encontrado "%c"|<char>
 15,  4,Apenas 0, 1, e espaço sao validos numa linha binaria; encontrado "%c"|<char>
 16,  0,Rotulo nao encontrado
 16,  1,Rotulo "%s" nao encontrado|<name>
 16,  2,Impossivel aplicar SIGNAL ao rotulo "%s" porque estah dentro de um grupo IF, SELECT ou DO|<name>
 16,  3,Impossivel invocar rotulo "%s" porque estah dentro de um grupo IF, SELECT ou DO|<name>
 17,  0,PROCEDURE imprevista
 17,  1,PROCEDURE eh valida apenas quando for a primeira instrucao executada depois de um CALL interno ou chamada de funcao
 18,  0,THEN eh necess rio
 18,  1,Palavra-chave IF na linha %d requer clausula THEN; encontrado "%s"|<linenumber>,<token>
 18,  2,Palavra-chave WHEN na linha %d requer clausula THEN correspondente; encontrado "%s"|<linenumber>,<token>
 19,  0,linha ou simbolo sao necessarios
 19,  1,linha ou simbolo sao necessarios depois da palavra-chave ADDRESS; encontrado "%s"|<token>
 19,  2,linha ou simbolo sao necessarios depois da palavra-chave CALL; encontrado "%s"|<token>
 19,  3,linha ou simbolo sao necessarios depois da palavra-chave NAME; encontrado "%s"|<token>
 19,  4,linha ou simbolo sao necessarios depois da palavra-chave SIGNAL; encontrado "%s"|<token>
 19,  6,linha ou simbolo sao necessarios depois da palavra-chave TRACE; encontrado "%s"|<token>
 19,  7,Simbolo eh necessario no padrao de analise; encontrado "%s"|<token>
 20,  0,Nome eh necessario
 20,  1,Nome requerido; encontrado "%s"|<token>
 20,  2,Encontrado "%s" onde apenas um nome eh valido|<token>
 21,  0,Dados invalidos no final da clausula
 21,  1,A clausula terminou num sinal inesperado; encontrado "%s"|<token>
 22,  0,Linha de caracteres invalida
 22,  1,Linha de caracteres invalida '%s'X|<hex-encoding>
 23,  0,Linha de dados invalida
 23,  1,Linha de dados invalida '%s'X|<hex-encoding>
 24,  0,Pedido de TRACE invalido
 24,  1,A letra de pedido de TRACE deve ser uma de "%s"; encontrado "%c"|ACEFILNOR,<value>
 25,  0,Encontrada sub-palavra-chave invalida
 25,  1,CALL ON deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25,  2,CALL OFF deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25,  3,SIGNAL ON deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25,  4,SIGNAL OFF deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25,  5,ADDRESS WITH deve ser seguido por uma das palavras-chave INPUT, OUTPUT ou ERROR; encontrado "%s"|<token>
 25,  6,INPUT deve ser seguido por uma das palavras-chave STREAM, STEM, LIFO, FIFO ou NORMAL; encontrado "%s"|<token>
 25,  7,OUTPUT deve ser seguido por uma das palavras-chave STREAM, STEM, LIFO, FIFO, APPEND, REPLACE ou NORMAL; encontrado "%s"|<token>
 25,  8,APPEND deve ser seguido por uma das palavras-chave STREAM, STEM, LIFO ou FIFO; encontrado "%s"|<token>
 25,  9,REPLACE deve ser seguido por uma das palavras-chave STREAM, STEM, LIFO ou FIFO; encontrado "%s"|<token>
 25, 11,NUMERIC FORM deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25, 12,PARSE deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25, 13,UPPER deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25, 14,ERROR deve ser seguido por uma das palavras-chave STREAM, STEM, LIFO, FIFO, APPEND, REPLACE ou NORMAL; encontrado "%s"|<token>
 25, 15,NUMERIC deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25, 16,FOREVER deve ser seguido por uma das palavras-chave %s; encontrado "%s"|<keywords>,<token>
 25, 17,PROCEDURE deve ser seguido por uma das palavras-chave EXPOSE; encontrado "%s"|<token>
 26,  0,Numero inteiro invalido
 26,  1,Numeros inteiros devem encaixar-se na configuracao DIGITS em uso (%d); encontrado "%s"|<value>,<value>
 26,  2,O valor da expressao de contagem de repeticoes na instrucao DO deve ser zero ou um numero inteiro positivo; encontrado "%s"|<value>
 26,  3,O valor da expressao FOR na instrucao DO deve ser zero ou um numero inteiro positivo; encontrado "%s"|<value>
 26,  4,O parametro posicional do modelo de analise deve ser um numero inteiro; encontrado "%s"|<value>
 26,  5,O valor dos NUMERIC DIGITS deve ser um numero inteiro positivo; encontrado "%s"|<value>
 26,  6,O valor NUMERIC FUZZ deve ser zero ou um numero inteiro positivo; encontrado "%s"|<value>
 26,  7,O numero usado na configuracao TRACE deve ser um numero inteiro; encontrado "%s"|<value>
 26,  8,Operand a direita do operador da potencia ("**") deve ser um numero inteiro; encontrado "%s"|<value>
 26, 11,O resultado da operacao %s %% %s precisaria de anotacao exponencial nos NUMERIC DIGITS %d em uso|<value>,<value>,<value>
 26, 12,O resultado da operacao %% usada para a operacao %s // %s precisaria de uma anotacao exponencial nos NUMERIC DIGITS %d em uso|<value>,<value>,<value>
 27,  0,Sintaxe DO invalida
 27,  1,Uso invalido da palavra-chave "%s" na clausula DO|<token>
 28,  0,LEAVE ou ITERATE invalidos
 28,  1,LEAVE eh valido apenas dentro de um ciclo DO repetitivo
 28,  2,ITERATE eh valido apenas dentro de um ciclo DO repetitivo
 28,  3,Simbolo subsequente a LEAVE ("%s") deve ser correspondente a variavel de controle de um ciclo DO em uso, ou ser omitido|<token>
 28,  4,Simbolo subsequente a ITERATE ("%s") deve ser correspondente a variavel de controle de um ciclo DO em uso, ou ser omitido|<token>
 29,  0,Nome do ambiente longo demais
 29,  1,Nome do ambiente ultrapassa %d caracteres; encontrado "%s"|#Limit_EnvironmentName,<name>
 30,  0,Nome ou linha longos demais
 30,  1,Nome ultrapassa %d caracteres|#Limit_Name
 30,  2,Linha literal ultrapassa %d caracteres|#Limit_Literal
 31,  0,Nome inicia com numero ou "."
 31,  1,Um valor nao pode ser atribuido a um numero; encontrado "%s"|<token>
 31,  2,Simbolo de variavel nao deve iniciar com um numero; encontrado "%s"|<token>
 31,  3,Simbolo de variavel nao deve iniciar com "."; encontrado "%s"|<token>
 32,  0,[Uso invalido de stem]
 33,  0,Resultado de expressao invalido
 33,  1,O valor de NUMERIC DIGITS ("%d") deve ultrapassar o valor de NUMERIC FUZZ ("%d")|<value>,<value>
 33,  2,O valor de NUMERIC DIGITS ("%d") nao deve ultrapassar %d|<value>,#Limit_Digits
 33,  3,O resultado da expressao subsequente a NUMERIC FORM deve iniciar com "E" ou "S"; encontrado "%s"|<value>
 34,  0,Valor logico nao eh "0" ou "1"
 34,  1,Valor da expressao subsequente a palavra-chave IF deve ser exatamente "0" ou "1"; encontrado "%s"|<value>
 34,  2,Valor da expressao subsequente a palavra-chave WHEN deve ser exatamente "0" ou "1"; encontrado "%s"|<value>
 34,  3,Valor da expressao subsequente a palavra-chave WHILE deve ser exatamente "0" ou "1"; encontrado "%s"|<value>
 34,  4,Valor da expressao subsequente a palavra-chave UNTIL deve ser exatamente "0" ou "1"; encontrado "%s"|<value>
 34,  5,Valor da expressao a esquerda do operador logico "%s" deve ser exatamente "0" ou "1"; encontrado "%s"|<operator>,<value>
 34,  6,Valor da expressao a direita do operador logico "%s" deve ser exatamente "0" ou "1"; encontrado "%s"|<operator>,<value>
 35,  0,Expressao invalida
 35,  1,Expressao invalida detectada em "%s"|<token>
 36,  0,"(" nao-correspondente na expressao
 37,  0,"," ou ")" imprevistos
 37,  1,"," imprevisto
 37,  2,")" nao-correspondente na expressao
 38,  0,Modelo ou padrao invalidos
 38,  1,modelo de analise invalido detectado em "%s"|<token>
 38,  2,Posicao de analise invalida detectada em "%s"|<token>
 38,  3,Instrucao PARSE VALUE requer palavra-chave WITH
 39,  0,[pilha de avaliacao excedente]
 40,  0,Chamada a rotina incorreta
 40,  1,Rotina externa "%s" falhou|<name>
 40,  3,Argumentes insuficientes na chamada de "%s"; minimo necessario eh %d|<bif>,<argnumber>
 40,  4,Argumentos em excesso na chamada de "%s"; maximo necessario eh %d|<bif>,<argnumber>
 40,  5,Argumento inexistente na chamada de "%s"; requerido argumento %d|<bif>,<argnumber>
 40,  9,argumento %s expoente %d ultrapassam %d digitos; encontrado "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,%s argumento %d deve ser um numero; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 12,%s argumento %d deve ser um numero inteiro; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 13,%s argumento %d deve ser zero ou positivo; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 14,%s argumento %d deve ser positivo; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 17,%s argumento 1, deve ter uma parte inteira na escala 0:90 e uma parte decimal nao maior que .9; encontrado "%s"|<bif>,<value>
 40, 18,conversao %s deve ter um ano na escala de 0001 a 9999|<bif>
 40, 19,%s argumento 2, "%s", nao estah no formato descrito em argumento 3, "%s"|<bif>,<value>,<value>
 40, 21,%s argumento %d nao deve ser nulo|<bif>,<argnumber>
 40, 23,%s argumento %d deve ser um caractere unico; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 24,%s argumento 1 deve ser uma linha de caracteres binaria; encontrado "%s"|<bif>,<value>
 40, 25,%s argumento 1 deve ser uma linha de caracteres hexadecimal; encontrado "%s"|<bif>,<value>
 40, 26,%s argumento 1 deve ser um simbolo valido; encontrado "%s"|<bif>,<value>
 40, 27,%s argumento 1, deve ser um nome de stream valido; encontrado "%s"|<bif>,<value>
 40, 28,%s argumento %d, a opcao deve iniciar com um dos "%s"; encontrado "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,conversao %s ao formato "%s" nao eh permitida|<bif>,<value>
 40, 31,%s argumento 1 ("%d") nao deve ultrapassar 100000|<bif>,<value>
 40, 32,%s a diferenca entre argumento 1 ("%d") e argumento 2 ("%d") nao deve ultrapassar 100000|<bif>,<value>,<value>
 40, 33,%s argumento 1 ("%d") deve ser menos que ou igual ao argumento 2 ("%d")|<bif>,<value>,<value>
 40, 34,%s argumento 1 ("%d") deve ser menos que ou igual ao numero de linhas no the programa (%d)|<bif>,<value>,<sourceline()>
 40, 35,%s argumento 1 nao pode ser expresso como um numero inteiro; encontrado "%s"|<bif>,<value>
 40, 36,%s argumento 1 deve ser o nome de uma variavel no agrupamento; encontrado "%s"|<bif>,<value>
 40, 37,%s argumento 3 deve ser o nome de um agrupamento; encontrado "%s"|<bif>,<value>
 40, 38,%s argumento %d nao eh grande o bastante para formatar "%s"|<bif>,<argnumber>,<value>
 40, 39,%s argumento 3 nao eh zero ou um; encontrado "%s"|<bif>,<value>
 40, 41,%s argumento %d deve estar dentro dos limites do stream; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 42,%s argumento 1; nao eh possivel posicionar neste stream; encontrado "%s"|<bif>,<value>
 40, 43,%s argumento %d deve ser um caractere nao-alfanumerico ou a linha nula;  encontrado "%s"|<bif>,<argnumber>,<value>
 40, 44,%s argumento %d, "%s", eh um formato incompativel com o separador especificado no argumento %d|<bif>,<argnumber>,<value>,<argnumber>
 40,914,[%s argumento  %d, deve ser um dos "%s"; encontrado "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: erro I/O de stream de nivel baixo; %s]|<bif>,<description>
 40,921,[%s argumento %d, modo de posicionamento do stream "%s"; incompativel com o modo stream aberto]|<bif>,<argnumber>,<value>
 40,922,[%s argumento %d, sub-comando insuficientes; minimo necessario eh %d; encontrado %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[%s argumento %d, sub-comandos demais; maximo necessario eh %d; encontrado %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[%s argumento %d, especificacao posicional invalida; eh necessario um dos "%s"; encontrado "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, funcao TIMEOUT, espera-se um numero total entre 0 e %d; encontrado \"%s\"]|<value>,<value>
 40,980,Input inesperado, de tipo desconhecido ou dados ilegais%s%s|: ,<location>
 40,981,Numero fora do intervalo permitido%s%s|: ,<location>
 40,982,Linha muito grande para o buffer definido%s%s|: ,<location>
 40,983,Cominacao ilegal de tipo/tamanho%s%s|: ,<location>
 40,984,Numero nao suportado como NAN, +INF, -INF%s%s|: ,<location>
 40,985,Estrutura complexa demais para o buffer estatico interno buffer%s%s|: ,<location>
 40,986,Falta um elemento da estrutura%s%s|: ,<location>
 40,987,Falta um valor da estrutura%s%s|: ,<location>
 40,988,O nome ou parte do nome eh ilegal para o interprete%s%s|: ,<location>
 40,989,Houve um problema na interface entre Regina e GCI%s%s|: ,<location>
 40,990,Tipo nao se ajusta aos requisitos para tipos basicos (argumentos/valor de retorno)%s%s|: ,<location>
 40,991,O numero de argumentos estah errado ou falta um argumento%s%s|: ,<location>
 40,992,A pilha interna do GCI para argumentos recebeu sobrecarga%s%s|: ,<location>
 40,993,GCI contou um excesso de recipientes LIKE%s%s|: ,<location>
 41,  0,mah conversao aritmetica
 41,  1,valor nao-numerico ("%s") a esquerda da operacao aritmetica "%s"|<value>,<operator>
 41,  2,valor nao-numerico ("%s") a direita da operacao aritmetica "%s"|<value>,<operator>
 41,  3,valor nao-numerico ("%s") usado com o operador de prefixo "%s"|<value>,<operator>
 41,  4,valor da expressao TO na instrucao DO deve ser numerico; encontrado "%s"|<value>
 41,  5,valor da expressao BY na instrucao DO deve ser numerico; encontrado "%s"|<value>
 41,  6,valor da exressao da variavel de controle na instrucao DO deve ser numerico; encontrado "%s"|<value>
 41,  7,expoente ultrapassa %d digitos; encontrado "%s"|#Limit_ExponentDigits,<value>
 42,  0,excedentes/excedentes negativos aritmeticos
 42,  1,excedente aritmetico detectado em "%s %s %s"; expoente do resultado requer mais do que %d digitos|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,excedente negativo aritmetico detectado em "%s %s %s"; expoente do resultado requer mais do que %d digitos|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,excedente aritmetico; divisor nao deve ser zero
 43,  0,rotina nao encontrada
 43,  1,nao foi possivel achar rotina "%s"|<name>
 44,  0,funcao nao devolveu dados
 44,  1,nao houve dados devolvidos da funcao "%s"|<name>
 45,  0,nao ha dados especificados na funcao RETURN
 45,  1,No aguardo de dados na instrucao RETURN porque a rotina "%s" foi considerada uma funcao|<name>
 46,  0,referencia invalida de variavel
 46,  1,Sinal a mais ("%s") encontrado na referencia de variavel; eh necess rio ")"|<token>
 47,  0,rotulo imprevisto
 47,  1,dados INTERPRET nao devem conter rotulos; encontrado "%s"|<name>
 48,  0,Falha no programa de servico do sistema
 48,  1,Falha no programa de servico do sistema: %s|<description>
 48,920,Erro I/O de stream de nivel baixo: %s %s: %s|<description>,<stream>,<description>
 49,  0,Erro de interpretacao
 49,  1,Erro de interpretacao: Falhou em %s, linha %d: "%s". Notifique o erro!|<module>,<linenumber>,<description>
 50,  0,simbolo reservado nao-reconhecido
 50,  1,simbolo reservado "%s" nao-reconhecido|<token>
 51,  0,funcao de nome invalida
 51,  1,nomes de funcao sem aspas nao devem terminar num ponto; encontrado "%s"|<token>
 52,  0,Resulto devolvido por "%s" ultrapassa %d caracteres|<name>,#Limit_String
 53,  0,opcao invalida
 53,  1,linha ou simbolo eh necess ria depois da palavra-chave STREAM; encontrado "%s"|<token>
 53,  2,referencia da variavel eh necess ria depois da palavra-chave STEM; encontrado "%s"|<token>
 53,  3,Argumento para STEM deve ter um ponto como ultimo caractere; encontrado "%s"|<name>
 53,100,linha ou simbolo eh necess ria depois da palavra-chave LIFO; encontrado "%s"|<token>
 53,101,linha ou simbolo eh necess ria depois da palavra-chave FIFO; encontrado "%s"|<token>
 54,  0,Valor STEM invalido
 54,  1,Para este STEM APPEND, o valor de "%s" deve ser uma contagem de linhas; encontrado "%s"|<name>,<value>
#
# Todas as mensagens de erro a partir daqui nao estao definidas pelo ANSI
#
 60,  0,[nao eh possivel recarregar arquivo temporario]
 61,  0,[operacao incorreta de busca no arquivo]
 64,  0,[erro de sintaxe ao analisar]
 64,  1,[erro de sintaxe na linha %d]
 64,  2,[erro geral de sintaxe na %d, coluna %d]|<linenumber>,<columnnumber>
 90,  0,[dispositivo nao-conforme com ANSI usado com "OPTIONS STRICT_ANSI"]
 90,  1,[%s eh uma extensao Regina BIF]|<bif>
 90,  2,[%s eh uma instrucao de extensao Regina]|<token>
 90,  3,[%s argumento %d, a opcao deve iniciar com um dos "%s" com "OPTIONS STRICT_ANSI"; encontrado "%s"; uma extensao Regina]|<bif>,<argnumber>,<optionslist>,<value>
 90,  4,[%s eh uma operador de extensao Regina]|<token>
 93,  0,[chamada incorreta para rotina]
 93,  1,[o comando STREAM %s deve ser seguido por um dos "%s"; encontrado "%s"]|<token>,<value>,<value>
 93,  3,[o comando STREAM deve ser um dos "%s"; encontrado "%s"]|<value>,<value>
 94,  0,[erro de interface de fila externa]
 94,  1,[a fila externa cronometrou para fora]
 94, 99,[erro interno com interface de fila externa: %d "%s"]|<description>,<systemerror>
 94,100,[erro geral do sistema com interface de fila externa. %s. %s]|<description>,<systemerror>
 94,101,[Erro ao conectar a %s na porta %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[nao eh possivel obter endereco IP para %s]|<machine>
 94,103,[formato invalido para servidor no nome da fila especificado: "%s"]|<queuename>
 94,104,[formato invalido para o nome da fila: "%s"]|<queuename>
 94,105,[nao eh possivel iniciar a interface do soquete Windows: %s]|<systemerror>
 94,106,[foi ultrapasso o numero maximo de filas externas: %d]|<maxqueues>
 94,107,[ocorreu um erro ao ler o soquete: %s]|<systemerror>
 94,108,[foi passado comutador invalido. Deve ser um dos "%s"]|<switch>
 94,109,[Fila \"%s\" nao encontrada]|<queuename>
 94,110,[%s invalido para linhas externasl|<bif>
 94,111,[RXQUEUE funcao %s invalida para linhas internas]|<functionname>
 94,112,[Unable to %s SESSION queue]|<action>
 95,  0,[dispositivo restrito usado em modo "seguro"]
 95,  1,[%s invalido no modo "seguro"]|<token>
 95,  2,[%s argumento %d invalido no modo "seguro"]|<bif>,<argnumber>
 95,  3,[%s argumento %d: "%s", invalido no modo "seguro"]|<bif>,<argnumber>,<token>
 95,  4,[argumento STREAM 3: abrir arquivos para acesso WRITE invalido no modo "seguro"]
 95,  5,[executar comandos externos eh invalido no modo "seguro"]
100,  0,[erro desconhecido de sistema de arquivos]
#
# frases que precisam ser traduzidas  - Extra geral:
# O texto entre () eh o nome correspondente da matriz em error.c
P,  0,Erro %d ao executar "%.*s", linha %d: %.*s
P,  1,Erro %d.%d: %.*s
P,  2,Erro %d ao executar "%.*s": %.*s
#
# (err1prefix)   "Erro %d em execucao\"%.*s\", linha %d: %.*s",
# (suberrprefix) "erro %d.%d: %.*s",
# (err2prefix)   "erro %d em execucao\"%.*s\": %.*s",
# (erropen)      "Nao eh possivel abrir arquivo de idiomas%s",
# (errcount)     "numero incorreto de mensagens no arquivo de idioma: %s",
# (errread)      "Nao eh possivel ler o arquivo de idioma: %s",
# (errmissing)   "Falta texto no arquivo de idioma: %s.mtb",
# (errcorrupt)   "Arquivo de idioma: %s.mtb  estah corrompido",
