; ModuleID = 'algebraic_test.bc'
source_filename = "algebraic_test.ll"

define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %3 = add nsw i32 %0, 0
  %4 = add nsw i32 0, %1
  %5 = add nsw i32 %0, 34
  %6 = add nsw i32 1, %1
  %7 = sub nsw i32 %0, 0
  %8 = sub nsw i32 0, %1
  %9 = sub nsw i32 %0, 34
  %10 = sub nsw i32 1, %8
  %11 = mul nsw i32 %0, 1
  %12 = mul nsw i32 1, %1
  %13 = mul nsw i32 %0, 34
  %14 = mul nsw i32 13, %1
  %15 = sdiv i32 %0, 1
  %16 = sdiv i32 1, %1
  %17 = sdiv i32 %0, 34
  %18 = sdiv i32 4, %16
  ret i32 %0
}
