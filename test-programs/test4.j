global int x;
global int ynotused;
global int arr[100];

function makePattern(string s1, string s2, int val)
{
   local int y;
   call printf("Arg val is: %d\n", val);
   call printf("\nPrinting a pattern\n");
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

function arrayFun(int size, int startVal)
{
   local int i;
   local int sum;
   i = 0; 
   while (i < size) do {
      arr[i] = startVal;
      startVal = startVal + 1;
      i = i + 1;
   }
   i = 0; 
   while (i < size) do {
      call printf("arr[%d]=%d\n",i,arr[i]);
      i = i + 1;
   }
   i = 0; 
   sum = 0;
   while (i < size) do {
      sum = sum + arr[i];
      i = i + 1;
   }
   call printf("array sum = %d\n", sum);
}

program {
   call printf("Enter value for x: \n");
   call readInt();
   x = returnvalue;
   if (x > 100) then {
      call printf("x is over 100!\n");
   } else {
      call printf("x is 100 or less!\n");
   }
   call makePattern("hello", "goodbye", 42);
   call printf("Enter starting array value: \n");
   call readInt();
   x = returnvalue;
   call arrayFun(20,x);
   call printf("Program done.\n");
}
