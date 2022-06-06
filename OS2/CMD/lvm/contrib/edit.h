/* edit.h */
int  DisplayString (char *string, int row, int column, int len );
int  DisplayStringWithAttribute (char *string, int row, int column, int len, int attr );
int  DisplayChars(char ch, int row, int column, int a, int b );
int  DisplayCharsWithAttribute(char ch, int row, int column, int row_length, int  number_of_rows, int attr );
void * SaveDisplayBlock (int row, int column, int row_length, int  number_of_rows);
void * RestoreDisplayBlock(int row, int column, int row_length, int  number_of_rows, char *buf);

int  DisplayAttributes (int row, int column, int row_length, int  number_of_rows, int attr);
uint ReverseAttributes ( uint row, uint column,uint max_string_length, uint bb );
