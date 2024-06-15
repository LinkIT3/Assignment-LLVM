; ModuleID = 'Test.bc'
source_filename = "Test.ll"

define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %3 = add nsw i32 %1, 0
  %4 = mul nsw i32 %1, 1
  %5 = mul nsw i32 %0, 4
  %6 = shl i32 %0, 2
  %7 = udiv i32 %6, 4
  %8 = lshr i32 %6, 2
  %9 = mul nsw i32 5, %0
  %10 = shl i32 %0, 2
  %11 = add i32 %10, %0
  %12 = add nsw i32 %11, 0
  %13 = udiv i32 %11, 1
  %14 = udiv i32 0, %11
  %15 = add nsw i32 %11, 0
  %16 = add nsw i32 %0, 5
  %17 = sub nsw i32 %16, 5
  %18 = mul nsw i32 %0, %0
  ret i32 %18
}
