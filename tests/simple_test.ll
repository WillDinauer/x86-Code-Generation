; ModuleID = '<stdin>'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @f(i32 %0) #0 {
  ret i32 %0
}

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @invfib(i32 %0) #0 {
  %2 = icmp sge i32 %0, 10
  br i1 %2, label %3, label %4

3:                                                ; preds = %1
  ret i32 1

4:                                                ; preds = %1
  %5 = add nsw i32 %0, 1
  %6 = call i32 @invfib(i32 %5)
  %7 = add nsw i32 %0, 2
  %8 = call i32 @invfib(i32 %7)
  %9 = add nsw i32 %6, %8
  ret i32 %9
}

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @main() #0 {
  %1 = call i32 @invfib(i32 8)
  ret i32 %1
}

attributes #0 = { nounwind sspstrong uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"clang version 13.0.1"}
