// VRM4U Copyright (c) 2021-2026 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "AnimNode_VrmCurveNormalizer.h"

#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "VrmUtil.h"

namespace {
	static const TArray<FName>& GetPerfectSyncMorphCurveNames() {
		static const TArray<FName> CurveNames = {
			TEXT("EyeBlinkLeft"), TEXT("EyeLookDownLeft"), TEXT("EyeLookInLeft"), TEXT("EyeLookOutLeft"),
			TEXT("EyeLookUpLeft"), TEXT("EyeSquintLeft"), TEXT("EyeWideLeft"),
			TEXT("EyeBlinkRight"), TEXT("EyeLookDownRight"), TEXT("EyeLookInRight"), TEXT("EyeLookOutRight"),
			TEXT("EyeLookUpRight"), TEXT("EyeSquintRight"), TEXT("EyeWideRight"),
			TEXT("JawForward"), TEXT("JawLeft"), TEXT("JawRight"), TEXT("JawOpen"),
			TEXT("MouthClose"), TEXT("MouthFunnel"), TEXT("MouthPucker"), TEXT("MouthLeft"), TEXT("MouthRight"),
			TEXT("MouthSmileLeft"), TEXT("MouthSmileRight"), TEXT("MouthFrownLeft"), TEXT("MouthFrownRight"),
			TEXT("MouthDimpleLeft"), TEXT("MouthDimpleRight"), TEXT("MouthStretchLeft"), TEXT("MouthStretchRight"),
			TEXT("MouthRollLower"), TEXT("MouthRollUpper"), TEXT("MouthShrugLower"), TEXT("MouthShrugUpper"),
			TEXT("MouthPressLeft"), TEXT("MouthPressRight"), TEXT("MouthLowerDownLeft"), TEXT("MouthLowerDownRight"),
			TEXT("MouthUpperUpLeft"), TEXT("MouthUpperUpRight"),
			TEXT("BrowDownLeft"), TEXT("BrowDownRight"), TEXT("BrowInnerUp"), TEXT("BrowOuterUpLeft"), TEXT("BrowOuterUpRight"),
			TEXT("CheekPuff"), TEXT("CheekSquintLeft"), TEXT("CheekSquintRight"),
			TEXT("NoseSneerLeft"), TEXT("NoseSneerRight"), TEXT("TongueOut"),
		};
		return CurveNames;
	}

	static FVrmCurveNormalizationGroup MakeGroup(
		std::initializer_list<const TCHAR*> CurveNames,
		float MaximumCombinedValue,
		float Strength) {
		FVrmCurveNormalizationGroup Group;
		for (const TCHAR* CurveName : CurveNames) {
			Group.CurveNames.Add(FName(CurveName));
		}
		Group.MaximumCombinedValue = MaximumCombinedValue;
		Group.Strength = Strength;
		return Group;
	}
}

FAnimNode_VrmCurveNormalizer::FAnimNode_VrmCurveNormalizer() {
	// These defaults only reduce clearly competing controls. Correctives such as
	// JawOpen + MouthClose are deliberately left intact because they commonly need
	// to coexist in ARKit facial rigs.
	NormalizationGroups.Add(MakeGroup({TEXT("MouthFunnel"), TEXT("MouthPucker")}, 1.f, 0.75f));
	NormalizationGroups.Add(MakeGroup({TEXT("MouthLeft"), TEXT("MouthRight")}, 1.f, 1.f));
	NormalizationGroups.Add(MakeGroup({TEXT("JawLeft"), TEXT("JawRight")}, 1.f, 1.f));
	NormalizationGroups.Add(MakeGroup({TEXT("MouthSmileLeft"), TEXT("MouthFrownLeft")}, 1.f, 0.8f));
	NormalizationGroups.Add(MakeGroup({TEXT("MouthSmileRight"), TEXT("MouthFrownRight")}, 1.f, 0.8f));
}

void FAnimNode_VrmCurveNormalizer::Initialize_AnyThread(const FAnimationInitializeContext& Context) {
	Super::Initialize_AnyThread(Context);
	SourcePose.Initialize(Context);
#if UE_VERSION_OLDER_THAN(5,3,0)
	CacheCurveUIDs(Context.AnimInstanceProxy);
#endif
}

void FAnimNode_VrmCurveNormalizer::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) {
	Super::CacheBones_AnyThread(Context);
	SourcePose.CacheBones(Context);
#if UE_VERSION_OLDER_THAN(5,3,0)
	CacheCurveUIDs(Context.AnimInstanceProxy);
#endif
}

