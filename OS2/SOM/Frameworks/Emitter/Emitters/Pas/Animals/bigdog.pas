(******************************************************************************)
(*  Pascal SOM Samples and Examples                                           *)
(*                                                                            *)
(*  bigdog.pas - Interact with SOM class BIGDOG                               *)
(*                                                                            *)
(*  This program demonstrates creating instances and sending messages to a    *)
(*  SOM class.                                                                *)
(******************************************************************************)

Uses BDogInt;

Var
  Dog: TBigDog;
Begin
  Dog:=TBigDog.Create;
  Dog.Display;
  Dog.Talk;
  Dog.somPrintSelf;
  Dog.Destroy;
End.

