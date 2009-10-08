; RUN: llc < %s -march=arm -mattr=+neon | FileCheck %s

define <8 x i8> @vrhadds8(<8 x i8>* %A, <8 x i8>* %B) nounwind {
;CHECK: vrhadds8:
;CHECK: vrhadd.s8
	%tmp1 = load <8 x i8>* %A
	%tmp2 = load <8 x i8>* %B
	%tmp3 = call <8 x i8> @llvm.arm.neon.vrhadds.v8i8(<8 x i8> %tmp1, <8 x i8> %tmp2)
	ret <8 x i8> %tmp3
}

define <4 x i16> @vrhadds16(<4 x i16>* %A, <4 x i16>* %B) nounwind {
;CHECK: vrhadds16:
;CHECK: vrhadd.s16
	%tmp1 = load <4 x i16>* %A
	%tmp2 = load <4 x i16>* %B
	%tmp3 = call <4 x i16> @llvm.arm.neon.vrhadds.v4i16(<4 x i16> %tmp1, <4 x i16> %tmp2)
	ret <4 x i16> %tmp3
}

define <2 x i32> @vrhadds32(<2 x i32>* %A, <2 x i32>* %B) nounwind {
;CHECK: vrhadds32:
;CHECK: vrhadd.s32
	%tmp1 = load <2 x i32>* %A
	%tmp2 = load <2 x i32>* %B
	%tmp3 = call <2 x i32> @llvm.arm.neon.vrhadds.v2i32(<2 x i32> %tmp1, <2 x i32> %tmp2)
	ret <2 x i32> %tmp3
}

define <8 x i8> @vrhaddu8(<8 x i8>* %A, <8 x i8>* %B) nounwind {
;CHECK: vrhaddu8:
;CHECK: vrhadd.u8
	%tmp1 = load <8 x i8>* %A
	%tmp2 = load <8 x i8>* %B
	%tmp3 = call <8 x i8> @llvm.arm.neon.vrhaddu.v8i8(<8 x i8> %tmp1, <8 x i8> %tmp2)
	ret <8 x i8> %tmp3
}

define <4 x i16> @vrhaddu16(<4 x i16>* %A, <4 x i16>* %B) nounwind {
;CHECK: vrhaddu16:
;CHECK: vrhadd.u16
	%tmp1 = load <4 x i16>* %A
	%tmp2 = load <4 x i16>* %B
	%tmp3 = call <4 x i16> @llvm.arm.neon.vrhaddu.v4i16(<4 x i16> %tmp1, <4 x i16> %tmp2)
	ret <4 x i16> %tmp3
}

define <2 x i32> @vrhaddu32(<2 x i32>* %A, <2 x i32>* %B) nounwind {
;CHECK: vrhaddu32:
;CHECK: vrhadd.u32
	%tmp1 = load <2 x i32>* %A
	%tmp2 = load <2 x i32>* %B
	%tmp3 = call <2 x i32> @llvm.arm.neon.vrhaddu.v2i32(<2 x i32> %tmp1, <2 x i32> %tmp2)
	ret <2 x i32> %tmp3
}

define <16 x i8> @vrhaddQs8(<16 x i8>* %A, <16 x i8>* %B) nounwind {
;CHECK: vrhaddQs8:
;CHECK: vrhadd.s8
	%tmp1 = load <16 x i8>* %A
	%tmp2 = load <16 x i8>* %B
	%tmp3 = call <16 x i8> @llvm.arm.neon.vrhadds.v16i8(<16 x i8> %tmp1, <16 x i8> %tmp2)
	ret <16 x i8> %tmp3
}

define <8 x i16> @vrhaddQs16(<8 x i16>* %A, <8 x i16>* %B) nounwind {
;CHECK: vrhaddQs16:
;CHECK: vrhadd.s16
	%tmp1 = load <8 x i16>* %A
	%tmp2 = load <8 x i16>* %B
	%tmp3 = call <8 x i16> @llvm.arm.neon.vrhadds.v8i16(<8 x i16> %tmp1, <8 x i16> %tmp2)
	ret <8 x i16> %tmp3
}

