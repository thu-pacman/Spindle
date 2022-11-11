; ModuleID = './examples/nested_while_loop.c'
source_filename = "./examples/nested_while_loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"%d, %d\0A\00", align 1
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  call void @__spindle_init_main()
  %1 = alloca i32, align 4
  %2 = bitcast i32* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #3
  %3 = getelementptr inbounds [3 x i8], [3 x i8]* @.str, i64 0, i64 0
  %4 = call i32 (i8*, ...) @__isoc99_scanf(i8* noundef %3, i32* noundef nonnull %1)
  %5 = load i32, i32* %1, align 4, !tbaa !3
  %6 = icmp sgt i32 %5, 1
  call void @__spindle_record_br(i1 %6)
  br i1 %6, label %7, label %26

7:                                                ; preds = %0
  br label %8

8:                                                ; preds = %7, %21
  %9 = phi i32 [ %22, %21 ], [ %5, %7 ]
  %10 = phi i32 [ %23, %21 ], [ 1, %7 ]
  %11 = icmp sgt i32 %9, 2
  call void @__spindle_record_br(i1 %11)
  br i1 %11, label %12, label %21

12:                                               ; preds = %8
  br label %13

13:                                               ; preds = %12, %13
  %14 = phi i32 [ %17, %13 ], [ 2, %12 ]
  %15 = getelementptr inbounds [8 x i8], [8 x i8]* @.str.1, i64 0, i64 0
  %16 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) %15, i32 noundef %10, i32 noundef %14)
  %17 = add nuw nsw i32 %14, 1
  %18 = load i32, i32* %1, align 4, !tbaa !3
  call void @__spindle_record_value(i32 %18)
  %19 = icmp slt i32 %17, %18
  br i1 %19, label %13, label %20, !llvm.loop !7

20:                                               ; preds = %13
  br label %21

21:                                               ; preds = %20, %8
  %22 = phi i32 [ %9, %8 ], [ %18, %20 ]
  %23 = add nuw nsw i32 %10, 1
  %24 = icmp slt i32 %23, %22
  call void @__spindle_record_br(i1 %24)
  br i1 %24, label %8, label %25, !llvm.loop !10

25:                                               ; preds = %21
  br label %26

26:                                               ; preds = %25, %0
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #3
  call void @__spindle_fini_main()
  ret i32 0
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

declare void @__spindle_record_br(i1)

declare void @__spindle_record_value(i32)

declare void @__spindle_init_main()

declare void @__spindle_fini_main()

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

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
