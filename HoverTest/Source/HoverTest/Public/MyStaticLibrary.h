// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Materials/MaterialInterface.h"
#include "MyStaticLibrary.generated.h"

/**
* struct that defines an integer vector in 2D space, since Unreal decides to not come up with such a thing by default
*/
USTRUCT(BlueprintType)
struct FIntVector2D
{
	GENERATED_USTRUCT_BODY()

		int32 X;
	int32 Y;

	FIntVector2D(int32 x, int32 y)
	{
		X = x;
		Y = y;
	}

	FIntVector2D()
	{
		X = 0;
		Y = 0;
	}

	FORCEINLINE bool operator==(const FIntVector2D& Other) const
	{
		return (X == Other.X) && (Y == Other.Y);
	}

	FORCEINLINE bool operator!=(const FIntVector2D& Other) const
	{
		return (X != Other.X) || (Y != Other.Y);
	}

	FIntVector2D operator-(const FIntVector2D& Other) const
	{
		return FIntVector2D(X - Other.X, Y - Other.Y);
	}

	FIntVector2D operator+(const FIntVector2D& Other) const
	{
		return FIntVector2D(X + Other.X, Y + Other.Y);
	}

	FIntVector2D operator*(const FIntVector2D& Other) const
	{
		return FIntVector2D(X * Other.X, Y * Other.Y);
	}

	FIntVector2D operator*(const int32& Other) const
	{
		return FIntVector2D(X * Other, Y * Other);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("X=%i Y=%i"), X, Y);
	}

	FORCEINLINE uint32 GetTypeHash(const FIntVector2D& Vector)
	{
		// Note: this assumes there's no padding in FINTVector2D that could contain uncompared data.
		//return FCrc::MemCrc_DEPRECATED(&Vector, sizeof(Vector));
		return FCrc::MemCrc32(&Vector, sizeof(FIntVector2D));
	}
};


/**
* struct for terrain settings
*/
USTRUCT(BlueprintType)
struct FTerrainSettings
{
	GENERATED_USTRUCT_BODY()

		// size of a terrain tile in X direction in tile units
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 TileSizeXUnits = 0;

	// size of a terrain tile in Y direction in tile units
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 TileSizeYUnits = 0;

	// size of a terrain tile unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 UnitTileSize = 0;

	// shall async collision cooking be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bUseAsyncCollisionCooking = true;

	// material that should be applied to the default terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* TerrainMaterial = nullptr;

	// material that should be applied to the track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* TrackMaterial = nullptr;

	/**
	* specifies the number of tiles that should be created around a tracked actor
	* since it is the radius, a value of n will result in (2*n + 1) * (2*n + 1) tiles created around each tracked actor
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 TilesToBeCreatedAroundActorRadius = 3;

	//Time in seconds after which a freed tile should be deleted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SecondsUntilFreeTileGetsDeleted = 30.f;


};

/**
* struct for vertex and triangle buffer
*/
USTRUCT()
struct FMeshData
{
	GENERATED_USTRUCT_BODY()

		// vertex buffer
		TArray<FRuntimeMeshVertexSimple> VertexBuffer;

	// triangle buffer
	UPROPERTY()
		TArray<int32> TriangleBuffer;

};

/**
* TILE_FREE:			Tile was created, initialized, used, freed and is now waiting for usage again
*						Runtime mesh sections do not exist
*
* TILE_INITIALIZED:	Tile was created and initialized (i.e. RMC were created)
*						The runtime mesh component's mesh sections are not yet created
*
* TILE_FINISHED:		The runtime mesh component's mesh sections were created and are available
*
* TILE_UNDEFINED:		default value, state of every tile directly after creation until SetupTile is called
*
* TILE_TRANSITION:		Tile was freed and now the update position method was called
*						Runtime mesh sections are not yet created
*/
UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	TILE_FREE UMETA(DisplayName = "Free"),
	TILE_INITIALIZED UMETA(DisplayName = "Initialized"),
	TILE_FINISHED UMETA(DisplayName = "Finished"),
	TILE_UNDEFINED UMETA(DisplayName = "Undefined"),
	TILE_TRANSITION UMETA(DisplayName = "Transition")
};

/**
 * 
 */
UCLASS()
class HOVERTEST_API UMyStaticLibrary : public UObject
{
	GENERATED_BODY()
	
public:

	static void PrintActorArray(TArray<AActor*> ActorArray)
	{
		int32 i = 0;
		for (AActor* Actor : ActorArray)
		{
			UE_LOG(LogTemp, Warning, TEXT("%i: %s"), i, *Actor->GetName());
			i++;
		}
	}

	static void PrintFIntVector2DArray(TArray<FIntVector2D> VectorArray)
	{
		int32 i = 0;
		for (FIntVector2D Vector : VectorArray)
		{
			UE_LOG(LogTemp, Warning, TEXT("%i: %s"), i, *Vector.ToString());
			i++;
		}
	}


};
	
