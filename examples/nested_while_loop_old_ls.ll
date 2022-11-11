; ModuleID = './examples/nested_while_loop_old.ll'
source_filename = "./examples/nested_while_loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"%d, %d\0A\00", align 1
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  %1 = alloca i32, align 4
  %2 = bitcast i32* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #3
  %3 = call i32 (i8*, ...) @__isoc99_scanf(i8* noundef getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* noundef nonnull %1)
  %4 = load i32, i32* %1, align 4, !tbaa !3
  %5 = icmp sgt i32 %4, 1
  br i1 %5, label %.preheader1, label %20

.preheader1:                                      ; preds = %0
  br label %6

6:                                                ; preds = %.preheader1, %16
  %7 = phi i32 [ %17, %16 ], [ %4, %.preheader1 ]
  %8 = phi i32 [ %18, %16 ], [ 1, %.preheader1 ]
  %9 = icmp sgt i32 %7, 2
  br i1 %9, label %.preheader, label %16

.preheader:                                       ; preds = %6
  br label %10

10:                                               ; preds = %.preheader, %10
  %11 = phi i32 [ %13, %10 ], [ 2, %.preheader ]
  %12 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 noundef %8, i32 noundef %11)
  %13 = add nuw nsw i32 %11, 1
  %14 = load i32, i32* %1, align 4, !tbaa !3
  %15 = icmp slt i32 %13, %14
  br i1 %15, label %10, label %.loopexit, !llvm.loop !7

.loopexit:                                        ; preds = %10
  br label %16

16:                                               ; preds = %.loopexit, %6
  %17 = phi i32 [ %7, %6 ], [ %14, %.loopexit ]
  %18 = add nuw nsw i32 %8, 1
  %19 = icmp slt i32 %18, %17
  br i1 %19, label %6, label %.loopexit2, !llvm.loop !10

.loopexit2:                                       ; preds = %16
  br label %20

20:                                               ; preds = %.loopexit2, %0
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #3
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
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
