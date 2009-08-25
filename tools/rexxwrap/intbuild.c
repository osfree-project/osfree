/*
 * This program takes a Rexx program, reads it into memory, encrypts it
 * compresses it and uuencodes it, and creates a C program that uses the
 * in-memory image to pass to RexxStart()
 */
#if defined(HAVE_CONFIG_H)
# include "config.h"
#else
# include "defines.h"
#endif

#include <stdio.h>
#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif
#if defined(HAVE_MALLOC_H)
# include <malloc.h>
#endif
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(HAVE_STRING_H)
# include <string.h>
#endif
#if defined(HAVE_MEM_H)
# include <mem.h>
#endif
#if defined(HAVE_PROCESS_H)
# include <process.h>
#endif
#if defined(USE_ZLIB)
# include "zlib.h"
#endif
#if defined(USE_DES)
# include "des.h"
#endif

#include <time.h>

#if defined(USE_REGINA)
# define DEFAULT_ENV "SYSTEM"
#elif defined(USE_OREXX) || defined(USE_OS2REXX)
# define DEFAULT_ENV "CMD"
#else
# define DEFAULT_ENV "COMMAND"
#endif

int encode(unsigned char *out, unsigned char *in, int len)
{
   unsigned char three[3];
   unsigned char tmp1,tmp2;
   int i,j;

   for (i=0,j=0;i<len;i+=3,j+=4)
   {
      three[0] = in[i];
      if (i+1 < len)
         three[1] = in[i+1];
      else
         three[1] = '\0';
      if (i+2 < len)
         three[2] = in[i+2];
      else
         three[2] = '\0';
      out[j] = (three[0]>>2 & 0x3f) + ' ';
      tmp1 = ((three[0] & 0x03)<<4) & 0x3f;
      tmp2 = (three[1]>>4) & 0x3f;
      out[j+1] = ((tmp1 + tmp2) & 0x3f) + ' ';
      tmp1 = (three[1]<<2) & 0x3c;
      tmp2 = (three[2]>>6) & 0x03;
      out[j+2] = ((tmp1 + tmp2) & 0x3f) + ' ';
      out[j+3] = (three[2] & 0x3f)+' ';
   }
   return j;
}


