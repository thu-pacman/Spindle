; ModuleID = './examples/nested_loop.c'
source_filename = "./examples/nested_loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@n = dso_local global i32 0, align 4
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@str = dso_local local_unnamed_addr global [20 x i8] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  call void @__spindle_init_main()
  %1 = getelementptr inbounds [3 x i8], [3 x i8]* @.str, i64 0, i64 0
  %2 = tail call i32 (i8*, ...) @__isoc99_scanf(i8* noundef %1, i32* noundef nonnull @n)
  %3 = load i32, i32* @n, align 4, !tbaa !3
  call void @__spindle_record_value(i32 %3)
  %4 = icmp sgt i32 %3, 0
  call void @__spindle_record_br(i1 %4)
  br i1 %4, label %5, label %10

5:                                                ; preds = %0
  %6 = zext i32 %3 to i64
  call void bitcast (void (i32)* @__spindle_record_value to void (i64)*)(i64 %6)
  br label %7

7:                                                ; preds = %11, %5
  %8 = phi i32 [ 0, %5 ], [ %12, %11 ]
  %9 = sub i32 %3, %8
  br label %14

10:                                               ; preds = %11, %0
  call void @__spindle_fini_main()
  ret i32 0

11:                                               ; preds = %14
  %12 = add nuw nsw i32 %8, 1
  %13 = icmp eq i32 %12, %3
  br i1 %13, label %10, label %7, !llvm.loop !7

14:                                               ; preds = %7, %14
  %15 = phi i64 [ 0, %7 ], [ %26, %14 ]
  %16 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %15
  %17 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %15
  %18 = load i32, i32* %17, align 4, !tbaa !3
  %19 = trunc i64 %15 to i32
  %20 = add i32 %9, %19
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %21
  %23 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %21
  %24 = load i32, i32* %23, align 4, !tbaa !3
  %25 = add nsw i32 %24, %18
  store i32 %25, i32* %22, align 4, !tbaa !3
  %26 = add nuw nsw i64 %15, 1
  %27 = icmp eq i64 %26, %6
  br i1 %27, label %11, label %14, !llvm.loop !10
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #1

declare void @__spindle_record_br(i1)

declare void @__spindle_record_value(i32)

declare void @__spindle_init_main()

declare void @__spindle_fini_main()

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
