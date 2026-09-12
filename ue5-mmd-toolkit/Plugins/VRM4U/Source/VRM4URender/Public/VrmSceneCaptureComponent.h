// VRM4U Copyright (c) 2021-2026 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "Components/SceneCaptureComponent2D.h"
#include "VrmSceneCaptureComponent.generated.h"

//DECLARE_LOG_CATEGORY_EXTERN(LogCineCapture, Log, All);

class FVrmSceneCaptureSceneViewExtension;

/**
* Scene Capture component that follows the level editor viewport associated
* with its world or Player 0's game view, and copies GBuffer-derived and
* scene-texture data into render targets.
*/
UCLASS(meta = (BlueprintSpawnableComponent))
//UCLASS(hidecategories = (Transform, Collision, Object, Physics, SceneComponent, PostProcessVolume, Projection, Rendering, PlanarReflection), ClassGroup = Rendering, editinlinenew, meta = (BlueprintSpawnableComponent))
class VRM4URENDER_API UVrmSceneCaptureComponent2D : public USceneCaptureComponent2D
{
	GENERATED_UCLASS_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings")
	TObjectPtr<UTextureRenderTarget2D> RT_BaseColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings")
	TObjectPtr<UTextureRenderTarget2D> RT_Normal;

	/** Metallic / Specular / Roughness (GBufferB) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings")
	TObjectPtr<UTextureRenderTarget2D> RT_MRS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings")
	TObjectPtr<UTextureRenderTarget2D> RT_Depth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings")
	TObjectPtr<UTextureRenderTarget2D> RT_CustomStencil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings")
	TObjectPtr<UTextureRenderTarget2D> RT_CustomDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings", meta = (UIMin = 1.0, UIMax = 10.0))
	float RenderTargetResolutionDivisorX = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Settings", meta = (UIMin = 1.0, UIMax = 10.0))
	float RenderTargetResolutionDivisorY = 1.f;

public:
	virtual void OnComponentCreated() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void EnsureTextureTargetCreated();

	FDelegateHandle handle;

#if WITH_EDITOR
	void OnCameraTransformChanged(const FVector&, const FRotator&, ELevelViewportType, int32);
#endif
	void OnCameraTransformChanged();

	void ResizeRenderTargets(FIntPoint size);

private:
	/** This scene view extension is used to get ahold of views during the setup process. */
	TSharedPtr<FVrmSceneCaptureSceneViewExtension, ESPMode::ThreadSafe> SceneViewExtension;
	//TSharedPtr<class FVrmSceneViewExtension, ESPMode::ThreadSafe> SceneViewExtension;

};
