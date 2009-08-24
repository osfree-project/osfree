/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dblqueue.hpp"
#include "regexp.hpp"

// constructor: initialize the double queue
doubleQueue::doubleQueue(int n) : memory(NULL), size(n), head(n/2), tail((n/2)+1)
{
  memory = (int*) malloc(sizeof(int)*size);
  put(SCAN);  // insert delimiter
}

// destructor: free memory
doubleQueue::~doubleQueue()
{
  if (memory) free(memory);
}

/**************************************************/
/* doubleQueue::push                              */
/*                                                */
/* push an element at the beginning of the queue. */
/**************************************************/
void doubleQueue::push(int value)
{
  memory[head] = value;
  if (head == 0) enlarge();
  head--;
}

/*******************************************/
/* doubleQueue::put                        */
/*                                         */
/* put an element at the end of the queue. */
/*******************************************/
void doubleQueue::put(int value)
{
  memory[tail] = value;
  tail++;
  if (tail == size) enlarge();
}

/****************************************************/
/* doubleQueue::pop                                 */
/*                                                  */
/* get the first element of the queue and remove it */
/* from the queue.                                  */
/****************************************************/
int doubleQueue::pop()
{
  int value = 0;

  if (head < tail) {
    head++;
    value = memory[head];
  }

  return value;
}

/*******************************************************/
/* doubleQueue::enlarge                                */
/*                                                     */
/* the queue is an array holding the elements. if the  */
/* head or tail pointer pass the limits of the array,  */
/* it needs to be enlarged. reallocation is used here. */
/*******************************************************/
void doubleQueue::enlarge()
{
  int *newmemory = (int*) malloc(sizeof(int)*2*size);
  int *oldmemory = memory;
  int offset = size>>2;

  if (newmemory) {
//    memset(newmemory,0x00,sizeof(int)*2*size);
    memcpy(newmemory+offset,memory,size*sizeof(int));
    head+=offset;
    tail+=offset;
    size<<=1;
    memory = newmemory;
    free(oldmemory);
  }
}
