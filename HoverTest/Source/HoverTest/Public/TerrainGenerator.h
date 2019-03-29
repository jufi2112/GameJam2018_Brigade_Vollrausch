// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyStaticLibrary.h"
#include "TerrainGenerator.generated.h"

UENUM()
enum class EDEMState : uint8
{
	DEM_KNOWN,
	DEM_UNKNOWN
};

/**
 * struct that contains the relevant data of a DEM (a points elevation and its state)
 */
USTRUCT()
struct FDEMData
{
	GENERATED_USTRUCT_BODY()

	EDEMState State;
	float Elevation;

	FDEMData(float elevation)
	{
		Elevation = elevation;
		State = EDEMState::DEM_UNKNOWN;
	}

	FDEMData(float elevation, EDEMState state)
	{
		Elevation = elevation;
		State = state;
	}

	FDEMData()
	{
		Elevation = 0.f;
		State = EDEMState::DEM_UNKNOWN;
	}
};

/**
 * struct for a digital elevation map (DEM) as presented in "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj in 2007
 */
USTRUCT()
struct FDEM
{
	GENERATED_USTRUCT_BODY()
	// map to store DEM data for each point (using FString since points can have float coordinates that can suffer from precision problems)
	TMap<FString, FDEMData> DEM;
	// multimap to store all ascending points for a given point
	TMultiMap<FString, FVector2D> AscendingPoints;
	// multimap to store all children points for a given point
	TMultiMap<FString, FVector2D> ChildrenPoints;

	FDEM()
	{
		DEM.Empty();
		AscendingPoints.Empty();
		ChildrenPoints.Empty();
	}

	// for a given point, find all ascending points
	void GetAscendingPoints(const FVector2D OriginalPoint, TArray<FVector2D>& OUTAscendents) const
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.Y, 2);
		AscendingPoints.MultiFind(Key, OUTAscendents, true);
	}

	// for a given point, find all children points
	void GetChildrenPoints(const FVector2D OriginalPoint, TArray<FVector2D>& OUTChildren) const
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.Y, 2);
		ChildrenPoints.MultiFind(Key, OUTChildren, true);
	}

	// add a point to the ascending point map for the given point
	void AddAscendingPoint(const FVector2D OriginalPoint, const FVector2D Ascendent)
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.Y, 2);
		AscendingPoints.Add(Key, Ascendent);
	}

	// add a point to the children point map for the given point
	void AddChildrenPoint(const FVector2D OriginalPoint, const FVector2D Children)
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.Y, 2);
		ChildrenPoints.Add(Key, Children);
	}

	// gets the points elevation
	bool GetPointElevation(const FVector2D Point, float& Elevation) const
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.Y, 2);
		const FDEMData* Data = DEM.Find(Key);
		if (!Data) 
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find specified key (%s) in GetPointElevation"), *Key);
			return false; 
		}
		Elevation = Data->Elevation;
		return true;
	}

	// gets the points state
	bool GetPointState(const FVector2D Point, EDEMState& State)
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.Y, 2);
		const FDEMData* Data = DEM.Find(Key);
		if (!Data) 
		{ 
			UE_LOG(LogTemp, Error, TEXT("Could not find specified key (%s) in GetPointState"), *Key);
			return false; 
		}
		State = Data->State;
		return true;
	}

	// gets the point's whole FDEMData
	bool GetPointData(const FVector2D Point, FDEMData& PointData)
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.Y, 2);
		const FDEMData* Data = DEM.Find(Key);
		if (!Data) 
		{ 
			UE_LOG(LogTemp, Error, TEXT("Could not find specified key (%s) in GetPointData"), *Key);
			return false;
		}
		PointData.Elevation = Data->Elevation;
		PointData.State = Data->State;
		return true;
	}

	/**
	 * simulates the triangle edge algorithm
	 * i.e. executes the algorithm, but doesn't calculate any height displacements
	 * instead, only creates a hashmap with all ascending points for each point (AscendingPoints)
	 * @param DefiningPoints - points that define the current quad (5 points overall) with ordering [A, B, C, D, E]
	 *
	 *			D *-------------* C
	 *			  |	\			|	
	 *			  |	   \		|
	 *			  |	   E *		|
	 *			  |		   \	|
	 *			  |			  \ |
	 *			A *-------------* B  
	 * @param Iteration - the current iteration depth the recursion is in
	 * @param MaxIterations - number of iterations after which the recursion stops
	 */
	void SimulateTriangleEdge(TArray<FVector>* DefiningPoints, const int32 Iteration, const int32 MaxIterations)
	{
		if (!DefiningPoints)
		{
			UE_LOG(LogTemp, Error, TEXT("DefiningPoints is nullptr in SimulateTriangleEdge at iteration %i"), Iteration);
			return;
		}
		if (DefiningPoints->Num() != 5)
		{
			UE_LOG(LogTemp, Error, TEXT("Wrong number of points given to SimulateTriangleEdge at iteration %i. Should be 5, are: %i"), Iteration, DefiningPoints->Num());
			return;
		}
	}


};

/**
 * 
 */
UCLASS()
class HOVERTEST_API UTerrainGenerator : public UObject
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void GetTerrain(const FIntVector2D SectorToCreateTerrainFor, FTerrainJob& TerrainJob);
	
private:

	/**
	 * implementation of the MDBU algorithm from "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj
	 */
	//void MidpointDisplacementBottomUp()
};
