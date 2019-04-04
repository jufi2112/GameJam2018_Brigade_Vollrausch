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

///**
//* struct for a float based index system (x and y components of position) used in the DEM
//* specially constructed so that the == method could be overloaded by using FMath::IsNearlyEqual
//*/
//USTRUCT()
//struct FComparableFloat
//{
//	GENERATED_USTRUCT_BODY()
//
//	float Width;
//	float Height;
//
//	FComparableFloat(float width, float height)
//	{
//		Width = width;
//		Height = height;
//	}
//
//	FComparableFloat()
//	{
//		Width = 0.f;
//		Height = 0.f;
//	}
//
//	FORCEINLINE bool operator==(const FComparableFloat& Other) const
//	{
//		return (FMath::IsNearlyEqual(Width, Other.Width)) && (FMath::IsNearlyEqual(Height, Other.Height));
//	}
//
//	FORCEINLINE bool operator!=(const FComparableFloat& Other) const
//	{
//		return (Width != Other.Width) || (Height != Other.Height);
//	}
//
//	FORCEINLINE uint32 GetTypeHash(const FComparableFloat& ComparableFloat)
//	{
//		return FCrc::MemCrc32(&ComparableFloat, sizeof(FComparableFloat));
//	}
//};

//UENUM()
//enum class EDEMState : uint8
//{
//	DEM_UNKNOWN,
//	DEM_KNOWN
//};
//
///**
//* struct for a digital elevation map (DEM, as presented in "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj in 2007)
//* should be used as TMultiMap<FComparableFloat, TMap<FComparableFloat, FDEM>>
//*/
//USTRUCT()
//struct FDEM
//{
//	GENERATED_USTRUCT_BODY()
//
//	float Elevation;
//	EDEMState DEMState;
//
//	FDEM(float elevation)
//	{
//		Elevation = elevation;
//		DEMState = EDEMState::DEM_UNKNOWN;
//	}
//	FDEM(float elevation, EDEMState State)
//	{
//		Elevation = elevation;
//		DEMState = State;
//	}
//	FDEM()
//	{
//		Elevation = 0.f;
//		DEMState = EDEMState::DEM_UNKNOWN;
//	}
//
//};

/**
 * struct for fractal noise terrain generation settings
 * to be used as subcategory in FTerrainSettings
 * all parameters are used as suggested in "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj in 2007
 */
USTRUCT(BlueprintType)
struct FFractalNoiseTerrainSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * used to translate random value interval [-1, 1] in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rt = -0.9f;

	/**
	 * scale factor to scale random value in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rs = 0.55f;

	/**
	 * space dimension used in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float n = 3.f;

	/**
	 * approximation of Hurst's parameter
	 * used to control fractal dimension in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float H = 0.5f;

	/**
	 * used to tune the non-linear interpolation curve in the midpoint displacement process
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float I = -2.f;

	/**
	 * used to tune the non-linear interpolation curve in the midpoint displacement bottom-up process
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float I_bu = 5.f;
};

/**
 * struct for terrain settings
 */
