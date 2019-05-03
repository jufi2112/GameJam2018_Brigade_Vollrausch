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

	friend FORCEINLINE uint32 GetTypeHash(const FIntVector2D& Vector)
	{
		// Note: this assumes there's no padding in FINTVector2D that could contain uncompared data.
		//return FCrc::MemCrc_DEPRECATED(&Vector, sizeof(Vector));
		//return FCrc::MemCrc32(&Vector, sizeof(FIntVector2D));
		return FCrc::MemCrc_DEPRECATED(&Vector, sizeof(FIntVector2D));
	}
};

/**
 * struct that is used to store track information for each sector
 */
USTRUCT()
struct FSectorTrackInfo
{
	GENERATED_USTRUCT_BODY()

	// does the sector contains a track?
	bool bSectorHasTrack = false;

	// start point of the track in this sector
	UPROPERTY()
	FVector2D TrackEntryPoint = FVector2D();

	// end point of the track in this sector
	UPROPERTY()
	FVector2D TrackExitPoint = FVector2D();

	// previous track sector
	UPROPERTY()
	FIntVector2D PreviousTrackSector = FIntVector2D();

	// following track sector
	UPROPERTY()
	FIntVector2D FollowingTrackSector = FIntVector2D();

	// sector has no track
	FSectorTrackInfo()
	{

	}

	// sector has a track
	FSectorTrackInfo(FVector2D EntryPoint, FVector2D ExitPoint, FIntVector2D PreviousSector, FIntVector2D FollowingSector)
	{
		bSectorHasTrack = true;
		TrackEntryPoint = EntryPoint;
		TrackExitPoint = ExitPoint;
		PreviousTrackSector = PreviousSector;
		FollowingTrackSector = FollowingSector;
	}
};

/**
 * struct to store a track segment and perform calculations on it
 */
USTRUCT()
struct FTrackSegment
{
	GENERATED_USTRUCT_BODY()

	// points that define the track segment
	UPROPERTY()
	TArray<FVector2D> DefiningPoints;

	// values for the bounding rectangle of the above points

	float MinX = 0.f;
	float MaxX = 0.f;
	float MinY = 0.f;
	float MaxY = 0.f;

	/**
	 * minimum parameters such that Points with X in [i_min * UnitSize, i_max * UnitSize] and Y in [j_min * UnitSize, j_max * UnitSize] lie within rectanglular bounding box
	 */
	int32 i_min = 0;
	int32 i_max = 0;
	int32 j_min = 0;
	int32 j_max = 0;

	/**
	 * size between to adjacent terrain mesh vertices
	 */
	float UnitSize = 0.f;

	/**
	 * array of points that lie on the track segment (or its rectangular bounding box)
	 */
	UPROPERTY()
	TArray<FVector> PointsOnTrackSegment;

	/**
	 * the height of the base line of the track segment
	 * i.e. height of defining points 1 and 2
	 */
	float BaseLineHeight = 0.f;

	/**
	 * the height of the end line of the track segment
	 * i.e. height of defining points 3 and 4
	 */
	float EndLineHeight = 0.f;

	FTrackSegment(const FVector Point1, const FVector Point2, const FVector Point3, const FVector Point4)
	{
		DefiningPoints.Add(FVector2D(Point1.X, Point1.Y));
		DefiningPoints.Add(FVector2D(Point2.X, Point2.Y));
		DefiningPoints.Add(FVector2D(Point3.X, Point3.Y));
		DefiningPoints.Add(FVector2D(Point4.X, Point4.Y));

		BaseLineHeight = Point1.Z;
		EndLineHeight = Point3.Z;
	}

	/**
	 * calculates all points that lie on the given track segment
	 * @param UnitSize The distance between to adjacent mesh vertices
	 * @param UseTightBoundingBox Using a tight bounding box will return points that lie in the initially defined track segment, false if all points in the rectangular bounding box around the given track segment should be returned
	 * @param OUTPointsOnTrack All points that lie on the track as specified by UseTightBoundingBox with correct height
	 */
	void CalculatePointsOnTrack(const float UnitSize, const bool UseTightBoundingBox, TArray<FVector>& OUTPointsOnTrack)
	{
		this->UnitSize = UnitSize;

		CalculateBoundingRectangle();

		CalculatePointsInBoundingBox(UseTightBoundingBox);
		OUTPointsOnTrack = PointsOnTrackSegment;

	}

	/**
	 * ! For internal use only, use CalculatePointsOnTrack instead !
	 */
	void CalculateBoundingRectangle()
	{
		for (const FVector2D Point : DefiningPoints)
		{
			MinX = Point.X < MinX ? Point.X : MinX;
			MaxX = Point.X > MaxX ? Point.X : MaxX;
			MinY = Point.Y < MinY ? Point.Y : MinY;
			MaxY = Point.Y > MaxY ? Point.Y : MaxY;
		}
	}


