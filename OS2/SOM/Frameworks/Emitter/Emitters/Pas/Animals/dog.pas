(******************************************************************************)
(*  Pascal SOM Samples and Examples                                           *)
(*                                                                            *)
(*  dog.pas - Interact with SOM class DOG                                     *)
(*                                                                            *)
(*  This program demonstrates creating instances and sending messages to a    *)
(*  SOM class.                                                                *)
(******************************************************************************)

Uses DogInt;

Var
  Dog: TDog;
Begin
  Dog:=TDog.Create;
  Dog.Display;
  Dog.Talk;
  Dog.somPrintSelf;
  Dog.Destroy;
End.