define <4 x i32> @vrhaddQs32(<4 x i32>* %A, <4 x i32>* %B) nounwind {
;CHECK: vrhaddQs32:
;CHECK: vrhadd.s32
	%tmp1 = load <4 x i32>* %A
	%tmp2 = load <4 x i32>* %B
	%tmp3 = call <4 x i32> @llvm.arm.neon.vrhadds.v4i32(<4 x i32> %tmp1, <4 x i32> %tmp2)
	ret <4 x i32> %tmp3
}

define <16 x i8> @vrhaddQu8(<16 x i8>* %A, <16 x i8>* %B) nounwind {
;CHECK: vrhaddQu8:
;CHECK: vrhadd.u8
	%tmp1 = load <16 x i8>* %A
	%tmp2 = load <16 x i8>* %B
	%tmp3 = call <16 x i8> @llvm.arm.neon.vrhaddu.v16i8(<16 x i8> %tmp1, <16 x i8> %tmp2)
	ret <16 x i8> %tmp3
}

define <8 x i16> @vrhaddQu16(<8 x i16>* %A, <8 x i16>* %B) nounwind {
;CHECK: vrhaddQu16:
;CHECK: vrhadd.u16
	%tmp1 = load <8 x i16>* %A
	%tmp2 = load <8 x i16>* %B
	%tmp3 = call <8 x i16> @llvm.arm.neon.vrhaddu.v8i16(<8 x i16> %tmp1, <8 x i16> %tmp2)
	ret <8 x i16> %tmp3
}

define <4 x i32> @vrhaddQu32(<4 x i32>* %A, <4 x i32>* %B) nounwind {
;CHECK: vrhaddQu32:
;CHECK: vrhadd.u32
	%tmp1 = load <4 x i32>* %A
	%tmp2 = load <4 x i32>* %B
	%tmp3 = call <4 x i32> @llvm.arm.neon.vrhaddu.v4i32(<4 x i32> %tmp1, <4 x i32> %tmp2)
	ret <4 x i32> %tmp3
}

declare <8 x i8>  @llvm.arm.neon.vrhadds.v8i8(<8 x i8>, <8 x i8>) nounwind readnone
declare <4 x i16> @llvm.arm.neon.vrhadds.v4i16(<4 x i16>, <4 x i16>) nounwind readnone
declare <2 x i32> @llvm.arm.neon.vrhadds.v2i32(<2 x i32>, <2 x i32>) nounwind readnone

declare <8 x i8>  @llvm.arm.neon.vrhaddu.v8i8(<8 x i8>, <8 x i8>) nounwind readnone
declare <4 x i16> @llvm.arm.neon.vrhaddu.v4i16(<4 x i16>, <4 x i16>) nounwind readnone
declare <2 x i32> @llvm.arm.neon.vrhaddu.v2i32(<2 x i32>, <2 x i32>) nounwind readnone

declare <16 x i8> @llvm.arm.neon.vrhadds.v16i8(<16 x i8>, <16 x i8>) nounwind readnone
declare <8 x i16> @llvm.arm.neon.vrhadds.v8i16(<8 x i16>, <8 x i16>) nounwind readnone
declare <4 x i32> @llvm.arm.neon.vrhadds.v4i32(<4 x i32>, <4 x i32>) nounwind readnone

declare <16 x i8> @llvm.arm.neon.vrhaddu.v16i8(<16 x i8>, <16 x i8>) nounwind readnone
declare <8 x i16> @llvm.arm.neon.vrhaddu.v8i16(<8 x i16>, <8 x i16>) nounwind readnone
declare <4 x i32> @llvm.arm.neon.vrhaddu.v4i32(<4 x i32>, <4 x i32>) nounwind readnone
