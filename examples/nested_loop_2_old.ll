; ModuleID = './examples/nested_loop_2.c'
source_filename = "./examples/nested_loop_2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@n = dso_local global i32 0, align 4
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@str = dso_local local_unnamed_addr global [20 x i8] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  %1 = tail call i32 (i8*, ...) @__isoc99_scanf(i8* noundef getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* noundef nonnull @n)
  %2 = load i32, i32* @n, align 4, !tbaa !3
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %12

4:                                                ; preds = %0
  %5 = add i32 %2, -1
  %6 = icmp eq i32 %2, 1
  %7 = zext i32 %5 to i64
  br label %8

8:                                                ; preds = %4, %13
  %9 = phi i32 [ 0, %4 ], [ %14, %13 ]
  br i1 %6, label %13, label %10

10:                                               ; preds = %8
  %11 = sub i32 %2, %9
  br label %16

12:                                               ; preds = %13, %0
  ret i32 0

13:                                               ; preds = %16, %8
  %14 = add nuw nsw i32 %9, 1
  %15 = icmp eq i32 %14, %2
  br i1 %15, label %12, label %8, !llvm.loop !7

16:                                               ; preds = %10, %16
  %17 = phi i64 [ 0, %10 ], [ %24, %16 ]
  %18 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %17
  %19 = load i32, i32* %18, align 4, !tbaa !3
  %20 = trunc i64 %17 to i32
  %21 = add i32 %11, %20
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %22
  store i32 %19, i32* %23, align 4, !tbaa !3
  %24 = add nuw nsw i64 %17, 1
  %25 = icmp eq i64 %24, %7
  br i1 %25, label %13, label %16, !llvm.loop !10
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"clang version 14.0.0 (https://github.com/llvm/llvm-project.git 329fda39c507e8740978d10458451dcdb21563be)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!"llvm.loop.unroll.disable"}
!10 = distinct !{!10, !8, !9}
