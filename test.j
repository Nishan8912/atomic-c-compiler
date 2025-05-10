global int a;
global int b;
global int result;
global int arr[10];

function compute(int x, int y)
{
  local int i;
  local int temp;

  i = 0;
  do {
    call printf("Index: %d\n", i);
    arr[i] = (x * y) + i;
    i = i + 1;
  } while (i < 5);

  i = 0;
  result = 0;
  while (i < 5) do {
    temp = arr[i];
    result = result + temp;
    i = i + 1;
  }

  call printf("Result is: %d\n", result);
}

program {
  a = 6;
  b = 4;

  if ((a * b) >= 24) then {
    call printf("Product is large enough\n");
  } else {
    call printf("Product too small\n");
  }

  call compute(a, b);     

  if ((result % 2) != 0) then {
    call printf("Result is odd\n");
  } else {
    call printf("Result is even\n");
  }

  call printf("Done.\n");
}