	/**
	 * ! For internal use only, use CalculatePointsOnTrack instead !
	 */
	void CalculatePointsInBoundingBox(const bool UseTightBoundingBox)
	{
		j_min = FMath::FloorToInt(MinY / UnitSize);
		j_max = FMath::CeilToInt(MaxY / UnitSize);
		i_min = FMath::FloorToInt(MinX / UnitSize);
		i_max = FMath::CeilToInt(MaxX / UnitSize);

		// calculate points in rectangular bounding box
		for (int32 i = i_min; i <= i_max; ++i)
		{
			for (int32 j = j_min; j <= j_max; ++j)
			{
				PointsOnTrackSegment.Add(FVector(i * UnitSize, j * UnitSize));
			}
		}
		if (!UseTightBoundingBox) { return; }
		// use cross product (in 2D space determinant) to check if point lies within original track segment
		// point numeration switches from 1-based to 0-based to better comply with array indices
		FVector2D Vector_Point0Point1 = DefiningPoints[1] - DefiningPoints[0];
		FVector2D Vector_Point1Point2 = DefiningPoints[2] - DefiningPoints[1];
		FVector2D Vector_Point2Point3 = DefiningPoints[3] - DefiningPoints[2];
		FVector2D Vector_Point3Point0 = DefiningPoints[0] - DefiningPoints[3];

		// iterate all points and remove those that don't lie within original track segment
		for (int k = PointsOnTrackSegment.Num() - 1; k >= 0; --k)
		{
			FVector2D Vector_Point0P = PointsOnTrackSegment[k] - DefiningPoints[0];
			FVector2D Vector_Point1P = PointsOnTrackSegment[k] - DefiningPoints[1];
			FVector2D Vector_Point2P = PointsOnTrackSegment[k] - DefiningPoints[2];
			FVector2D Vector_Point3P = PointsOnTrackSegment[k] - DefiningPoints[3];

			float detA = FVector2D::CrossProduct(Vector_Point0Point1, Vector_Point0P);
			float detB = FVector2D::CrossProduct(Vector_Point1Point2, Vector_Point1P);
			float detC = FVector2D::CrossProduct(Vector_Point2Point3, Vector_Point2P);
			float detD = FVector2D::CrossProduct(Vector_Point3Point0, Vector_Point3P);

			// point lies within convex quad, if all cross poducts have the same sign
			if (!((detA <= 0 && detB <= 0 && detC <= 0 && detD <= 0) || (detA >= 0 && detB >= 0 && detC >= 0 && detD >= 0)))
			{
				PointsOnTrackSegment.RemoveAt(k);
			}
		}
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

/**
 * struct for fractal noise terrain generation settings
 * to be used as subcategory in FTerrainSettings
 * all parameters are used as suggested in "Terrain Modeling: A Constrained Fractal Model" by Far�s Belhadj in 2007
 */
USTRUCT(BlueprintType)
struct FFractalNoiseTerrainSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * used to translate random value interval [-1, 1] in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rt = 0.0f;

	/**
	 * scale factor to scale random value in random displacement calculation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rs = -0.5f;

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

	// number of iterations for the triangle edge algorithms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TriangleEdgeIterations = 5;
};

/**
 * struct with all settings to configure the track generation
 */
USTRUCT(BlueprintType)
struct FTrackGenerationSettings
{
	GENERATED_USTRUCT_BODY()

	/**
	 * defines how many points on the B�zier curve should be calculated
	 * a higher number creates a smoother mesh, but also increases performance costs
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackResolution = 30;

	/**
	 * the overall track width in cm
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackWidth = 500;

	/**
	 * if enabled, tries to only add those points as track constraints that lie on a track segment, otherwise all points in a rectangle around the track segment get added as constraints
	 * disable to save performance
	 */
	bool bUseTightTrackBoundingBox = true;
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
	int32 TileEdgeSize = 131072;

	// shall async collision cooking be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAsyncCollisionCooking = true;

	// fractal noise terrain generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFractalNoiseTerrainSettings FractalNoiseTerrainSettings;

	// track generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTrackGenerationSettings TrackGenerationSettings;

	// height in cm at which a transition from low terrain material to medium terrain material should occur
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainMaterialTransitionLowMediumElevation = 10000.f;

	// height in cm at which a transition from medium terrain material to high terrain material should occur
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainMaterialTransitionMediumHighElevation = 17000.f;

	// variation of the transition elevation medium high parameter (in cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionElevationVariationMediumHigh = 2000.f;

	// variation of the transition elevation low medium parameter (in cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionElevationVariationLowMedium = 1000.f;

	/** 
	 * array of materials that should be applied to the terrain
	 * index 0 is the default track material
	 * index 1 is the default terrain material for all elevations <= TerrainMaterialTransitionLowMediumElevation
	 * index 2 is the default terrain material for all elevations > TerrainMaterialTransitionLowMediumHeight and <= TerrainMaterialTransitionMediumHighHeight
	 * index 3 is the default terrain material for all elevations > TerrainMaterialTransitionMediumHighHeight
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 NumberOfThreadsToUse = 1;

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

	FTerrainJob()
	{
		MeshData.Init(FMeshData(), 4);
	}
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

	/**
	 * calculates the minimum distance between a point and a given line segment
	 * code adopted from https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	 * @param Point The Point for which the distance to the given line segment should be calculated
	 * @param LineStartPoint The start point of the line segment
	 * @param LineEndPoint The end point of the line segment
	 * @return Minimum distance between given point and line segment
	 */
	static float CalculateMinimumDistancePointLineSegment(const FVector2D Point, const FVector2D LineStartPoint, const FVector2D LineEndPoint)
	{
		const float L2 = FVector2D::DistSquared(LineStartPoint, LineEndPoint);
		if (FMath::IsNearlyZero(L2))
		{
			return FVector2D::Distance(Point, LineStartPoint);
		}
		const float t = FMath::Max<float>(0.f, FMath::Min<float>(1.f, FVector2D::DotProduct(Point - LineStartPoint, LineEndPoint - LineStartPoint) / L2));
		
		const FVector2D Projection = LineStartPoint + t * (LineEndPoint - LineStartPoint);
		return FVector2D::Distance(Point, Projection);
	}


};
	
