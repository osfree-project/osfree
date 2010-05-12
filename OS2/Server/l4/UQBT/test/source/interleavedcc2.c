/* Test of interleaved integer and floating point defines and uses */
int interleaved(int i, float a, float b);

int main()
{
    printf("Call with  1, 2.0, 3.0: %d\n", interleaved( 1, 2.0, 3.0));
    printf("Call with -1, 2.0, 3.0: %d\n", interleaved(-1, 2.0, 3.0));
    printf("Call with  1, 2.0, 2.0: %d\n", interleaved( 1, 2.0, 2.0));
    printf("Call with -1, 2.0, 2.0: %d\n", interleaved(-1, 2.0, 2.0));
    return 0;
}
