// VRM4U Copyright (c) 2021-2026 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VrmUtil.h"

#include "VrmImporterBPFunctionLibrary.generated.h"

class UVrmAssetListObject;

/**
 * Provides non-interactive VRM4U editor import operations.
 */
UCLASS()
class VRM4UIMPORTER_API UVrmImporterBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Imports a VRM4U-supported model or animation file with explicit options without
	 * displaying the import dialog. VRM, VRMA, GLB, and BVH are supported by default.
	 * VRM4U plugin settings can enable additional extensions and all Assimp-supported
	 * formats, including PMX/MMD models.
	 * The destination is a long package name such as /Game/Characters/Alicia.
	 * Generated assets remain dirty and are not saved automatically.
	 *
	 * @param SourceFile Source model or animation file on disk.
	 * @param DestinationPackagePath Package that will contain the generated assets.
	 * @param Options Import settings to apply.
	 * @return The generated VRM asset list, or null when the import fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "VRM4U|Import")
	static UVrmAssetListObject* ImportVRMFileWithOptions(
		const FString& SourceFile,
		const FString& DestinationPackagePath,
		const FImportOptionData& Options);
};
