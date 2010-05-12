// QAD program to read a .cc file and change the labels to that they are
// unique. Application: translated (.cc) files from the ML version of the
// New Jersey Machine Code Toolkit, where the source match file (.m) has
// more than one match line per function.

// Assumptions:
// 1) All translated match statements have curly brackets that extend to the
// first column of the file enclosing the match code.
// 2) Anything starting with "MATCH_f" or "MATCH_l" is a label
// 3) Labels can be made unique by appending a single letter, e.g.
//   MATCH_finished_17 -> MATCH_finished_17a
// 4) lines do not exceed 199 characters
// 5) Only one label reference will appear per line

#include <stdio.h>
#include <string.h>
#include <ctype.h>          // For isalnum()

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("Usage: fixLabels <infile> <outfile>\n");
        return 1;
    }

    FILE *f1, *f2;

    f1 = fopen(argv[1], "r");
    if (f1 == NULL) {
        printf("Could not open %s for reading\n", argv[1]);
        return 2;
    }

    f2 = fopen(argv[2], "w");
    if (f1 == NULL) {
        printf("Could not open %s for writing\n", argv[2]);
        return 3;
    }

    char buf[200];
    char buf2[201];

    int braceCount = 0;     // Count of braces in first column only!!
    char letter = 'a';      // Letter to append to a label
    while (!feof(f1)) {
        fgets(buf, 200, f1);
        if (feof(f1)) break;
        if (buf[0] == '{')
            braceCount++;
        if (buf[0] == '}') {
            braceCount--;
            if (braceCount == 0)
                // We are outside a function
                letter = 'a';           // Prevent "letter creep"
            if (braceCount == 1)
                // We are between match statements; inc the letter
                letter++;
        }
        if (braceCount > 1) {
            // We are in a match statement. Modify all labels
            char* p = strstr(buf, "MATCH_");
            if (p) {
                p += 6;                             // Skip the "MATCH_"
                if ((*p == 'f') || (*p == 'l')) {
                    // Skip to the end of the label
                    while (isalnum(*p) || (*p == '_'))
                        p++;
                    strncpy(buf2, buf, (p - buf)); 
                    char* q = buf2 + (p - buf);
                    *q++ = letter;
                    strcpy(q, p);
                    fputs(buf2, f2);
                    continue;
                }
            }
        }
        fputs(buf, f2);
    }

    fclose(f1);
    fclose(f2);
    return 0;
}

