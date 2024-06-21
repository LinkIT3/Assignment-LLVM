; ModuleID = 'LoopGuarded.bc'
source_filename = "LoopGuarded.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @calcoli(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, i32 noundef %4) #0 {
  %6 = icmp sgt i32 %4, 0
  br i1 %6, label %7, label %23

7:                                                ; preds = %5
  br label %8

8:                                                ; preds = %20, %7
  %.0 = phi i32 [ 0, %7 ], [ %19, %20 ]
  %9 = sext i32 %.0 to i64
  %10 = getelementptr inbounds i32, ptr %1, i64 %9
  %11 = load i32, ptr %10, align 4
  %12 = sdiv i32 1, %11
  %13 = sext i32 %.0 to i64
  %14 = getelementptr inbounds i32, ptr %2, i64 %13
  %15 = load i32, ptr %14, align 4
  %16 = mul nsw i32 %12, %15
  %17 = sext i32 %.0 to i64
  %18 = getelementptr inbounds i32, ptr %0, i64 %17
  store i32 %16, ptr %18, align 4
  %19 = add nsw i32 %.0, 1
  br label %20

20:                                               ; preds = %8
  %21 = icmp slt i32 %19, %4
  br i1 %21, label %8, label %22, !llvm.loop !6

22:                                               ; preds = %20
  br label %23

23:                                               ; preds = %22, %5
  %24 = icmp sgt i32 %4, 0
  br i1 %24, label %25, label %40

25:                                               ; preds = %23
  br label %26

26:                                               ; preds = %37, %25
  %.1 = phi i32 [ 0, %25 ], [ %36, %37 ]
  %27 = sext i32 %.1 to i64
  %28 = getelementptr inbounds i32, ptr %0, i64 %27
  %29 = load i32, ptr %28, align 4
  %30 = sext i32 %.1 to i64
  %31 = getelementptr inbounds i32, ptr %2, i64 %30
  %32 = load i32, ptr %31, align 4
  %33 = add nsw i32 %29, %32
  %34 = sext i32 %.1 to i64
  %35 = getelementptr inbounds i32, ptr %3, i64 %34
  store i32 %33, ptr %35, align 4
  %36 = add nsw i32 %.1, 1
  br label %37

37:                                               ; preds = %26
  %38 = icmp slt i32 %36, %4
  br i1 %38, label %26, label %39, !llvm.loop !8

39:                                               ; preds = %37
  br label %40

40:                                               ; preds = %39, %23
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
