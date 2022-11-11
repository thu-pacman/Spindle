; ModuleID = './examples/GEP_nested.c'
source_filename = "./examples/GEP_nested.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@n = dso_local global i32 0, align 4
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  %1 = tail call i32 (i8*, ...) @__isoc99_scanf(i8* noundef getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* noundef nonnull @n)
  %2 = load i32, i32* @n, align 4, !tbaa !3
  %3 = add nsw i32 %2, 6
  store i32 %3, i32* @n, align 4, !tbaa !3
  %4 = shl i32 %3, 1
  %5 = icmp sgt i32 %4, 7
  br i1 %5, label %6, label %9

6:                                                ; preds = %0
  %7 = zext i32 %4 to i64
  %8 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @a, i64 0, i64 6), align 8, !tbaa !3
  br label %10

9:                                                ; preds = %10, %0
  ret i32 0

10:                                               ; preds = %6, %10
  %11 = phi i32 [ %8, %6 ], [ %14, %10 ]
  %12 = phi i64 [ 7, %6 ], [ %16, %10 ]
  %13 = trunc i64 %12 to i32
  %14 = add nsw i32 %11, %13
  %15 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %12
  store i32 %14, i32* %15, align 4, !tbaa !3
  %16 = add nuw nsw i64 %12, 1
  %17 = icmp eq i64 %16, %7
  br i1 %17, label %9, label %10, !llvm.loop !7
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
