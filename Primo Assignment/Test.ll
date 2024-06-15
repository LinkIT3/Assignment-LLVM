; int foo(int e, int a) {
;   int b = a + 0;
;   int c = b * 1;
;   b = e * 4;
;   int d = b / 4;
;   int f = 5 * d;
;   int g = f + 0;
;   int h = g / 1;
;   int l = 0 / h;
;   int m = h + l;
;   int n = e + 5;
;   int o = n - 5;
;   return o * d;
; }

define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %1, 0
  %4 = mul nsw i32 %3, 1
  %5 = mul nsw i32 %0, 4
  %6 = udiv i32 %5, 4
  %7 = mul nsw i32 5, %6
  %8 = add nsw i32 %7, 0
  %9 = udiv i32 %8, 1
  %10 = udiv i32 0, %9
  %11 = add nsw i32 %9, %10
  %12 = add nsw i32 %0, 5
  %13 = sub nsw i32 %12, 5 
  %14 = mul nsw i32 %13, %6
  ret i32 %14
}
