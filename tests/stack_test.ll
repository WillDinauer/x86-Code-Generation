; ModuleID = '<stdin>'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @longfunc() #0 {
  %1 = add nsw i32 0, 1
  %2 = add nsw i32 0, 1
  %3 = add nsw i32 0, 1
  %4 = add nsw i32 0, 1
  %5 = add nsw i32 0, 1
  %6 = add nsw i32 0, 1
  %7 = add nsw i32 0, 1
  %8 = add nsw i32 0, 1
  %9 = add nsw i32 0, 1
  %10 = add nsw i32 0, 1
  %11 = add nsw i32 0, 1
  %12 = add nsw i32 0, 1
  %13 = add nsw i32 0, 1
  %14 = add nsw i32 0, 1
  %15 = add nsw i32 0, 1
  %16 = add nsw i32 0, 1
  %17 = add nsw i32 0, 1
  %18 = add nsw i32 0, 1
  %19 = add nsw i32 0, 1
  %20 = add nsw i32 0, 1
  %21 = add nsw i32 0, 1
  %22 = add nsw i32 0, 1
  %23 = add nsw i32 0, 1
  %24 = add nsw i32 0, 1
  %25 = add nsw i32 0, 1
  %26 = add nsw i32 0, 1
  %27 = add nsw i32 %1, %2
  %28 = add nsw i32 %27, %3
  %29 = add nsw i32 %28, %4
  %30 = add nsw i32 %29, %5
  %31 = add nsw i32 %30, %6
  %32 = add nsw i32 %31, %7
  %33 = add nsw i32 %32, %8
  %34 = add nsw i32 %33, %9
  %35 = add nsw i32 %34, %10
  %36 = add nsw i32 %35, %11
  %37 = add nsw i32 %36, %12
  %38 = add nsw i32 %37, %13
  %39 = add nsw i32 %38, %14
  %40 = add nsw i32 %39, %15
  %41 = add nsw i32 %40, %16
  %42 = add nsw i32 %41, %17
  %43 = add nsw i32 %42, %18
  %44 = add nsw i32 %43, %19
  %45 = add nsw i32 %44, %20
  %46 = add nsw i32 %45, %21
  %47 = add nsw i32 %46, %22
  %48 = add nsw i32 %47, %23
  %49 = add nsw i32 %48, %24
  %50 = add nsw i32 %49, %25
  %51 = add nsw i32 %50, %26
  ret i32 %51
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @main() #0 {
  %1 = call i32 @longfunc()
  ret i32 %1
}

attributes #0 = { nounwind sspstrong uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"clang version 13.0.1"}
