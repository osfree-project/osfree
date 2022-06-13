/**************************************************************************
 *
 *  Copyright 1994, Christina Lau
 *
 *  This file is part of Object-Orientated Programming using SOM and DSOM.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include <repostry.xh>
#include <containd.xh>
#include <stdio.h>
#include <intfacdf.xh>          // InterfaceDef include
#include <attribdf.xh>          // AttributeDef include
#include <constdef.xh>          // ConstantDef include
#include <typedef.xh>           // TypeDef include
#include <string.h>

Environment *ev; 

//****************************************
// Prototypes
//****************************************
void displayInterfaceDef(InterfaceDef *);
void displayTypeDef(TypeDef *);
void displayConstantDef(ConstantDef *);
void displayTypeCode(TypeCode);
void printType(TypeCode);

//******************************************************
//  Simple Interface Repository Browser
//******************************************************
int main(int argc, char *argv[], char *envp[])                                   
{                                                                            
   Repository              *repo;                                            
   _IDL_SEQUENCE_Contained allObj;                                           
   short                   i ;                                               
                                                                             
   ev = SOM_CreateLocalEnvironment();                                        
                                                                             
   repo = new Repository();                                                  
   allObj = repo->contents(ev, "all", TRUE);                                 
                                                                             
   for (i=0; i < sequenceLength(allObj); i++ )                               
   {                                                                         
      Contained *contained;                                                  
      Description desc;                                                      
                                                                             
      contained = sequenceElement(allObj,i);                                 
      desc = contained->describe(ev);                                        
                                                                             
      if (strcmp(desc.name, "InterfaceDescription") == 0)                    
      {                                                                      
         displayInterfaceDef( (InterfaceDef *) contained);                   
      }                                                                      
   }                                                                         
}                                                                            

void displayInterfaceDef(InterfaceDef *intdef)
{
   short                    i, j;
   FullInterfaceDescription fid;
   TypeCode                 tc;
   Description              desc;
   InterfaceDescription     *id;

   _IDL_SEQUENCE_Contained  allObj;

   desc = intdef->describe(ev);
   id = (InterfaceDescription *) desc.value._value;
   printf("Interface Name: %s\n", id->name);

   //******************************************************
   // An Interface Defn can contain ConstantDef, TypeDef,
   // ExceptionDef, AttributeDef and OperationDef.
   // In the following, we displayed TypeDef and ConstantDef
   //******************************************************
   allObj = intdef->contents(ev, "all", TRUE);
   for (i=0; i < sequenceLength(allObj); i++ )
   {
     Contained *contained;
     Description desc;
                                                         
     contained = sequenceElement(allObj,i);
     desc = contained->describe(ev);

     if (strcmp(desc.name, "ConstantDescription") == 0)
     {
       displayConstantDef( (ConstantDef *) contained);
     }
     if (strcmp(desc.name, "TypeDescription") == 0)
     {
       displayTypeDef( (TypeDef *) contained);
     }
   }

   // Get a description of all the methods and attributes in the IR
   fid = intdef->describe_interface(ev);

   //***********************************************************
   // Display all the attributes in this Interface Defn
   //*********************************************************** 
   _IDL_SEQUENCE_AttributeDescription attrd;
   attrd = fid.attributes;

   printf("List of attributes\n");
   for (i=0; i< sequenceLength(attrd); i++)
   {
     printf("<Attribute Name:> %s", sequenceElement(attrd,i).name);
     printf(" <Type:> ");
     printType(sequenceElement(attrd,i).type);
                                                                      
     if (sequenceElement(attrd,i).mode == AttributeDef_READONLY)
     {
        printf(" (readonly)");
     }
     printf("\n");
   }
 
   //*************************************************************
   // Display all the methods in this Interface Defn
   //*************************************************************
   _IDL_SEQUENCE_OperationDescription opd;
   opd = fid.operation;

   printf("\nList of methods\n");
   for (i=0; i < sequenceLength(opd); i++)
   {
     printf("<Method Name:> %s <Return Type:> ",
             sequenceElement(opd,i).name);
     printType(sequenceElement(opd,i).result);
     printf("\n");

     //************************************************** 
     // Display all parameters
     //**************************************************
     _IDL_SEQUENCE_ParameterDescription parmd;
     parmd = sequenceElement(opd,i).parameter;
     for (j=0; j < sequenceLength(parmd); j++ )
     {
        switch (sequenceElement(parmd,j).mode) 
        {
          case ParameterDef_IN:
              printf("  in ");
              break;
          case ParameterDef_OUT:
              printf("  out ");
              break;
          default:
              printf(" inout ");
        } 
        printf("%s ", sequenceElement(parmd,j).name);
        printType(sequenceElement(parmd,j).type);
        printf("\n");
     } 
   }

   printf("\n*************************************************\n");
}

void displayTypeDef(TypeDef *typdef)
{
   Description      desc;
   TypeDescription  *td;
                                                   
   desc = typdef->describe(ev);
   td = (TypeDescription *) desc.value._value;
                                                   
   printf("Typedef %s defined in %s\n", td->name, td->defined_in);
   displayTypeCode(td->type);
   printf("\n");
}

void displayConstantDef(ConstantDef *condef)
{
   Description         desc;
   ConstantDescription *cd;

   desc = condef->describe(ev);
   cd = (ConstantDescription *) desc.value._value;

   printf("Constant %s defined in %s\n", cd->name, cd->defined_in);
   printf("<Type:> ");

   switch (TypeCode_kind(cd->value._type, ev))
   {
      case tk_string:
         printf("string <Value:> %s\n", *((string *) cd->value._value));
         break;
      case tk_long:
         printf("long <Value:> %ld\n", *((long *) cd->value._value));
         break;
      case tk_float:
         printf("float <Value:> %f\n", *((float *) cd->value._value));
         break;
      case tk_ushort:
         printf("unsigned short <Value:> %d\n", *((unsigned short *) cd->value._value));
         break;
      case tk_ulong:
         printf("unsigned long <Value:> %d\n", *((unsigned long *) cd->value._value));
         break;
   } 
   printf("\n");
}

void printType(TypeCode tc)
{
  switch (TypeCode_kind(tc, ev))                             
  {
    case tk_null:
      printf("null ");
      break;
    case tk_void:
      printf("void ");
      break;
    case tk_short:
      printf("short ");
      break;                                               
    case tk_long:
      printf("long ");
      break;                                               
    case tk_ushort:
      printf("unsigned short ");
      break;                                               
    case tk_ulong:
      printf("unsigned long ");
      break;                                               
    case tk_float:
      printf("float ");
      break;                                               
    case tk_double:
      printf("double ");
      break;                                               
    case tk_boolean:
      printf("boolean ");
      break;                                               
    case tk_char:
      printf("char ");
      break;                                               
    case tk_octet:
      printf("octet ");
      break;                                               
    case tk_any:
      printf("any ");
      break;                                               
    case tk_TypeCode:
      printf("TypeCode ");
      break;
    case tk_Principal:
      printf("Principal ");
      break;
    case tk_objref:
      printf("Object Reference ");
      break;
    case tk_struct:
      printf("struct ");
      break;
    case tk_union:
      printf("union ");
      break;
    case tk_enum:
      printf("enum ");
      break;
    case tk_string:
      printf("string ");
      break;
    case tk_sequence:
      printf("sequence ");
      break;
    case tk_array:
      printf("array ");
      break;
    case tk_pointer:
      printf("pointer ");
      break;
    case tk_self:
      printf("Self ");
      break;
    case tk_foreign:
      printf("foreign ");
      break;
    default:
      printf("not valid data type\n");
      break;                       
  }                                  
}                                    

//********************************************************* 
// Navigate TypeCode to display complex data types
//*********************************************************
void displayTypeCode(TypeCode tc)
{
  TypeCode membertc, seqtc;
  short    i;
  any      parm;
  long     len;
  string   memname;

  switch (TypeCode_kind(tc, ev))
  {
    case tk_short:
        printf("short\n");
        break;
 
    case tk_long:
        printf("long\n");
        break;
 
    case tk_ushort:
        printf("unsigned short\n");
        break;
                                
    case tk_ulong:
        printf("unsigned long\n");
        break;
    
    case tk_float:
        printf("float\n");
        break;
                                                             
    case tk_double:
        printf("double\n");
        break;
                                                             
    case tk_boolean:
        printf("boolean\n");
        break;
                                                             
    case tk_char:
        printf("char\n");
        break;
 
    case tk_octet:
        printf("octet\n");
        break;
     
    case tk_any:
        printf("any\n");
        break;

    case tk_objref:
        parm = TypeCode_parameter(tc, ev, 0);                    
        printf("%s\n", *((string *)parm._value));
        break;
 
    case tk_struct:
        parm = TypeCode_parameter(tc, ev, 0);
        printf("<Structure:> %s\n", *((string *)parm._value));
        //**************************************************
        // Get the name and type for each struct member
        //**************************************************
        for (i=1; i < TypeCode_param_count(tc,ev); i+=2 )
        {
           parm = TypeCode_parameter(tc,ev,i);
           printf("  <Member Name:> %s", *((string *)parm._value));

           parm = TypeCode_parameter(tc,ev,i+1);
           printf("  <Member Type:> ");
           //*********************************************** 
           // Recursively call displayType to display the
           // type of each member
           //***********************************************
           displayTypeCode( *( (TypeCode *)parm._value ) );
        } 
        printf("\n");
        break;
 
    case tk_union:
        parm = TypeCode_parameter(tc, ev, 0);                 
        printf("<Union:> %s\n", *((string *)parm._value));
        //************************************************ 
        // Get the union switch type 
        //************************************************  
        parm = TypeCode_parameter(tc, ev, 1);
        printf("<Discriminator Type:> ");
        printType( *((TypeCode *)parm._value) );
        
        for (i=2; i < TypeCode_param_count(tc,ev); i+=3) 
        {
           // Get label value
           parm = TypeCode_parameter(tc,ev,i);
           printf("\n  <Label:> %ld ", *((long *)parm._value) );
           
           // Get member name
           parm = TypeCode_parameter(tc,ev,i+1);
           memname = *((string *)parm._value);
 
           // Get member type
           parm = TypeCode_parameter(tc,ev,i+2);
           printf("<Member Type:> ");
           printType( *((TypeCode *)parm._value) );

           printf(" <Member Name:> %s", memname);
        }
        printf("\n\n");
        break;

    case tk_enum:
        parm = TypeCode_parameter(tc, ev, 0);
        printf("<Enum:> %s\n", *((string *)parm._value));
        //**************************************************
        // Get the name for all enumerators
        //**************************************************
        for (i=1; i < TypeCode_param_count(tc,ev); i++ )
        {
           parm = TypeCode_parameter(tc,ev,i);
           printf("  <enumerator:> %s\n", *((string *)parm._value));
        }
        printf("\n");
        break;
 
    case tk_string:
        parm = TypeCode_parameter(tc, ev, 0);
        len = *((long *)parm._value);
        if (len != 0) 
        {
           printf("string, maximum length: %ld\n", len);
        } 
        else
        {        
           printf("string\n");
        }
        break;
 
    case tk_sequence:
        printf("sequence\n");
        //*************************************************
        // First parm contains the sequence type
        //*************************************************
        parm = TypeCode_parameter(tc, ev, 0);
        printf("<Sequence Type:> ");
        printType( *((TypeCode *)parm._value) );

        //************************************************* 
        // Second parm contains the sequence maximum len
        //*************************************************
        parm = TypeCode_parameter(tc,ev,1);
        len = *((long *)parm._value);
        if (len != 0) 
        {
           printf(" <Maximum Length:> %ld\n", len);
        } 
        printf("\n");
        break;
     
    case tk_array:
        printf("array\n");
        //*********************************************** 
        // First parm contains the array type
        //***********************************************
        parm = TypeCode_parameter(tc, ev, 0);
        printf("<Array Type:> ");
        printType( *((TypeCode *)parm._value) );

        //***********************************************
        // Second parm contains array size
        //***********************************************
        parm = TypeCode_parameter(tc, ev, 1);
        printf("<Size:> %ld\n", *((long *)parm._value));
        break;
        
    default: 
        printf("not supported ...");
        break;
  }
}
