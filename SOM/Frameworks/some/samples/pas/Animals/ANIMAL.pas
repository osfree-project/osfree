(******************************************************************************)
(*  Pascal SOM Samples and Examples                                           *)
(*                                                                            *)
(*  animal.pas - Interact with SOM class ANIMAL                               *)
(*                                                                            *)
(*  This program demonstrates creating instances and sending messages to a    *)
(*  SOM class.                                                                *)
(******************************************************************************)

Uses AnimalInt;

Var
  Animal: TAnimal;
Begin
  {Create animal object}
  Animal:=TAnimal.Create;
  {Display info about animal}
  Animal.Display;
  {Ask animal to talk}
  Animal.Talk;
  {Ask somPrintSelf method from parent SOMObject class}
  Animal.somPrintSelf;
  {Destroy animal}
  Animal.Destroy;
End.