void FAnimNode_VrmCurveNormalizer::Update_AnyThread(const FAnimationUpdateContext& Context) {
	SourcePose.Update(Context);
	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FAnimNode_VrmCurveNormalizer::Evaluate_AnyThread(FPoseContext& Output) {
	SourcePose.Evaluate(Output);

	const float UseAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
	if (UseAlpha <= 0.f) {
		return;
	}

	if (bClampPerfectSyncCurves) {
		for (const FName CurveName : GetPerfectSyncMorphCurveNames()) {
			float SourceValue = 0.f;
			if (TryGetCurveValue(Output.Curve, CurveName, SourceValue) == false) {
				continue;
			}

			const float ClampedValue = FMath::Clamp(SourceValue, 0.f, 1.f);
			SetCurveValue(Output.Curve, CurveName, FMath::Lerp(SourceValue, ClampedValue, UseAlpha));
		}
	}

	for (const FVrmCurveNormalizationGroup& Group : NormalizationGroups) {
		const float MaximumCombinedValue = FMath::Max(0.f, Group.MaximumCombinedValue);
		const float UseStrength = FMath::Clamp(Group.Strength, 0.f, 1.f) * UseAlpha;
		if (Group.CurveNames.Num() < 2 || UseStrength <= 0.f) {
			continue;
		}

		TArray<float, TInlineAllocator<8>> Values;
		TArray<bool, TInlineAllocator<8>> ValidCurves;
		Values.Reserve(Group.CurveNames.Num());
		ValidCurves.Reserve(Group.CurveNames.Num());

		float CombinedValue = 0.f;
		for (const FName CurveName : Group.CurveNames) {
			float CurveValue = 0.f;
			const bool bHasCurve = TryGetCurveValue(Output.Curve, CurveName, CurveValue);
			CurveValue = FMath::Max(0.f, CurveValue);
			Values.Add(CurveValue);
			ValidCurves.Add(bHasCurve);
			if (bHasCurve) {
				CombinedValue += CurveValue;
			}
		}

		if (CombinedValue <= MaximumCombinedValue || CombinedValue <= SMALL_NUMBER) {
			continue;
		}

		const float FullScale = MaximumCombinedValue / CombinedValue;
		const float AppliedScale = FMath::Lerp(1.f, FullScale, UseStrength);
		for (int32 CurveIndex = 0; CurveIndex < Group.CurveNames.Num(); ++CurveIndex) {
			if (ValidCurves[CurveIndex]) {
				SetCurveValue(Output.Curve, Group.CurveNames[CurveIndex], Values[CurveIndex] * AppliedScale);
			}
		}
	}
}

void FAnimNode_VrmCurveNormalizer::GatherDebugData(FNodeDebugData& DebugData) {
	FString DebugLine = DebugData.GetNodeName(this);
	DebugLine += FString::Printf(TEXT("(Alpha: %.2f, Groups: %d)"), Alpha, NormalizationGroups.Num());
	DebugData.AddDebugItem(DebugLine);
	SourcePose.GatherDebugData(DebugData);
}

bool FAnimNode_VrmCurveNormalizer::TryGetCurveValue(const FBlendedCurve& Curve, FName CurveName, float& OutValue) const {
#if UE_VERSION_OLDER_THAN(5,3,0)
	const SmartName::UID_Type* CurveUID = CachedCurveUIDs.Find(CurveName);
	if (CurveUID == nullptr) {
		return false;
	}
	OutValue = Curve.Get(*CurveUID);
	return true;
#else
	bool bFound = false;
	Curve.ForEachElement([&](const UE::Anim::FCurveElement& CurveElement) {
		if (CurveElement.Name == CurveName) {
			OutValue = CurveElement.Value;
			bFound = true;
		}
	});
	return bFound;
#endif
}

void FAnimNode_VrmCurveNormalizer::SetCurveValue(FBlendedCurve& Curve, FName CurveName, float Value) const {
#if UE_VERSION_OLDER_THAN(5,3,0)
	const SmartName::UID_Type* CurveUID = CachedCurveUIDs.Find(CurveName);
	if (CurveUID) {
		Curve.Set(*CurveUID, Value);
	}
#else
	Curve.Set(CurveName, Value);
#endif
}

#if UE_VERSION_OLDER_THAN(5,3,0)
void FAnimNode_VrmCurveNormalizer::CacheCurveUIDs(const FAnimInstanceProxy* AnimInstanceProxy) {
	CachedCurveUIDs.Reset();
	if (AnimInstanceProxy == nullptr) {
		return;
	}

	const USkeletalMeshComponent* Component = AnimInstanceProxy->GetSkelMeshComponent();
	if (Component == nullptr) {
		return;
	}

	const USkeletalMesh* SkeletalMesh = VRMGetSkinnedAsset(Component);
	const USkeleton* Skeleton = VRMGetSkeleton(SkeletalMesh);
	const FSmartNameMapping* CurveMapping = Skeleton ? Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName) : nullptr;
	if (CurveMapping == nullptr) {
		return;
	}

	TArray<FName> CurveNames;
	TArray<SmartName::UID_Type> CurveUIDs;
	CurveMapping->FillUIDToNameArray(CurveNames);
	CurveMapping->FillUidArray(CurveUIDs);
	const int32 CurveCount = FMath::Min(CurveNames.Num(), CurveUIDs.Num());
	for (int32 CurveIndex = 0; CurveIndex < CurveCount; ++CurveIndex) {
		CachedCurveUIDs.Add(CurveNames[CurveIndex], CurveUIDs[CurveIndex]);
	}
}
#endif
