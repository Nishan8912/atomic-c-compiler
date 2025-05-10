global int x;
global int y;

function makePattern(string s1, string s2, int val)
{    call printf("Printing a pattern\n");
   while (x != 0) do {
      y = 0;
      while (y < x) do {
         call printf("*");
         y = y + 1;
      }
      call printf("\n");
      x = x - 1;
   }
}

program {
   call printf("Enter value for x: ");
   call readInt();
   x = returnvalue;
   if (x > 100) then {
      call printf("%d is over 100! Reducing!\n", x);
      x = x - 100;
   } else {
      call printf("%d is 100 or less!\n", x);
   }
   call makePattern();
   call printf("Program done.\n");
}