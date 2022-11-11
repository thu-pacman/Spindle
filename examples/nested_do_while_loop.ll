; ModuleID = './examples/nested_do_while_loop.c'
source_filename = "./examples/nested_do_while_loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  call void @__spindle_init_main()
  %1 = alloca i32, align 4
  %2 = bitcast i32* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #4
  %3 = call i32 (i8*, ...) @__isoc99_scanf(i8* noundef getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* noundef nonnull %1)
  %4 = load i32, i32* %1, align 4, !tbaa !3
  %5 = call i32 @llvm.smax.i32(i32 %4, i32 2)
  %6 = add nuw i32 %5, 1
  %7 = call i32 @llvm.smax.i32(i32 %4, i32 1)
  %8 = add nuw i32 %7, 1
  %9 = zext i32 %8 to i64
  call void @__spindle_record_value(i64 %9)
  %10 = zext i32 %6 to i64
  call void @__spindle_record_value(i64 %10)
  br label %11

11:                                               ; preds = %24, %0
  %12 = phi i64 [ %25, %24 ], [ 1, %0 ]
  br label %13

13:                                               ; preds = %13, %11
  %14 = phi i64 [ %22, %13 ], [ 2, %11 ]
  %15 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %14
  %16 = load i32, i32* %15, align 4, !tbaa !3
  %17 = sub nsw i64 %14, %12
  %18 = trunc i64 %17 to i32
  %19 = add i32 %4, %18
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %20
  store i32 %16, i32* %21, align 4, !tbaa !3
  %22 = add nuw nsw i64 %14, 1
  %23 = icmp eq i64 %22, %10
  br i1 %23, label %24, label %13, !llvm.loop !7

24:                                               ; preds = %13
  %25 = add nuw nsw i64 %12, 1
  %26 = icmp eq i64 %25, %9
  br i1 %26, label %27, label %11, !llvm.loop !10

27:                                               ; preds = %24
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #4
  call void @__spindle_fini_main()
  ret i32 0
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smax.i32(i32, i32) #3

declare void @__spindle_record_value(i64)

declare void @__spindle_init_main()

declare void @__spindle_fini_main()

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { nounwind }

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
