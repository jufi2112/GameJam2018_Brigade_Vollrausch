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
	// map to store all ascending points for a given point
	TMap<FString, TArray<FVector2D>> AscendingPoints;
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
		OUTAscendents = (*(AscendingPoints.Find(Key)));
		//AscendingPoints.MultiFind(Key, OUTAscendents, true);
	}

	// for a given point, find all children points
	void GetChildrenPoints(const FVector2D OriginalPoint, TArray<FVector2D>& OUTChildren) const
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.Y, 2);
		ChildrenPoints.MultiFind(Key, OUTChildren, true);
	}

	// add a point to the ascending point map for the given point
	/*void AddAscendingPoint(const FVector2D OriginalPoint, const FVector2D Ascendent)
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(OriginalPoint.Y, 2);
		AscendingPoints.Add(Key, Ascendent);
	}*/

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
	 * @param DefiningPoints - points that define the current quad (4 points overall) with ordering [A, B, C, D] where A = (min_X, min_Y) and C = (max_X, max_Y)
	 *
	 *			D *-------------* C
	 *			  |	\			|	
	 *			  |	   \		|
	 *			  |	     \		|
	 *			  |		   \	|
	 *			  |			  \ |
	 *			A *-------------* B  
	 *
	 * newly created points follow this order:
	 *
	 *					 G
	 *			D *------*------* C
	 *			  |	\			|
	 *			  |	   \   I	|
	 *			H *	     *		* F
	 *			  |		   \	|
	 *			  |			  \ |
	 *			A *------*------* B
	 *					 E
	 *
	 *	newly created quads are:	Quad1 = {A, E, I, H}
	 *								Quad2 = {E, B, F, I}
	 *								Quad3 = {I, F, C, G}
	 *								Quad4 = {H, I, G, D}
	 *
	 * @param Iteration - the current iteration depth the recursion is in
	 * @param MaxIterations - number of iterations after which the recursion stops
	 */
	void SimulateTriangleEdge(TArray<FVector2D>* DefiningPoints, const int32 Iteration, const int32 MaxIterations)
	{
		if (!DefiningPoints)
		{
			UE_LOG(LogTemp, Error, TEXT("DefiningPoints is nullptr in SimulateTriangleEdge at iteration %i"), Iteration);
			return;
		}
		if (DefiningPoints->Num() != 4)
		{
			UE_LOG(LogTemp, Error, TEXT("Wrong number of points given to SimulateTriangleEdge at iteration %i. Should be 5, are: %i"), Iteration, DefiningPoints->Num());
			return;
		}

		float HalfWidth = ((*DefiningPoints)[1].X - (*DefiningPoints)[0].X) / 2.f;
		float HalfHeight = ((*DefiningPoints)[3].Y - (*DefiningPoints)[0].Y) / 2.f;

		/* calculate new points */
		FVector2D E = FVector2D(HalfWidth, (*DefiningPoints)[0].Y);
		FVector2D F = FVector2D((*DefiningPoints)[1].X, HalfHeight);
		FVector2D G = FVector2D(HalfWidth, (*DefiningPoints)[2].Y);
		FVector2D H = FVector2D((*DefiningPoints)[0].X, HalfHeight);
		FVector2D I = FVector2D(HalfWidth, HalfHeight);

		/* add ascending points to hashmap */
		// E
		FString Key_E = UMyStaticLibrary::GetFloatAsStringWithPrecision(E.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(E.Y, 2);
		TArray<FVector2D> Values_E;
		Values_E.Add((*DefiningPoints)[0]);
		Values_E.Add((*DefiningPoints)[1]);
		AscendingPoints.Add(Key_E, Values_E);
		// F
		FString Key_F = UMyStaticLibrary::GetFloatAsStringWithPrecision(F.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(F.Y, 2);
		TArray<FVector2D> Values_F;
		Values_F.Add((*DefiningPoints)[1]);
		Values_F.Add((*DefiningPoints)[2]);
		AscendingPoints.Add(Key_F, Values_F);
		// G
		FString Key_G = UMyStaticLibrary::GetFloatAsStringWithPrecision(G.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(G.Y, 2);
		TArray<FVector2D> Values_G;
		Values_G.Add((*DefiningPoints)[2]);
		Values_G.Add((*DefiningPoints)[3]);
		AscendingPoints.Add(Key_G, Values_G);
		// H
		FString Key_H = UMyStaticLibrary::GetFloatAsStringWithPrecision(H.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(H.Y, 2);
		TArray<FVector2D> Values_H;
		Values_H.Add((*DefiningPoints)[0]);
		Values_H.Add((*DefiningPoints)[3]);
		AscendingPoints.Add(Key_H, Values_H);
		// I
		FString Key_I = UMyStaticLibrary::GetFloatAsStringWithPrecision(I.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(I.Y, 2);
		TArray<FVector2D> Values_I;
		Values_I.Add((*DefiningPoints)[0]);
		Values_I.Add((*DefiningPoints)[1]);
		Values_I.Add((*DefiningPoints)[2]);
		Values_I.Add((*DefiningPoints)[3]);
		AscendingPoints.Add(Key_I, Values_I);


		if (Iteration == MaxIterations)
		{
			// in simulation, do nothing
			return;
		}
		if (Iteration < MaxIterations)
		{
			// apply triangle edge algorithm on all newly created quads
			TArray<FVector2D> Quad1;
			Quad1.Add((*DefiningPoints)[0]);
			Quad1.Add(E);
			Quad1.Add(I);
			Quad1.Add(H);
			TArray<FVector2D> Quad2;
			Quad2.Add(E);
			Quad2.Add((*DefiningPoints)[1]);
			Quad2.Add(F);
			Quad2.Add(I);
			TArray<FVector2D> Quad3;
			Quad3.Add(I);
			Quad3.Add(F);
			Quad3.Add((*DefiningPoints)[2]);
			Quad3.Add(G);
			TArray<FVector2D> Quad4;
			Quad4.Add(H);
			Quad4.Add(I);
			Quad4.Add(G);
			Quad4.Add((*DefiningPoints)[3]);
			SimulateTriangleEdge(&Quad1, Iteration + 1, MaxIterations);
			SimulateTriangleEdge(&Quad2, Iteration + 1, MaxIterations);
			SimulateTriangleEdge(&Quad3, Iteration + 1, MaxIterations);
			SimulateTriangleEdge(&Quad4, Iteration + 1, MaxIterations);

			return;
		}
		else
		{
			return;
		}
	}

	/**
	* implementation of the MDBU algorithm from "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj (2007)
	*/
	void MidpointDisplacementBottomUp(TArray<FVector>* InitialConstraints)
	{
		// FIFO Queue
		TQueue<FVector, EQueueMode::Spsc> FQ;
		// put all initial constraints in the FIFO Queue
		for (FVector Vec : *(InitialConstraints))
		{
			FQ.Enqueue(Vec);
		}
		while (!FQ.IsEmpty)
		{
			FVector E;
			FQ.Dequeue(E);
			// get all ascendents of E
			FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(E.X, 2) + UMyStaticLibrary::GetFloatAsStringWithPrecision(E.Y, 2);
			TArray<FVector>* A = AscendingPoints.Find(Key);
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
