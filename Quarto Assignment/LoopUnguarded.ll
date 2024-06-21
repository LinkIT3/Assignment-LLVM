; ModuleID = 'LoopUnguarded.bc'
source_filename = "LoopUnguarded.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @calcoli(i32 noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, ptr noundef %4) #0 {
  br label %6

6:                                                ; preds = %19, %5
  %.0 = phi i32 [ 0, %5 ], [ %20, %19 ]
  %7 = icmp slt i32 %.0, %0
  br i1 %7, label %8, label %21

8:                                                ; preds = %6
  %9 = sext i32 %.0 to i64
  %10 = getelementptr inbounds i32, ptr %2, i64 %9
  %11 = load i32, ptr %10, align 4
  %12 = sdiv i32 1, %11
  %13 = sext i32 %.0 to i64
  %14 = getelementptr inbounds i32, ptr %3, i64 %13
  %15 = load i32, ptr %14, align 4
  %16 = mul nsw i32 %12, %15
  %17 = sext i32 %.0 to i64
  %18 = getelementptr inbounds i32, ptr %1, i64 %17
  store i32 %16, ptr %18, align 4
  br label %19

19:                                               ; preds = %8
  %20 = add nsw i32 %.0, 1
  br label %6, !llvm.loop !6

21:                                               ; preds = %6
  br label %22

22:                                               ; preds = %34, %21
  %.1 = phi i32 [ 0, %21 ], [ %35, %34 ]
  %23 = icmp slt i32 %.1, %0
  br i1 %23, label %24, label %36

24:                                               ; preds = %22
  %25 = sext i32 %.1 to i64
  %26 = getelementptr inbounds i32, ptr %1, i64 %25
  %27 = load i32, ptr %26, align 4
  %28 = sext i32 %.1 to i64
  %29 = getelementptr inbounds i32, ptr %3, i64 %28
  %30 = load i32, ptr %29, align 4
  %31 = add nsw i32 %27, %30
  %32 = sext i32 %.1 to i64
  %33 = getelementptr inbounds i32, ptr %4, i64 %32
  store i32 %31, ptr %33, align 4
  br label %34

34:                                               ; preds = %24
  %35 = add nsw i32 %.1, 1
  br label %22, !llvm.loop !8

36:                                               ; preds = %22
  ret void
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
