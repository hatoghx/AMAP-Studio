// VRM4U Copyright (c) 2021-2026 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "BoneContainer.h"
#include "Misc/EngineVersionComparison.h"

#include "AnimNode_VrmCurveNormalizer.generated.h"

USTRUCT(BlueprintType)
struct VRM4U_API FVrmCurveNormalizationGroup {
	GENERATED_BODY()

	/** Curves that share the same value budget. Their relative balance is preserved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve Normalization")
	TArray<FName> CurveNames;

	/** Maximum sum of all active curves in this group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve Normalization", meta = (ClampMin = "0.0"))
	float MaximumCombinedValue = 1.f;

	/** 0 leaves the source unchanged; 1 applies the full normalization. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve Normalization", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 1.f;
};

/**
 * Normalizes animation curves after evaluating the input pose.
 *
 * The defaults are conservative PerfectSync/ARKit mouth corrections, but the
 * normalization groups are editable and can be used for arbitrary curves.
 * Place this node after the PoseAsset node, including in a Post Process AnimBP.
 */
USTRUCT(BlueprintInternalUseOnly)
struct VRM4U_API FAnimNode_VrmCurveNormalizer : public FAnimNode_Base {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink SourcePose;

	/** Applies the built-in 0-1 clamp to the 52 PerfectSync/ARKit face curves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	bool bClampPerfectSyncCurves = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	TArray<FVrmCurveNormalizationGroup> NormalizationGroups;

	/** Overall correction amount. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault, ClampMin = "0.0", ClampMax = "1.0"))
	float Alpha = 1.f;

	FAnimNode_VrmCurveNormalizer();

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;

private:
	bool TryGetCurveValue(const FBlendedCurve& Curve, FName CurveName, float& OutValue) const;
	void SetCurveValue(FBlendedCurve& Curve, FName CurveName, float Value) const;

#if UE_VERSION_OLDER_THAN(5,3,0)
	TMap<FName, SmartName::UID_Type> CachedCurveUIDs;
	void CacheCurveUIDs(const FAnimInstanceProxy* AnimInstanceProxy);
#endif
};
