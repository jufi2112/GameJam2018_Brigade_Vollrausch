// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Materials/MaterialInterface.h"
#include "MyStaticLibrary.generated.h"

class ATerrainTile;

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

	// number of threads that should be used for terrain generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfThreadsToUse = 4;

	// how many meshes should be updated per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MeshUpdatesPerFrame = 8;


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
* struct for a job in which terrain is generated
*/
USTRUCT()
struct FTerrainJob
{
	GENERATED_USTRUCT_BODY()

	// the terrain tile this job calculates mesh data for
	UPROPERTY()
	ATerrainTile* TerrainTile = nullptr;

	// the terrain mesh data
	UPROPERTY()
	FMeshData TerrainMeshData;

	// the track mesh data
	UPROPERTY()
	FMeshData TrackMeshData;
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

	static void CreateSimpleMeshData(FTerrainSettings TerrainSettings, FMeshData& TerrainMeshDataOUT, FMeshData& TrackMeshDataOUT)
	{
		// First vertex
		TerrainMeshDataOUT.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(0, 0, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(0, 0)));

		// second vertex
		TerrainMeshDataOUT.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize, 0, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(TerrainSettings.UnitTileSize, 0)));

		// third vertex
		TerrainMeshDataOUT.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(TerrainSettings.UnitTileSize, TerrainSettings.UnitTileSize)));

		// fourth vertex
		TerrainMeshDataOUT.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(0, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(0, TerrainSettings.UnitTileSize)));

		// Triangles
		TerrainMeshDataOUT.TriangleBuffer.Add(0);
		TerrainMeshDataOUT.TriangleBuffer.Add(2);
		TerrainMeshDataOUT.TriangleBuffer.Add(1);
		TerrainMeshDataOUT.TriangleBuffer.Add(3);
		TerrainMeshDataOUT.TriangleBuffer.Add(2);
		TerrainMeshDataOUT.TriangleBuffer.Add(0);
	}

	static void CreateComplexMeshData(FTerrainSettings TerrainSettings, FMeshData& TerrainMeshDataOUT, FMeshData& TrackMeshDataOUT)
	{
		for (int x = 0; x < TerrainSettings.TileSizeXUnits; ++x)
		{
			for (int y = 0; y < TerrainSettings.TileSizeYUnits; ++y)
			{
				float z = FMath::RandRange(-1.f, 1.f);
				FVector Vec = FVector(x * TerrainSettings.UnitTileSize, y * TerrainSettings.UnitTileSize, z * 10.f);
				TerrainMeshDataOUT.VertexBuffer.Add(FRuntimeMeshVertexSimple(Vec, FVector(0.f, 0.f, 1.f), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(x, y)));
			}
		}

		int32 VertexIterator = 0;

		while (VertexIterator < ((TerrainSettings.TileSizeXUnits - 1) * TerrainSettings.TileSizeXUnits))
		{
			/* 2 vertices from "left", 1 vertex from "right" */
			TerrainMeshDataOUT.TriangleBuffer.Add(VertexIterator);
			TerrainMeshDataOUT.TriangleBuffer.Add(VertexIterator + 1);
			TerrainMeshDataOUT.TriangleBuffer.Add(VertexIterator + TerrainSettings.TileSizeXUnits);

			/* 2 vertices from "right", 1 vertex from "left" */
			TerrainMeshDataOUT.TriangleBuffer.Add(VertexIterator + 1);
			TerrainMeshDataOUT.TriangleBuffer.Add((VertexIterator + TerrainSettings.TileSizeXUnits) + 1);
			TerrainMeshDataOUT.TriangleBuffer.Add(VertexIterator + TerrainSettings.TileSizeXUnits);

			/* check if we reached the end of the column */
			if ((VertexIterator + 2) % TerrainSettings.TileSizeXUnits == 0)	// +2 because VertexIterator is 0-based
			{
				VertexIterator += 2;
			}
			else
			{
				VertexIterator += 1;
			}
		}
	}


};
	
