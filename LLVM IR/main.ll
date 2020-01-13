; ModuleID = 'code.c'
source_filename = "code.c"

define i32 @add(i32, i32) {
  %3 = alloca i32
  %4 = alloca i32
  store i32 %0, i32* %3
  store i32 %1, i32* %4
  %5 = alloca i32
  %6 = load i32, i32* %3
  store i32 %6, i32* %5
  %7 = load i32, i32* %5
  %8 = alloca i32
  %9 = load i32, i32* %4
  store i32 %9, i32* %8
  %10 = load i32, i32* %8
  %11 = add nsw i32 %7, %10
  %12 = alloca i32
  store i32 %11, i32* %12
  %13 = load i32, i32* %12
  ret i32 %13
}

define i32 @main() {
  %1 = alloca i32
  %2 = alloca i32
  %3 = alloca i32
  %4 = alloca i32
  store i32 10, i32* %4
  %5 = load i32, i32* %4
  %6 = load i32, i32* %4
  store i32 %6, i32* %1
  %7 = alloca i32
  store i32 5, i32* %7
  %8 = load i32, i32* %7
  %9 = load i32, i32* %7
  store i32 %9, i32* %2
  %10 = alloca i32
  store i32 0, i32* %10
  %11 = load i32, i32* %10
  %12 = load i32, i32* %10
  store i32 %12, i32* %3
  br label %13

; <label>:13:                                     ; preds = %61, %0
  %14 = alloca i32
  %15 = load i32, i32* %1
  store i32 %15, i32* %14
  %16 = load i32, i32* %14
  %17 = alloca i32
  store i32 0, i32* %17
  %18 = load i32, i32* %17
  %19 = icmp sgt i32 %16, %18
  br i1 %19, label %20, label %27

; <label>:20:                                     ; preds = %13
  %21 = alloca i32
  %22 = load i32, i32* %2
  store i32 %22, i32* %21
  %23 = load i32, i32* %21
  %24 = alloca i32
  store i32 0, i32* %24
  %25 = load i32, i32* %24
  %26 = icmp sgt i32 %23, %25
  br i1 %26, label %31, label %51

; <label>:27:                                     ; preds = %13
  %28 = alloca i32
  %29 = load i32, i32* %3
  store i32 %29, i32* %28
  %30 = load i32, i32* %28
  ret i32 %30

; <label>:31:                                     ; preds = %20
  %32 = alloca i32
  %33 = load i32, i32* %3
  store i32 %33, i32* %32
  %34 = load i32, i32* %32
  %35 = alloca i32
  %36 = load i32, i32* %1
  store i32 %36, i32* %35
  %37 = load i32, i32* %35
  %38 = call i32 @add(i32 %34, i32 %37)
  %39 = alloca i32
  store i32 %38, i32* %39
  %40 = load i32, i32* %39
  %41 = load i32, i32* %39
  store i32 %41, i32* %3
  %42 = alloca i32
  %43 = load i32, i32* %2
  store i32 %43, i32* %42
  %44 = load i32, i32* %42
  %45 = alloca i32
  store i32 1, i32* %45
  %46 = load i32, i32* %45
  %47 = sub nsw i32 %44, %46
  %48 = alloca i32
  store i32 %47, i32* %48
  %49 = load i32, i32* %48
  %50 = load i32, i32* %48
  store i32 %50, i32* %2
  br label %61

; <label>:51:                                     ; preds = %20
  %52 = alloca i32
  %53 = load i32, i32* %3
  store i32 %53, i32* %52
  %54 = load i32, i32* %52
  %55 = alloca i32
  store i32 1, i32* %55
  %56 = load i32, i32* %55
  %57 = add nsw i32 %54, %56
  %58 = alloca i32
  store i32 %57, i32* %58
  %59 = load i32, i32* %58
  %60 = load i32, i32* %58
  store i32 %60, i32* %3
  br label %61

; <label>:61:                                     ; preds = %51, %31
  %62 = alloca i32
  %63 = load i32, i32* %1
  store i32 %63, i32* %62
  %64 = load i32, i32* %62
  %65 = alloca i32
  store i32 1, i32* %65
  %66 = load i32, i32* %65
  %67 = sub nsw i32 %64, %66
  %68 = alloca i32
  store i32 %67, i32* %68
  %69 = load i32, i32* %68
  %70 = load i32, i32* %68
  store i32 %70, i32* %1
  br label %13
}
