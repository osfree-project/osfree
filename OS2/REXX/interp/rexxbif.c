
#include <stdio.h>
#include <limits.h>
#include "rexxbif.h"

#ifdef EXTERNAL_TO_REGINA
#define exiterror __regina_exiterror
void DropStreng( streng *str )
{
   if ( str )
      free( str );
}

streng *MakeStreng( int num )
{
   streng *out = (streng *)malloc( sizeof(streng) + num );
   if ( out != NULL )
   {
      out->len = out->max = num;
      if ( num )
      {
         memset( out->value, '$', num );
      }
   }
if ( out == NULL ) fprintf(stderr,"ERROR allocating RXSTRING\n");
   return out;
}
void exiterror( int errorno, int suberrorno, ... )
{
}
#endif

int Rexx_x2d( const tsd_t *TSD, const streng *hex, int *error )
{
   int dec=0,i;
   char c;

   TSD = TSD; /* keep compiler happy */

   if ( PSTRENGLEN(hex) == 0 )
   {
      *error = 1;
      return -1;
   }
   for ( i = 0; i < PSTRENGLEN(hex); i++ )
   {
      dec <<= 4;
      c = hex->value[i];
      if ( c >='0'&& c<='9')
         dec += c-'0';
      else if ( c >= 'A' &&c <= 'F' )
         dec += c-'A'+10;
      else if( c >='a' && c <='f' )
         dec += c-'a'+10;
      else
      {
         *error = 1;
         return -1;
      }
   }
   *error = 0;
   return dec;
}

streng *Rexx_right( const tsd_t *TSD, streng *str, int length, char padch )
{
   streng *out = MAKESTRENG( length );
   int i,j;
   if ( out )
   {
      for ( j = 0; PSTRENGLEN(str) > j; j++) ;
      for (i=length-1,j--;(i>=0)&&(j>=0);out->value[i--]=str->value[j--]) ;
      for (;i>=0;out->value[i--]=padch) ;
      out->len = length;
   }
   return out;
}

streng *Rexx_d2x( const tsd_t *TSD, int num )
{
   streng *out;
   out = MAKESTRENG( 100 ); /* use 100 as the largest length of a hex value we will ever get */
   if ( out )
   {
      out->len = sprintf( PSTRENGVAL(out), "%X", num );
   }
   return out;
}

streng *Rexx_upper( const tsd_t *TSD, streng *str )
{
   int i;

   for ( i = 0; i < PSTRENGLEN( str ); i++ )
   {
      if ( islower( str->value[i] ) )
         str->value[i] = (char)toupper( str->value[i] );
   }
   return str;
}
