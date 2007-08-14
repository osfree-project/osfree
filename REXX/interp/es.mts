#
# spanish.mts Version 0, 30-08-2001
#
# Regina error messages - Spanish
# Original in English Written by Mark Hessling <M.Hessling@qut.edu.au>
# Translated by Pablo Garcia-Abia <Pablo.Garcia@cern.ch>
#
#
# NOTES:
# -----
#
# Some lines are preceded by a "-" in the first column. These are lines
# containing English words (see note 2).
#
#
# 1. I have finally used accents. This is a table of the characters affected:
#
#    What I typed   Hex code   Description
#    ------------   --------   -------------------
#         ñ            F1      n-tilde
#         á            E1      a with acute accent
#         é            E9      e with acute accent
#         í            ED      i with acute accent
#         ó            F3      o with acute accent
#         ú            FA      u with acute accent
#    ------------   --------   -------------------
#
# 2. Some terms have not been translated:
#
#       *  Debug
#       *  Overflow/underflow
#       *  Parsing
#       *  Pool
#       *  Socket
#       *  Stack
#       *  Stem
#       *  Switch
#
# 3. The term "stream" has been translated as "file", even though is not the same !!
#
# 4. In order to avoid changing the order of %s and %d in messages #40
#    the text:
#
#       "%s argument %d must be..."
#
#    has been translated as
#
#       "The argument of %s number %d must be..."
#
#    This has been found to be the only possible solution. I guess it will
#    also be the case for French, Portuguese and other Latin languages.
#
  0,  1,Error %s ejecutando %s, línea %d:|<value>,<source>,<linenumber>
  0,  2,Error %s en el trazado interactivo:|<value>
  0,  3,Trazado interactivo.  "Trace Off" para finalizar el "debug". ENTER para continuar.
  2,  0,Fallo durante la finalización
  2,  1,Fallo durante la finalización: %s|<description>
  3,  0,Fallo durante la inicialización
  3,  1,Fallo durante la inicialización: %s|<description>
  4,  0,Programa interrumpido
  4,  1,Programa interrumpido con la condición HALT: %s|<description>
  5,  0,Recursos del sistema agotados
  5,  1,Recursos del sistema agotados: %s|<description>
  6,  0,"/*" o comilla sin correspondencia
  6,  1,Delimitador de comentario ("/*") sin correspondencia
  6,  2,Comilla sencilla (') sin correspondencia
  6,  3,Comilla doble (") sin correspondencia
  7,  0,WHEN o OTHERWISE esperado
  7,  1,SELECT en la línea %d requiere WHEN; encontrado "%s"|<linenumber>,<token>
  7,  2,SELECT en la línea %d requiere WHEN, OTHERWISE, o END; encontrado "%s"|<linenumber>,<token>
  7,  3,Todas las expresiones WHEN del SELECT en la línea %d son falsas; OTHERWISE esperado|<linenumber>
  8,  0,THEN o ELSE inesperado
  8,  1,THEN no tiene las frases con IF o WHEN correspondientes
  8,  2,ELSE no tiene la frase THEN correspondiente
  9,  0,WHEN o OTHERWISE inesperado
  9,  1,WHEN no tiene el correspondiente SELECT
  9,  2,OTHERWISE no tiene el correspondiente SELECT
 10,  0,END inesperado o sin correspondencia
 10,  1,END no tiene el correspondiente DO o SELECT
 10,  2,END correspondiente al DO de la línea %d debe tener un símbolo a continuación que corresponda a la variable de control (o ningún símbolo); encontrado "%s"|<linenumber>,<token>
 10,  3,END correspondente al DO de la línea %d no debe llevar ningún símbolo a continuación porque no hay variable de control; encontrado "%s"|<linenumber>,<token>
 10,  4,END correspondiente al SELECT de la línea %d no debe estar seguido de ningún símbolo; encontrado "%s"|<linenumber>,<token>
 10,  5,END no debe estar inmediatamente a continuación de THEN
 10,  6,END no debe estar inmediatamente a continuación de ELSE
 11,  0,["Stack" de control lleno]
 12,  0,["Clause" > 1024 caracteres]
 13,  0,Carácter inválido en programa
 13,  1,Carácter inválido en programa "('%x'X)"|<hex-encoding>
 14,  0,DO/SELECT/IF incompleto
 14,  1,La instrucción DO requiere su correspondiente END
 14,  2,La instrucción SELECT requiere su correspondiente END
 14,  3,THEN requiere una instrucción a continuación
 14,  4,ELSE requiere una instrucción a continuación
 15,  0,Cadena hexadecimal o binaria inválida
 15,  1,Situación inválida de un espacio blanco en la posición %d en una cadena hexadecimal|<position>
 15,  2,Situación inválida de un espacio blanco en la posición %d en una cadena binaria|<position>
 15,  3,Solo 0-9, a-f, A-F, y espacio son válidos en una cadena hexadecimal; encontrado "%c"|<char>
 15,  4,Solo 0, 1, y espacio son válidos en una cadena binaria; encontrado "%c"|<char>
 16,  0,Etiqueta no encontrada
 16,  1,Etiqueta "%s" no encontrada|<name>
 16,  2,No se puede realizar SIGNAL a la etiqueta "%s" porque está dentro de un grupo IF, SELECT o DO|<name>
 16,  3,No se puede invocar la etiqueta "%s" porque está dentro de un grupo IF, SELECT o DO|<name>
 17,  0,PROCEDURE inesperado
 17,  1,PROCEDURE solo es válido cuando es la primera instrucción ejecutada después de un CALL interno o de invocar una función
 18,  0,Esperado THEN
 18,  1,La palabra clave IF en la línea %d requiere la "clause" THEN correspondiente; encontrado "%s"|<linenumber>,<token>
 18,  2,La palabra clave WHEN en la línea %d requiere la "clause" THEN correspondiente; encontrado "%s"|<linenumber>,<token>
 19,  0,Cadena o símbolo esperado
 19,  1,Cadena o símbolo esperado después de la palabra clave ADDRESS; encontrado "%s"|<token>
 19,  2,Cadena o símbolo esperado después de la palabra clave CALL; encontrado "%s"|<token>
 19,  3,Cadena o símbolo esperado después de la palabra clave NAME; encontrado "%s"|<token>
 19,  4,Cadena o símbolo esperado después de la palabra clave SIGNAL; encontrado "%s"|<token>
 19,  6,Cadena o símbolo esperado después de la palabra clave TRACE; encontrado "%s"|<token>
 19,  7,Símbolo esperado en el modelo de "parsing"; encontrado "%s"|<token>
 20,  0,Nombre esperado
 20,  1,Nombre requerido; encontrado "%s"|<token>
 20,  2,Encontrado "%s" cuando solo un nombre es válido|<token>
 21,  0,Datos inválidos al final de "clause"
 21,  1,La "clause" terminó con un elemento inesperado; encontrado "%s"|<token>
 22,  0,Cadena de caracteres inválida
 22,  1,Cadena de caracteres inválida '%s'X|<hex-encoding>
 23,  0,Cadena de datos inválida
 23,  1,Cadena de datos inválida '%s'X|<hex-encoding>
 24,  0,TRACE "request" inválido
 24,  1,La letra del TRACE "request" debe ser una de "%s"; encontrado "%c"|ACEFILNOR,<value>
 25,  0,Sub-palabra clave inválida encontrada
 25,  1,CALL ON debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25,  2,CALL OFF debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25,  3,SIGNAL ON debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25,  4,SIGNAL OFF debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25,  5,ADDRESS WITH debe estar seguido de una de las palabras clave INPUT, OUTPUT or ERROR; encontrado "%s"|<token>
 25,  6,INPUT debe estar seguido de una de las palabras clave STREAM, STEM, LIFO, FIFO o NORMAL; encontrado "%s"|<token>
 25,  7,OUTPUT debe estar seguido de una de las palabras clave STREAM, STEM, LIFO, FIFO, APPEND, REPLACE o NORMAL; encontrado "%s"|<token>
 25,  8,APPEND debe estar seguido de una de las palabras clave STREAM, STEM, LIFO o FIFO; encontrado "%s"|<token>
 25,  9,REPLACE debe estar seguido de una de las palabras clave STREAM, STEM, LIFO o FIFO; encontrado "%s"|<token>
 25, 11,NUMERIC FORM debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25, 12,PARSE debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25, 13,UPPER debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25, 14,ERROR debe estar seguido de una de las palabras clave STREAM, STEM, LIFO, FIFO, APPEND, REPLACE o NORMAL; encontrado "%s"|<token>
 25, 15,NUMERIC debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25, 16,FOREVER debe estar seguido de una de las palabras clave %s; encontrado "%s"|<keywords>,<token>
 25, 17,PROCEDURE debe estar seguido de una de las palabras clave EXPOSE; encontrado "%s"|<token>
 26,  0,Número entero inválido
 26,  1,Los números enteros deben ajustarse al valor actual de DIGITS establecido(%d); encontrado "%s"|<value>,<value>
 26,  2,El valor de contaje de repetición en la instrucción DO debe ser cero o un número entero positivo; encontrado "%s"|<value>
 26,  3,El valor de la expresión FOR en la instrucción DO debe ser cero o un número entero positivo; encontrado "%s"|<value>
 26,  4,El parámetro posicional de una regla de "parsing" debe ser un número entero; encontrado "%s"|<value>
 26,  5,El valor de NUMERIC DIGITS debe ser cero o un número entero positivo; encontrado "%s"|<value>
 26,  6,El valor de NUMERIC FUZZ debe ser cero o un número entero positivo; encontrado "%s"|<value>
 26,  7,El número usado en el "TRACE setting" debe ser un número entero; encontrado "%s"|<value>
 26,  8,El operando a la derecha del operador de exponenciación ("**") debe ser un número entero; encontrado "%s"|<value>
 26, 11,El resultado de la operación %s %% %s necesitaria notación exponencial notation para el valor actual de NUMERIC DIGITS %d|<value>,<value>,<value>
 26, 12,El resultado de la operación %% utilizada para la operación %s // %s necesitaría notación exponencial para el valor actual de NUMERIC DIGITS %d|<value>,<value>,<value>
 27,  0,Sintaxis de DO inválida
 27,  1,Uso inválido de la palabra clave "%s" en una "clause" DO|<token>
 28,  0,LEAVE o ITERATE inválido
 28,  1,LEAVE solo es válido dentro de un bucle DO de repetición
 28,  2,ITERATE solo es válido dentro de un bucle DO de repetición
 28,  3,El símbolo a continuación de LEAVE ("%s") debe corresponder a una variable de control de un bucle DO o ser omitido|<token>
 28,  4,El símbolo a continuación de ITERATE ("%s") debe corresponder a una variable de control de un bucle DO o ser omitido|<token>
 29,  0,Nombre de entorno demasiado largo
 29,  1,Nombre de entorno excede %d caracteres; encontrado "%s"|#Limit_EnvironmentName,<name>
 30,  0,Nombre o cadena demasiado largo
 30,  1,Nombre excede %d caracteres|#Limit_Name
 30,  2,Cadena literal excede %d caracteres|#Limit_Literal
 31,  0,Nombre empieza con un número o "."
 31,  1,Un valor no puede ser asignado a un número; encontrado "%s"|<token>
 31,  2,Un símbolo de variable no puede empezar por un número; encontrado "%s"|<token>
 31,  3,Un símbolo de variable no puede empezar por "."; encontrado "%s"|<token>
 32,  0,[Uso inválido de "stem"]
 33,  0,Resultado de expresión inválido
 33,  1,El valor de NUMERIC DIGITS "%d" debe exceder el valor de NUMERIC FUZZ "(%d)"|<value>,<value>
 33,  2,El valor de NUMERIC DIGITS "%d" no debe exceder %d|<value>,#Limit_Digits
 33,  3,El resultado de la expresión a continuación de NUMERIC FORM debe empezar por "E" o "S"; encontrado "%s"|<value>
 34,  0,El valor lógico no es "0" o "1"
 34,  1,El valor de la expresión que sigue a la palabra clave IF debe ser exactamente "0" o "1"; encontrado "%s"|<value>
 34,  2,El valor de la expresión que sigue a la palabra clave WHEN debe ser exactamente "0" o "1"; encontrado "%s"|<value>
 34,  3,El valor de la expresión que sigue a la palabra clave WHILE debe ser exactamente "0" o "1"; encontrado "%s"|<value>
 34,  4,El valor de la expresión que sigue a la palabra clave UNTIL debe ser exactamente "0" o "1"; encontrado "%s"|<value>
 34,  5,El valor de la expresión a la izquierda del operador lógico "%s" debe ser exactamente "0" o "1"; encontrado "%s"|<operator>,<value>
 34,  6,El valor de la expresión a la derecha del operador lógico "%s" debe ser exactamente "0" o "1"; encontrado "%s"|<operator>,<value>
 35,  0,Expresión inválida
 35,  1,Expresión inválida detectada en "%s"|<token>
 36,  0,"(" sin correspondencia en expresión
 37,  0,"," o ")" inesperado
 37,  1,"," inesperada
 37,  2,")" sin correspondencia en expresión
 38,  0,Regla de modelo inválido
 38,  1,Regla de "parsing" inválida detectado en "%s"|<token>
 38,  2,Posición de "parsing" inválida detectada en "%s"|<token>
 38,  3,La instrucción PARSE VALUE requiere la palabra clave WITH
 39,  0,["Overflow" del stack de evaluación]
 40,  0,Llamada incorrecta a rutina
 40,  1,La rutina externa "%s" ha fallado|<name>
 40,  3,Arguments insuficientes en la invocación de "%s"; el mínimo esperado es %d|<bif>,<argnumber>
 40,  4,Demasiados argumentos en la invocación de "%s"; el máximo esperado es %d|<bif>,<argnumber>
 40,  5,Falta un argumento en la invocación de "%s"; se requiere el argumento %d|<bif>,<argnumber>
 40,  9,El exponente del argumento de %s número %d excede %d digitos; encontrado "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,El argumento de %s número %d debe ser un número; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 12,El argumento de %s número %d debe ser un número entero; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 13,El argumento de %s número %d debe ser cero o positivo; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 14,El argumento de %s número %d debe ser positivo; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 17,El argumento 1 de %s, debe tener una parte entera en el rango 0:90 y una parte decimal que no sea mayor que .9; encontrado "%s"|<bif>,<value>
 40, 18,La conversión %s debe tener un año en el rango 0001 a 9999|<bif>
 40, 19,El argumento 2 de %s, "%s", no está en el formato descrito por el argumento 3, "%s"|<bif>,<value>,<value>
 40, 21,El argumento de %s número %d no debe ser nulo (null)|<bif>,<argnumber>
 40, 23,El argumento de %s número %d debe ser un solo carácter; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 24,El argumento 1 de %s debe ser una cadena binaria; encontrado "%s"|<bif>,<value>
 40, 25,El argumento 1 de %s debe ser una cadena hexadecimal; encontrado "%s"|<bif>,<value>
 40, 26,El argumento 1 de %s debe ser un símbolo válido; encontrado "%s"|<bif>,<value>
 40, 27,El argumento 1 de %s, debe ser un nombre de fichero válido; encontrado "%s"|<bif>,<value>
 40, 28,El argumento de %s número %d, la opción debe empezar con uno de "%s"; encontrado "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,La conversión %s al formato "%s" no está permitida|<bif>,<value>
 40, 31,El argumento 1 de %s ("%d") no debe exceder 100000|<bif>,<value>
 40, 32,%s la diferencia entre el argumento 1 ("%d") y el argumento 2 ("%d") no debe exceder 100000|<bif>,<value>,<value>
 40, 33,El argumento 1 de %s ("%d") debe ser menor o igual que el argumento 2 ("%d")|<bif>,<value>,<value>
 40, 34,El argumento 1 de %s ("%d") debe ser menor o igual que el número de líneas en el programa (%d)|<bif>,<value>,<sourceline()>
 40, 35,El argumento 1 de %s no puede expresarse como un número entero; encontrado "%s"|<bif>,<value>
 40, 36,El argumento 1 de %s debe ser el nombre de una variable en el "pool"; encontrado "%s"|<bif>,<value>
 40, 37,El argumento 3 de %s debe ser el nombre de un "pool"; encontrado "%s"|<bif>,<value>
 40, 38,El argumento de %s número %d no es suficientemente grande como para formatear "%s"|<bif>,<argnumber>,<value>
 40, 39,El argumento 3 de %s no es cero ni uno; encontrado "%s"|<bif>,<value>
 40, 41,El argumento de %s número %d debe estar dentro de los límites del fichero; encontrado "%s"|<bif>,<argnumber>,<value>
 40, 42,El argumento 1 de %s; no se puede posicionar en este fichero; encontrado "%s"|<bif>,<value>
 40,914,[El argumento de %s número %d, debe ser uno de "%s"; encontrado "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: error de bajo nivel de E/S del fichero; %s]|<bif>,<description>
 40,921,[Argumento de %s número %d, modo "%s" de posicionado en fichero; incompatible con el modo de fichero abierto]|<bif>,<argnumber>,<value>
 40,922,[Argumento de %s número %d, muy pocos sub-comandos; el mínimo esperado es %d; encontrado %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[Argumento de %s número %d, demasiados sub-comandos; el máximo esperado es %d; encontrado %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[Argumento de %s número %d, especificación posicional inválida; esperando uno de "%s"; encontrado "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, function TIMEOUT, expecting a whole number between -1 and %d; found \"%s\"]|<value>,<value>
 40,980,Unexpected input, either unknown type or illegal data%s%s|: ,<location>
 40,981,Number out of the allowed range%s%s|: ,<location>
 40,982,String too big for the defined buffer%s%s|: ,<location>
 40,983,Illegal combination of type/size%s%s|: ,<location>
 40,984,Unsupported number like NAN, +INF, -INF%s%s|: ,<location>
 40,985,Structure too complex for static internal buffer%s%s|: ,<location>
 40,986,An element of the structure is missing%s%s|: ,<location>
 40,987,A value of the structure is missing%s%s|: ,<location>
 40,988,The name or part of the name is illegal for the interpreter%s%s|: ,<location>
 40,989,A problem occured at the interface between Regina and GCI%s%s|: ,<location>
 40,990,The type won't fit the requirements for basic types (arguments/return value)%s%s|: ,<location>
 40,991,The number of arguments is wrong or an argument is missing%s%s|: ,<location>
 41,  0,Mala conversión aritmética
 41,  1,Valor no numérico ("%s") a la izquierda de la operación aritmética "%s"|<value>,<operator>
 41,  2,Valor no numérico ("%s") a la derecha de la operación aritmética "%s"|<value>,<operator>
 41,  3,Valor no numérico ("%s") utilizado con el operator de prefijo "%s"|<value>,<operator>
 41,  4,El valor de la expresión TO en la instrucción DO debe ser numérico; encontrado "%s"|<value>
 41,  5,El valor de la expresión BY en la instrucción DO debe ser numérico; encontrado "%s"|<value>
 41,  6,El valor de la expresión de la variable de control de la instrucción DO debe ser numérico; encontrado "%s"|<value>
 41,  7,Exponente excede %d dígitos; encontrado "%s"|#Limit_ExponentDigits,<value>
 42,  0,"Overflow/underflow" aritmético
 42,  1,"Overflow" aritmético detectado en "%s %s %s"; el exponente del resultado requiere mas de %d dígitos|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,"Underflow" aritmético detectado en "%s %s %s"; el exponente del resultado requiere mas de %d dígitos|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,"Overflow" aritmético; el divisor debe ser distinto de cero
 43,  0,Rutina no encontrada
 43,  1,No se pudo encontrar la rutina "%s"|<name>
 44,  0,La función no devolvió datos
 44,  1,Ningún dato devuelto por la función "%s"|<name>
 45,  0,Ningún dato especificado en la función RETURN
 45,  1,Dates esperados en la instrucción RETURN porque la rutina "%s" fue llamada como una función|<name>
 46,  0,Referencia a una variable inválida
 46,  1,Elemento extra ("%s") encontrado en la referencia a una variable; ")" esperado|<token>
 47,  0,Etiqueta inesperada
 47,  1,Los datos de INTERPRET no deben contener etiquetas; encontrado "%s"|<name>
 48,  0,Fallo en el servicio del sistema
 48,  1,Fallo en el servicio del sistema: %s|<description>
 48,920,Error de bajo nivel de E/S del fichero: %s %s: %s|<description>,<stream>,<description>
 49,  0,Error de interpretación
 49,  1,Error de interpretación: Falló en %s, línea %d: "%s". Por favor, comunicar el error!|<module>,<linenumber>,<description>
 50,  0,Símbolo reservado no reconocido
 50,  1,Símbolo reservado no reconocido "%s"|<token>
 51,  0,Nombre de función inválido
 51,  1,Los nombres de función que no están entre comillas no pueden terminar en punto; encontrado "%s"|<token>
 52,  0,El resultado devuelto por "%s" es más largo de %d caracteres|<name>,#Limit_String
 53,  0,Opción inválida
 53,  1,Cadena o símbolo esperado tras la palabra clave STREAM; encontrado "%s"|<token>
 53,  2,Referencia a variable esperada tras la palabra clave STEM; encontrado "%s"|<token>
 53,  3,El argumento de STEM debe tener un punto como último carácter; encontrado "%s"|<name>
 53,100,Cadena o símbolo esperado tras la palabra clave LIFO; encontrado "%s"|<token>
 53,101,Cadena o símbolo esperado tras la palabra clave FIFO; encontrado "%s"|<token>
 54,  0,Valor de STEM inválido
 54,  1,Para este STEM APPEND, el valor de "%s" debe ser el número de líneas; encontrado "%s"|<name>,<value>
#
# All error messages after this point are not defined by ANSI
#
 60,  0,[Imposible rebobinar el fichero transitorio]
 61,  0,[Operación de buscado ("seek") impropia en el fichero]
 64,  0,[Error de sintaxis en el "parsing"]
 64,  1,[Error de sintaxis en la línea %d]
 64,  2,[Error general de sintaxis en la línea %d, columna %d]|<linenumber>,<columnnumber>
 90,  0,[Propiedad no-ANSI usada con "OPTIONS STRICT_ANSI"]
 90,  1,[%s es un BIF de la extensión de Regina]|<bif>
 90,  2,[%s es una instrucción de la extensión de Regina]|<token>
 90,  3,[Argumento de %s número %d, opción debe empezar por uno de "%s" con "OPTIONS STRICT_ANSI"; encontrado "%s"; una extension de Regina]|<bif>,<argnumber>,<optionslist>,<value>
 93,  0,[Llamada incorrecta a rutina]
 93,  1,[El comando %s de STREAM debe estar seguido por uno de "%s"; encontrado "%s"]|<token>,<value>,<value>
 93,  3,[El comando de STREAM debe ser uno de "%s"; encontrado "%s"]|<value>,<value>
 94,  0,[Error de la interfaz de la cola externa]
 94, 99,[Error interno con la interfaz de la cola externa: %d "%s"]|<description>,<systemerror>
 94,100,[Error general del sistema con la interfaz de la cola externa. %s. %s]|<description>,<systemerror>
 94,101,[Error conectando con %s en la puerta %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[Incapaz de obtener la dirección de IP para %s]|<machine>
 94,103,[Formato inválido para el servidor en el nombre de cola especificado: "%s"]|<queuename>
 94,104,[Formato inválido para el nombre de cola: "%s"]|<queuename>
 94,105,[Incapaz de empezar la interfaz de "Sockets" de "Windows": %s]|<systemerror>
 94,106,[Excedido el número máximo de colas externas: %d]|<maxqueues>
 94,107,[Error ocurrido leyendo el "socket": %s]|<systemerror>
 94,108,[El "Switch" pasado es inválido. Debe ser uno de "%s"]|<switch>
 94,109,[Queue \"%s\" not found]|<queuename>
 94,110,[%s invalid for external queues]|<bif>
 94,111,[RXQUEUE function %s invalid for internal queues]|<functionname>
 95,  0,[Propiedad restringida usada en modo "seguro" (safe)]
 95,  1,[%s inválido en modo "seguro" (safe)]|<token>
 95,  2,[Argumento de %s número %d inválido en modo "seguro" (safe)]|<bif>,<argnumber>
 95,  3,[Argumento de %s número %d: "%s", inválido en modo "seguro" (safe)]|<bif>,<argnumber>,<token>
 95,  4,[Argumento 3 de STREAM: abrir ficheros en modo ESCRITURA es inválido en modo "seguro" (safe)]
 95,  5,[Ejecutar comandos externos es inválido en modo "seguro" (safe)]
100,  0,[Error desconocido del sistema de ficheros]
#
# Extra general phrases requiring translation:
#
# Error 3 ejecutando "<file>" línea 1:
# Incapaz de abrir el fichero de lenguaje: %s
# Número incorrecto de mensajes en el fichero de lenguaje: %s
# Incapaz de leer el fichero de lenguaje: %s
# Incapaz de mostrar el texto del error %d.%d; fichero de lenguaje: %s no disponible
# Incapaz de mostrar el texto del error %d.%d; falta el texto del fichero de lenguaje: %s
# Incapaz de mostrar el texto del error %d.%d; fichero de lenguaje: %s está corrupto
