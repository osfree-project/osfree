# fatal errors
0.0:No hay memoria suficiente. Se requieren %ld bytes m s.\n
0.1:LA MEMORIA DEL SISTEMA ESTµ CORRUPTA (fallo en int 21.5803)\n
0.2:Cadena de MCBs corrupta: no llega al tope de la TPA en %dk (£ltimo=0x%x)\n
0.3:Cadena de MCBs corrupta: sin MCB Z tras el £ltimo M (es %c, en seg 0x%x)\n
0.4:Opci¢n desconocida: %s\nUse /? para la ayuda\n
# misc messages
1.0:Sistema operativo desconocido
1.1:%lu bytes\n
1.2:(%s bytes)\n
1.3: (%7s bytes)\n
# memory types
2.0:Tipo de memoria    Total       Usado     Libre\n
#   ----------------  --------   --------   --------
2.1:Convencional
2.2:Superior
2.3:Reservada
2.4:Extendida (XMS)
2.5:Memoria total
2.6:Total bajo 1 MB
2.7:Total Expandida (EMS)
2.8:Libre Expandida (EMS)
2.9:Tama¤o de programa ejecutable m s grande
2.10:Bloque de memoria superior libre m s grande
2.11:%s reside en memoria alta (HMA).\n
# block types
3.0:
3.1:libre
3.2:c¢digo sistema
3.3:datos sistema
3.4:programa
3.5:entorno    
3.6: rea de datos
3.7:reservada
3.8:controlador
3.9:IFS
# classify msgs
4.0:\nM¢dulos que usan memoria bajo 1 MB:\n\n
4.1:  Nombre         Total           Convencional        Superior\n
#     --------  ----------------   ----------------   ----------------
4.2:SISTEMA
4.3:Libre
4.4:\nSegmento Tama¤o     Nombre        Tipo\n
#     ------- --------  ----------  -------------
4.5:\n  Direcci¢n   Atrib.   Nombre     Programa\n
#      -----------  ------ ----------  ----------
# EMS stuff
5.0:Error interno EMS.\n
5.1: No hay controlador EMS en el sistema.\n
5.2:\nVersi¢n del controlador EMS
5.3:Marco de p gina EMS
5.4:Total memoria EMS
5.5:Memoria EMS libre
5.6:Referencias (handles) totales
5.7:Referencias (handles) libres
5.8:\n  Handle  P ginas  Tama¤o     Nombre\n
#      -------- ------  --------   ----------
# XMS stuff
6.0:No hay controlador XMS instalado en el sistema.\n
6.1:\nComprobando la memoria XMS ...\n
6.2:Error interno XMS.\n
6.3:Se soporta INT 2F AX=4309\n
6.4:Versi¢n XMS
6.5:Versi¢n controlador XMS
6.6:Estado de HMA
6.7:existe
6.8:no existe
6.9:Estado de la l¡nea A20
6.10:habilitado
6.11:deshabilitado
6.12:Memoria XMS libre
6.13:Bloque libre XMS m s grande
6.14:Referencias (handles) libres
6.15: Bloque  Handle    Tama¤o  Bloqueos\n
#    ------- --------  --------  -------
6.16:Memoria superior libre
6.17:Bloque de memoria superior m s grande
6.18:No hay memoria superior disponible\n
# help message
7.0:FreeDOS MEM versi¢n %d.%d\n
7.1:Muestra la cantidad de memoria ocupada y libre del sistema.
7.2:Sintaxis: MEM [/E] [/F] [/C] [/D] [/U] [/X] [/P] [/?]
7.3:/E  Devuelve informaci¢n sobre memoria expandida (EMS)
7.4:/F  Listado completo de bloques de memoria
7.5:/C  Clasificar los m¢dulos de memoria en el primer MB
7.6:/D  Lista los controladores de dispositivo en memoria
7.7:/U  Lista los programas en memoria convencional y superior
7.8:/X  Devuelve informaci¢n sobre memoria extendida (XMS)
7.9:/P  Realiza una pausa despu‚s de cada pantalla completa
7.10:/?  Muestra este mensaje de ayuda
8.0:\nPulse <Enter> para continuar o <Esc> para salir . . .
