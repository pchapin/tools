
/* This program just emits the basic three letter word units. */

#include <stdio.h>

int Is_Vowel(int Letter)
  {
    if (Letter == 'A' ||
        Letter == 'E' ||
        Letter == 'I' ||
        Letter == 'O' ||
        Letter == 'U') return 1;
    return 0;
  }

int main(void)
  {
    int L_1, L_2, L_3;

    for (L_1 = 'A'; L_1 <= 'Z'; L_1++) {
      if (Is_Vowel(L_1)) continue;

      for (L_2 = 'A'; L_2 <= 'Z'; L_2++) {
        if (!Is_Vowel(L_2)) continue;

        for (L_3 = 'A'; L_3 <= 'Z'; L_3++) {
          if (Is_Vowel(L_3)) continue;
            printf("%c%c%c\n", L_1, L_2, L_3);
        }
      }
    }

    return 0;
  }
