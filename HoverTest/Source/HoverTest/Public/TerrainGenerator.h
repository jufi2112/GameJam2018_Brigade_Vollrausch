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

	// Delimiter used to separate X and Y coordinate values in FString Keys
	FString Delimiter = FString("=");

	// tunes the interpolation curve in the midpoint displacement bottom-up process
	float I_bu = -0.4f;



	FDEM()
	{
		DEM.Empty();
		AscendingPoints.Empty();
		ChildrenPoints.Empty();
	}

	FVector2D GetPointFromKey(FString TheKey)
	{
		FString FirstFloat;
		FString SecondFloat;
		if (!TheKey.Split(Delimiter, &FirstFloat, &SecondFloat))
		{
			UE_LOG(LogTemp, Warning, TEXT("Did not find splitting symbol %s in given text: %s"), *Delimiter, *TheKey);
		}
		FVector2D Coords;
		Coords.X = FCString::Atof(*FirstFloat);
		Coords.Y = FCString::Atof(*SecondFloat);
		return Coords;
	}

	FString GetKeyForPoint(const FVector2D Point) const
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.X, 2) + Delimiter + UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.Y, 2);
		return Key;
	}

	FString GetKeyForPoint(const FVector Point) const
	{
		FString Key = UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.X, 2) + Delimiter + UMyStaticLibrary::GetFloatAsStringWithPrecision(Point.Y, 2);
		return Key;
	}

	// for a given point, find all ascending points
	void GetAscendingPoints(const FVector2D OriginalPoint, TArray<FVector2D>& OUTAscendents) const
	{
		OUTAscendents = (*(AscendingPoints.Find(GetKeyForPoint(OriginalPoint))));
		//AscendingPoints.MultiFind(Key, OUTAscendents, true);
	}

	// for a given point, find all children points
	void GetChildrenPoints(const FVector2D OriginalPoint, TArray<FVector2D>& OUTChildren) const
	{
		ChildrenPoints.MultiFind(GetKeyForPoint(OriginalPoint), OUTChildren, true);
	}

	// add a point to the children point map for the given point
	void AddChildrenPoint(const FVector2D OriginalPoint, const FVector2D Children)
	{
		ChildrenPoints.Add(GetKeyForPoint(OriginalPoint), Children);
	}

	// gets the points elevation
	bool GetPointElevation(const FVector2D Point, float& OUTElevation) const
	{
		const FDEMData* Data = DEM.Find(GetKeyForPoint(Point));
		if (!Data) 
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find specified key (%s) in GetPointElevation"), *GetKeyForPoint(Point));
			return false; 
		}
		OUTElevation = Data->Elevation;
		return true;
	}

	// gets the points state
	bool GetPointState(const FVector2D Point, EDEMState& OUTState)
	{
		const FDEMData* Data = DEM.Find(GetKeyForPoint(Point));
		if (!Data) 
		{ 
			UE_LOG(LogTemp, Error, TEXT("Could not find specified key (%s) in GetPointState"), *GetKeyForPoint(Point));
			return false; 
		}
		OUTState = Data->State;
		return true;
	}

	// gets the point's whole FDEMData
	bool GetPointData(const FVector2D Point, FDEMData& OUTPointData)
	{
		const FDEMData* Data = DEM.Find(GetKeyForPoint(Point));
		if (!Data) 
		{ 
			UE_LOG(LogTemp, Error, TEXT("Could not find specified key (%s) in GetPointData"), *GetKeyForPoint(Point));
			return false;
		}
		OUTPointData.Elevation = Data->Elevation;
		OUTPointData.State = Data->State;
		return true;
	}

	int32 Sigma(const float I)
	{
		return (I >= 0 ? 1 : -1);
	}

	float DeltaBU(const float e, const float d)
	{
		// TODO implement formula given in paper
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
	void SimulateTriangleEdge(const TArray<FVector2D>* DefiningPoints, const int32 Iteration, const int32 MaxIterations)
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

		/* add defining points to DEM */
		for (FVector2D Vec : (*DefiningPoints))
		{
			// at this point we don't care if we overwrite already set DEM data, because they all get defaultet
			DEM.Add(GetKeyForPoint(Vec), FDEMData());
		}

		/* calculate new points */
		FVector2D E = FVector2D(HalfWidth, (*DefiningPoints)[0].Y);
		FVector2D F = FVector2D((*DefiningPoints)[1].X, HalfHeight);
		FVector2D G = FVector2D(HalfWidth, (*DefiningPoints)[2].Y);
		FVector2D H = FVector2D((*DefiningPoints)[0].X, HalfHeight);
		FVector2D I = FVector2D(HalfWidth, HalfHeight);

		/* add ascending points to hashmap */
		// E
		TArray<FVector2D> Values_E;
		Values_E.Add((*DefiningPoints)[0]);
		Values_E.Add((*DefiningPoints)[1]);
		AscendingPoints.Add(GetKeyForPoint(E), Values_E);
		// F
		TArray<FVector2D> Values_F;
		Values_F.Add((*DefiningPoints)[1]);
		Values_F.Add((*DefiningPoints)[2]);
		AscendingPoints.Add(GetKeyForPoint(F), Values_F);
		// G
		TArray<FVector2D> Values_G;
		Values_G.Add((*DefiningPoints)[2]);
		Values_G.Add((*DefiningPoints)[3]);
		AscendingPoints.Add(GetKeyForPoint(G), Values_G);
		// H
		TArray<FVector2D> Values_H;
		Values_H.Add((*DefiningPoints)[0]);
		Values_H.Add((*DefiningPoints)[3]);
		AscendingPoints.Add(GetKeyForPoint(H), Values_H);
		// I
		TArray<FVector2D> Values_I;
		Values_I.Add((*DefiningPoints)[0]);
		Values_I.Add((*DefiningPoints)[1]);
		Values_I.Add((*DefiningPoints)[2]);
		Values_I.Add((*DefiningPoints)[3]);
		AscendingPoints.Add(GetKeyForPoint(I), Values_I);


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
	* comments refer to the pseudo code provided in the paper above
	*/
	void MidpointDisplacementBottomUp(const TArray<FVector>* InitialConstraints)
	{
		// add constraints to the DEM
		for (const FVector Constraint : (*InitialConstraints))
		{
			DEM.Add(GetKeyForPoint(Constraint), FDEMData(Constraint.Z, EDEMState::DEM_KNOWN));
		}

		// FIFO Queue
		TQueue<FVector, EQueueMode::Spsc> FQ;
		// put all initial constraints in the FIFO Queue
		for (FVector Vec : *(InitialConstraints))
		{
			FQ.Enqueue(Vec);
		}
		while (!FQ.IsEmpty())
		{
			FVector E;
			while (FQ.Dequeue(E))
			{
				// get all ascendents A of E
				TArray<FVector2D> A;
				GetAscendingPoints(FVector2D(E.X, E.Y), A);
				for (FVector2D a : A)
				{
					FDEMData Data_a;
					if (!GetPointData(a, Data_a))
					{
						UE_LOG(LogTemp, Error, TEXT("Could not find specified ascending point in DEM in MidpointDisplacementBottomUp"));
						return;
					}
					if (Data_a.State == EDEMState::DEM_UNKNOWN)
					{
						// add A as key in hashtable and add E as value to hashtable : list of known child of A
						ChildrenPoints.Add(GetKeyForPoint(a), FVector2D(E.X, E.Y));
					}
				}
			}

			TArray<FString> Keys;
			ChildrenPoints.GetKeys(Keys);
			for (FString Key : Keys)
			{
				FVector2D A = GetPointFromKey(Key);
				float e = 0.f;
				int32 n = 0.f;
				TArray<FVector2D> Children;
				ChildrenPoints.MultiFind(Key, Children, true);
				for (FVector2D Child : Children)
				{
					//e = e +
				}
			}



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
