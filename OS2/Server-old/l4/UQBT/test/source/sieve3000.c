

void main() {
unsigned int got=0;
unsigned int i,j;
unsigned int ok;
  for (i=3; got<3000; i+=2) {
    ok=1;
    for (j=3; j<i; j+=2) 
      if ((i%j)==0) ok=0;
    if (ok) { printf("%i\n",i); got++; } 
  }
}