USTRUCT(BlueprintType)
struct FTerrainSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * to be removed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point1Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point2Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point3Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point4Elevation = 0.f;

	/**
	* to be removed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Point5Elevation = 0.f;

	/**
	 * size of a terrain tile in X direction in tile units
	 * @DEPRECATED
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileSizeXUnits = 0;

	/**
	 * size of a terrain tile in Y direction in tile units
	 * @DEPRECATED
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileSizeYUnits = 0;

	/**
	 * size of a terrain tile unit
	 * @DEPRECATED
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitTileSize = 0;

	// the edge length of a quadratic tile in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileEdgeSize = 65536;

	// number of iterations for the triangle edge algorithms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TriangleEdgeIterations = 6;

	// shall async collision cooking be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAsyncCollisionCooking = true;

	// fractal noise terrain generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFractalNoiseTerrainSettings FractalNoiseTerrainSettings;

	/** 
	 * array of materials that should be applied to the terrain
	 * index 0 is default terrain material
	 * index 1 is default track material
	 * corresponds to the mesh index in FTerrainJob
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UMaterialInterface*> Materials;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UMaterialInterface* TerrainMaterial = nullptr;

	//// material that should be applied to the track
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UMaterialInterface* TrackMaterial = nullptr;

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
USTRUCT(BlueprintType)
struct FTerrainJob
{
	GENERATED_USTRUCT_BODY()

	// the terrain tile this job calculates mesh data for
	UPROPERTY()
	ATerrainTile* TerrainTile = nullptr;

	//// the terrain mesh data
	//UPROPERTY()
	//FMeshData TerrainMeshData;

	//// the track mesh data
	//UPROPERTY()
	//FMeshData TrackMeshData;

	/**
	* array where each index corresponds to one mesh section
	* first index corresponds to terrain
	* second index corresponds to track
	*/
	UPROPERTY()
	TArray<FMeshData> MeshData;
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

	static void CreateSimpleMeshData(FTerrainSettings TerrainSettings, TArray<FMeshData>& MeshDataOUT)
	{
		// First vertex
		FMeshData MeshData;
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(0, 0, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(0, 0)));

		// second vertex
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize, 0, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(TerrainSettings.UnitTileSize, 0)));

		// third vertex
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(TerrainSettings.UnitTileSize, TerrainSettings.UnitTileSize)));

		// fourth vertex
		MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(FVector(0, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize, 0), FVector(0, 0, 1), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(0, TerrainSettings.UnitTileSize)));

		// Triangles
		MeshData.TriangleBuffer.Add(0);
		MeshData.TriangleBuffer.Add(2);
		MeshData.TriangleBuffer.Add(1);
		MeshData.TriangleBuffer.Add(3);
		MeshData.TriangleBuffer.Add(2);
		MeshData.TriangleBuffer.Add(0);
		MeshDataOUT.Add(MeshData);
	}

	static void CreateComplexMeshData(FTerrainSettings TerrainSettings, TArray<FMeshData>& MeshDataOUT)
	{
		FMeshData MeshData;
		for (int x = 0; x < TerrainSettings.TileSizeXUnits; ++x)
		{
			for (int y = 0; y < TerrainSettings.TileSizeYUnits; ++y)
			{
				float z = FMath::RandRange(-1.f, 1.f);
				FVector Vec = FVector(x * TerrainSettings.UnitTileSize, y * TerrainSettings.UnitTileSize, z * 10.f);
				MeshData.VertexBuffer.Add(FRuntimeMeshVertexSimple(Vec, FVector(0.f, 0.f, 1.f), FRuntimeMeshTangent(0, -1, 0), FColor::White, FVector2D(x, y)));
			}
		}

		int32 VertexIterator = 0;

		while (VertexIterator < ((TerrainSettings.TileSizeXUnits - 1) * TerrainSettings.TileSizeXUnits))
		{
			/* 2 vertices from "left", 1 vertex from "right" */
			MeshData.TriangleBuffer.Add(VertexIterator);
			MeshData.TriangleBuffer.Add(VertexIterator + 1);
			MeshData.TriangleBuffer.Add(VertexIterator + TerrainSettings.TileSizeXUnits);

			/* 2 vertices from "right", 1 vertex from "left" */
			MeshData.TriangleBuffer.Add(VertexIterator + 1);
			MeshData.TriangleBuffer.Add((VertexIterator + TerrainSettings.TileSizeXUnits) + 1);
			MeshData.TriangleBuffer.Add(VertexIterator + TerrainSettings.TileSizeXUnits);

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

		MeshDataOUT.Add(MeshData);
	}

	//Float as String With Precision! || Code Taken from https://wiki.unrealengine.com/Float_as_String_With_Precision Author: Rama
	static FORCEINLINE FString GetFloatAsStringWithPrecision(float TheFloat, int32 Precision, bool IncludeLeadingZero = true)
	{
		////Round to integral if have something like 1.9999 within precision
		//float Rounded = roundf(TheFloat);
		//if (FMath::Abs(TheFloat - Rounded) < FMath::Pow(10, -1 * Precision))
		//{
		//	TheFloat = Rounded;
		//}
		//FNumberFormattingOptions NumberFormat;					//Text.h
		//NumberFormat.MinimumIntegralDigits = (IncludeLeadingZero) ? 1 : 0;
		//NumberFormat.MaximumIntegralDigits = 10000;
		//NumberFormat.MinimumFractionalDigits = Precision;
		//NumberFormat.MaximumFractionalDigits = Precision;
		//return FText::AsNumber(TheFloat, &NumberFormat).ToString();

		// own code follows
		float Buffer = TheFloat * FMath::Pow(10, Precision);
		Buffer = FMath::FloorToFloat(Buffer);
		return FString::SanitizeFloat((Buffer / FMath::Pow(10, Precision)), Precision);
	}

	static FORCEINLINE float GetFloatWithPrecision(const float TheFloat, const int32 Precision)
	{
		float Buffer = TheFloat * FMath::Pow(10, Precision);
		Buffer = FMath::FloorToFloat(Buffer);
		return (Buffer / FMath::Pow(10, Precision));
	}

	static void SaveBuffersToFile(const TArray<FRuntimeMeshVertexSimple>& VertexBuffer, const TArray<int32>& TriangleBuffer)
	{
		FString SaveDirectory = "D:/Users/Julien/Documents/Unreal Engine Dumps";
		FString VertexFileName = "VertexBuffer.txt";
		FString TriangleFileName = "TriangleBuffer.txt";
		FString VertexContent = "";
		FString TriangleContent = "";
		int32 TriangleIndex = 0;

		// Vertex Buffer
		VertexContent.Append("-------------- Begin of VertexBuffer --------------\n");
		for (int32 VertexIndex = 0; VertexIndex < VertexBuffer.Num(); ++VertexIndex)
		{
			VertexContent.Append(FString::FromInt(VertexIndex) + ": " + VertexBuffer[VertexIndex].Position.ToString() + "\n");
		}
		VertexContent.Append("-------------- End of VertexBuffer --------------\n");

		TriangleContent.Append("-------------- Begin of TriangleBuffer --------------\n");
		// Triangle Buffer
		for (const int32 TrianglePoint : TriangleBuffer)
		{
			if (TriangleIndex < 2)
			{
				TriangleContent.Append(FString::FromInt(TrianglePoint) + ", ");
				++TriangleIndex;
			}
			else
			{
				TriangleContent.Append(FString::FromInt(TrianglePoint) + "\n----------------\n");
				TriangleIndex = 0;
			}
		}
		TriangleContent.Append("-------------- End of TriangleBuffer --------------\n");

		bool AllowOverwriting = true;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// CreateDirectoryTree returns true if the destination
		// directory existed prior to call or has been created
		// during the call.
		if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
		{
			FString VertexAbsolutePath = SaveDirectory + "/" + VertexFileName;
			FString TriangleAbsolutePath = SaveDirectory + "/" + TriangleFileName;

			if (AllowOverwriting || !PlatformFile.FileExists(*VertexAbsolutePath))
			{
				FFileHelper::SaveStringToFile(VertexContent, *VertexAbsolutePath);
			}

			if (AllowOverwriting || !PlatformFile.FileExists(*TriangleAbsolutePath))
			{
				FFileHelper::SaveStringToFile(TriangleContent, *TriangleAbsolutePath);
			}
		}

		return;

	}


};
	