int main(int argc, char *argv[])
{
   FILE *fp=NULL,*fpout=NULL;
   unsigned char buf[100]; /* no line in loader.c is longer than this */
   unsigned char *original_code=NULL;
   unsigned char *encrypted_code=NULL;
   unsigned char *compressed_code=NULL;
   unsigned char *encoded_code=NULL;
   long original_length=0;
   int off[3] = {0, 2, 1};
   int rem,i,j,len,rc;
   unsigned char *ptr;
   long compressed_length;
   long encoded_length;
   long encrypted_length;
   long size;
   char outfile[100];
#if defined(USE_DES)
   des_cblock deskey;
   des_cblock in_cblock;
   des_cblock out_cblock;
   des_key_schedule ks;
   char str[9];
   char *fixed_str=NULL;
   int maxi=122, mini=65, num_chunks, num_extra;
# if defined(HAVE_RANDOM)
   static int seed=0;
# else
   static unsigned int seed=0;
# endif
#endif
#ifdef USE_DES
# ifdef UNIX
   char *quote="'";
# else
   char *quote="\"";
# endif
#endif

   /*
    * Find the Rexx program(s) and their size(s)
    * Read them in in one chunk and compress, encrypt, and uuencode
    * them.
    */
   argc = argc; /* keep compiler happy */
   fp = fopen(argv[3],"rb");
   if (fp == NULL)
   {
      fprintf(stderr,"Cannot open %s for reading\n",argv[3]);
      exit(1);
   }
   fseek( fp, 0, SEEK_END );
   size = ftell( fp );
   rewind( fp );
   original_code = (unsigned char *)malloc(size+5);
   if (original_code == NULL)
   {
      fprintf(stderr,"No memory at line %d\n",__LINE__);
      exit(1);
   }
   memset(original_code,' ',size+5);
   original_length = size;
   if ((long)fread((void *)original_code,sizeof(unsigned char),size,fp) != size)
   {
      fprintf(stderr,"Error reading %ld bytes from %s\n",(long)size,argv[3]);
      if (original_code)
      {
         free(original_code);
         original_code = NULL;
      }
      fclose(fp);
      exit(1);
   }
   fclose(fp);

#if defined(USE_ZLIB)
   /*
    * Compress original_code to produce compressed_code
    */
   compressed_length = (12 + ((original_length * 101 / 100)));
   compressed_code = (unsigned char *)malloc(compressed_length);
   if (compressed_code == NULL)
   {
      fprintf(stderr,"No memory at line %d\n",__LINE__);
      if (original_code)
      {
         free(original_code);
         original_code = NULL;
      }
      exit(1);
   }
   rc = compress(compressed_code, (uLongf *)&compressed_length,
                 original_code, original_length);
   if (rc != Z_OK)
   {
      fprintf(stderr,"Error compressing program code\n");
      if (original_code)
      {
         free(original_code);
         original_code = NULL;
      }
      if (compressed_code)
      {
         free(compressed_code);
         compressed_code = NULL;
      }
      exit(1);
   }
#else
   compressed_length = original_length;
   compressed_code = original_code;
#endif

#if defined(USE_DES)
   /*
    * Encrypt compressed_code to produce encrypted_code
    */
   fixed_str = getenv("REXXWRAPPER_KEY");
   if ( fixed_str == NULL )
   {
# if defined(HAVE_RANDOM)
      srandom(seed=((time((time_t *)0)+getpid())%(3600*24))) ;
      for ( i=0; i<8; )
      {
         str[i] = (random() % (maxi-mini+1)) + mini ;
         if ( str[i] != 39 && str[i] != 34 && str[i] != 96 )
            i++;
      }
# else
      srand(seed=((time((time_t *)0)+getpid())%(3600*24))) ;
      for ( i=0; i<8; )
      {
         str[i] = (rand() % (maxi-mini+1)) + mini ;
         if ( str[i] != 39 && str[i] != 34 && str[i] != 96 )
            i++;
      }
# endif
      str[8] = '\0';
      fixed_str = str;
   }
   des_string_to_key(fixed_str,&deskey);
   fprintf(stderr,"   The generated key for this program is: %s\n",fixed_str);
   fprintf(stderr,"   If you need to dump the original code from the %s executable, run:\n    %s %s--key=%s%s\n",argv[2],argv[2],quote,fixed_str,quote);
   encrypted_code = (unsigned char *)malloc(compressed_length + 8);
   if (encrypted_code == NULL)
   {
      fprintf(stderr,"No memory at line %d\n",__LINE__);
      if (compressed_code)
      {
         free(compressed_code);
         compressed_code = NULL;
      }
      exit(1);
   }
   /*
    * Convert the DES key into a DES key schedule
    */
   des_set_key((des_cblock *)&deskey,ks);
   /*
    * Now encrypt compressed_code in 8byte chunks
    */
   encrypted_length = compressed_length;
   num_chunks = encrypted_length / 8;
   for (i=0;i<num_chunks;i++)
   {
      memcpy(in_cblock,compressed_code+(i*8),8);
      des_ecb_encrypt(&in_cblock,&out_cblock,ks,DES_ENCRYPT);
      memcpy(encrypted_code+(i*8),out_cblock,8);
   }
   num_extra = encrypted_length % 8;
   if (num_extra > 0)
   {
      memset(in_cblock,' ',8);
      memcpy(in_cblock,compressed_code+(i*8),num_extra);
      des_ecb_encrypt(&in_cblock,&out_cblock,ks,DES_ENCRYPT);
      memcpy(encrypted_code+(i*8),out_cblock,8);
      encrypted_length += (8-num_extra);
   }
   if (compressed_code)
   {
      free(compressed_code);
      compressed_code = NULL;
# if !defined(USE_ZLIB)
      original_code = NULL;
# endif
   }
#else
   fprintf(stderr,"   If you need to dump the original code from the %s executable, run:\n %s --key=1\n",argv[2],argv[2]);
   encrypted_length = compressed_length;
   encrypted_code = compressed_code;
#endif

   /*
    * UUencode encrypted_code to produce uuencoded_code
    */

   rem = encrypted_length % 3;
   encoded_length = ((encrypted_length+off[rem]) * 4) / 3;
   encoded_code = (unsigned char *)malloc(encoded_length+off[rem]);
   if (encoded_code == NULL)
   {
      fprintf(stderr,"No memory at line %d\n",__LINE__);
      exit(1);
   }
   rc = encode(encoded_code, encrypted_code, encrypted_length);
   if (rc != encoded_length)
   {
      fprintf(stderr,"Error encoding; encrypted %ld encoded %ld\n",encrypted_length, encoded_length);
      exit(1);
   }


   /*
    * Read loader.c and replace occurrences of %%keyword%% with the
    * appropriate values generated above and write ou the new file
    */
   fp = fopen(argv[1],"rb");
   if (fp == NULL)
   {
      fprintf(stderr,"Cannot open %s for reading\n",argv[1]);
      if (original_code)
      {
         free(original_code);
         original_code = NULL;
      }
      exit(1);
   }
   sprintf(outfile,"%s.c",argv[2]);
   fpout = fopen(outfile,"wb");
   if (fpout == NULL)
   {
      fprintf(stderr,"Cannot open %s for writing\n",outfile);
      if (original_code)
      {
         free(original_code);
         original_code = NULL;
      }
      fclose(fp);
      exit(1);
   }

   while(fgets((char *)buf,sizeof(buf),fp))
   {
      len = strlen((char *)buf);
      if ( len > 2
      &&  memcmp(buf,"%%",2) == 0)
      {
         if (len >= 17 && memcmp(buf+2,"ORIGINAL_LENGTH",15) == 0)
         {
            fprintf(fpout,"long original_length = %ld;\n",original_length);
         }
         else if (len >= 19 && memcmp(buf+2,"COMPRESSED_LENGTH",17) == 0)
         {
            fprintf(fpout,"long compressed_length = %ld;\n",compressed_length);
         }
         else if (len >= 18 && memcmp(buf+2,"ENCRYPTED_LENGTH",16) == 0)
         {
            fprintf(fpout,"long encrypted_length = %ld;\n",encrypted_length);
         }
         else if (len >= 16 && memcmp(buf+2,"ENCODED_LENGTH",14) == 0)
         {
            fprintf(fpout,"long encoded_length = %ld;\n",encoded_length);
         }
         else if (len >= 14 && memcmp(buf+2,"ORIGINAL_KEY",12) == 0)
         {
#if defined(USE_DES)
            fprintf(fpout,"char *original_key = \"");
            for (i=0;i<8;i++)
            {
               if (fixed_str[i] == '\"'
               ||  fixed_str[i] == '\\'
               ||  fixed_str[i] == '\?')
                  fputc('\\',fpout);
               fputc(fixed_str[i],fpout);
            }
            fprintf(fpout,"\";\n");
            fprintf(fpout,"int original_key_length = 8;\n");
#else
            fprintf(fpout,"char *original_key = \"1\";\n");
            fprintf(fpout,"int original_key_length = 1;\n");
#endif
         }
         else if (len >= 14 && memcmp(buf+2,"PROGRAM_NAME",12) == 0)
         {
            fprintf(fpout,"char *ProgramName = \"%s\";\n",argv[2]);
         }
         else if (len >= 21 && memcmp(buf+2,"DEFAULT_ENVIRONMENT",19) == 0)
         {
            fprintf(fpout,"char *DefaultEnvironment = \"%s\";\n", DEFAULT_ENV);
         }
         else if (len >= 17 && memcmp(buf+2,"ENCODED_PROGRAM",15) == 0)
         {
            /* write out uuencoded lines, replacing " with \", ? with \? and \ with \\ */
            for (i=0;i<encoded_length/100;i++)
            {
               fprintf(fpout,"   { 100, \"");
               ptr = (unsigned char *)encoded_code+(i*100);
               for (j=0;j<100;j++)
               {
                  if (ptr[j] == '\"'
                  ||  ptr[j] == '\\'
                  ||  ptr[j] == '\?')
                  {
                     fputc('\\',fpout);
                  }
                  fputc(ptr[j],fpout);
               }
               fprintf(fpout,"\" },\n");
            }
            if (encoded_length % 100 != 0)
            {
               fprintf(fpout,"   { %ld, \"",encoded_length % 100);
               ptr = (unsigned char *)encoded_code+(i*100);
               for (j=0;j<encoded_length % 100;j++)
               {
                  if (ptr[j] == '\"'
                  ||  ptr[j] == '\\'
                  ||  ptr[j] == '\?')
                  {
                     fputc('\\',fpout);
                  }
                  fputc(ptr[j],fpout);
               }
               fprintf(fpout,"\" },\n");
            }
            fprintf(fpout,"   { 0, NULL }");
         }
         else
         {
            fprintf(stderr,"Malformed loader.c. Aborting.\n");
            exit(1);
         }
      }
      else
      {
         fprintf(fpout,"%s",buf);
      }
   }
   fclose(fp);
   fclose(fpout);
   if (original_code)
   {
      free(original_code);
      original_code = NULL;
   }
   if (encoded_code)
   {
      free(encoded_code);
      encoded_code = NULL;
   }
   return 0;
}
