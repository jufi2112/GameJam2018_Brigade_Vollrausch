// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyStaticLibrary.h"
#include <random>
#include "RuntimeMeshComponent.h"
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

	UPROPERTY()
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
 * struct because unreal c++ won't allow nested containers
 */
USTRUCT()
struct FVector2DArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FVector2D> Array;

	FVector2DArray(const TArray<FVector2D>& ArrayToInitialize)
	{
		Array = ArrayToInitialize;
	}

	FVector2DArray()
	{
		Array.Empty();
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
	UPROPERTY()
	TMap<FString, FDEMData> DEM;
	// map to store all ascending points for a given point
	UPROPERTY()
	TMap<FString, FVector2DArray> AscendingPoints;
	// multimap to store all children points for a given point
	//UPROPERTY()
	TMultiMap<FString, FVector2D> ChildrenPoints;

	/**
	 * array that contains all vertices on the left border of the tile
	 */
	UPROPERTY()
	TArray<FVector> VerticesLeftBorder;

	/**
	 * array that contains all vertices on the top border of the tile
	 */
	UPROPERTY()
	TArray<FVector> VerticesTopBorder;

	/**
	 * array that contains all vertices on the right border of the tile
	 */
	UPROPERTY()
	TArray<FVector> VerticesRightBorder;

	/**
	 * array that contains all vertices on the bottom border of the tile
	 */
	UPROPERTY()
	TArray<FVector> VerticesBottomBorder;

	/**
	 * values for each border
	 */
	float XLeftBorder = 0.f;
	float XRightBorder = 0.f;
	float YTopBorder = 0.f;
	float YBottomBorder = 0.f;

	/**
	 * the four corner points of the DEM
	 */
	UPROPERTY()
	FVector BottomLeftCorner;
	UPROPERTY()
	FVector BottomRightCorner;
	UPROPERTY()
	FVector TopLeftCorner;
	UPROPERTY()
	FVector TopRightCorner;

	// Delimiter used to separate X and Y coordinate values in FString Keys
	FString Delimiter = FString("=");

	// tunes the interpolation curve in the midpoint displacement bottom-up process
	float I_bu = -0.4f;

	// tunes the interpolation curve in the midpoint displacement process
	float I = -0.4f;

	/** 
	 * the DEM diagonal, used in Delta_BU and Delta calculation
	 * calculated during SimulateTriangleEdge
	 */ 
	float d_max = 0.f;

	// fractal dimension
	float H = -0.5f;

	/**
	 * scaling factor, used in Deviation calculation
	 * @DEPRECATED
	 */
	float k = 100.f;

	// translated the random number in the displacement calculation
	float rt = 0.2f;

	// scaling factor for displacement calculation
	float rs = 1.f;

	// spatial dimension used in displacement calculation
	float n = 3.f;

	/** 
	 * hügeligkeit, used as mean for normal distribution
	 * @DEPRECATED
	 */
	float Hilly = 600.f;


	/**
	 * ! Please use other constructor so that terrain setting variables can be used !
	 * @DEPRECATED
	 */
	FDEM()
	{
		DEM.Empty();
		AscendingPoints.Empty();
		ChildrenPoints.Empty();
	}

	FDEM(const float H_FractalDimension, const float I_InterpolationCurveTuning, const float I_bu_InterpolationCurveTuning, const float rt_RandomNumberTranslation, const float rs_ScaleFactorRandomNumber, const float n_SpatialDomainRandomNumber)
	{
		DEM.Empty();
		AscendingPoints.Empty();
		ChildrenPoints.Empty();
		H = H_FractalDimension;
		I = I_InterpolationCurveTuning;
		I_bu = I_bu_InterpolationCurveTuning;
		rt = rt_RandomNumberTranslation;
		rs = rs_ScaleFactorRandomNumber;
		n = n_SpatialDomainRandomNumber;
	}

	void GetVerticesLeftBorder(TArray<FVector>& OUTVertices) const
	{
		OUTVertices.Append(VerticesLeftBorder);
	}

	void GetVerticesTopBorder(TArray<FVector>& OUTVertices) const
	{
		OUTVertices.Append(VerticesTopBorder);
	}

	void GetVerticesRightBorder(TArray<FVector>& OUTVertices) const
	{
		OUTVertices.Append(VerticesRightBorder);
	}

	void GetVerticesBottomBorder(TArray<FVector>& OUTVertices) const
	{
		OUTVertices.Append(VerticesBottomBorder);
	}

	FVector2D GetPointFromKey(FString TheKey)
	{
		FString FirstFloat;
		FString SecondFloat;
		if (!TheKey.Split(Delimiter, &FirstFloat, &SecondFloat))
		{
			UE_LOG(LogTemp, Warning, TEXT("Did not find splitting symbol %s in given text: %s"), *Delimiter, *TheKey);
			return FVector2D(0.f, 0.f);
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
		const FVector2DArray* Ascendents = AscendingPoints.Find(GetKeyForPoint(OriginalPoint));
		if (!Ascendents)
		{
			// this is the normal case for all points that define the first, big quad
			//UE_LOG(LogTemp, Error, TEXT("Did not find any ascending points for original point %s"), *OriginalPoint.ToString());
			return;
		}
		OUTAscendents = Ascendents->Array;
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

	// gets the point elevation
	bool GetPointElevation(const FVector Point, float& OUTElevation) const
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

	// gets the points state
	bool GetPointState(const FVector Point, EDEMState& OUTState)
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

	/** 
	 * set new DEMData
	 * @param Point The point for which DEMData should be set
	 * @param NewPointData The DEMData that should be set
	 * @param WithCheck If the DEMData should only be set if the previous DEMData's state is DEM_UNKNOWN
	 */
	void SetNewDEMPointData(const FVector2D Point, const FDEMData NewPointData, bool WithCheck = false)
	{
		if (WithCheck)
		{
			EDEMState State;
			if (GetPointState(Point, State))
			{
				if (State == EDEMState::DEM_KNOWN)
				{
					return;
				}
			}
		}
		DEM.Add(GetKeyForPoint(Point), NewPointData);
	}

	/**
	* set new DEMData
	* @param Point The point for which DEMData should be set
	* @param NewPointData The DEMData that should be set
	* @param WithCheck If the DEMData should only be set if the previous DEMData's state is DEM_UNKNOWN
	*/
	void SetNewDEMPointData(const FVector Point, const FDEMData NewPointData, bool WithCheck = false)
	{
		if (WithCheck)
		{
			EDEMState State;
			if (GetPointState(Point, State))
			{
				if (State == EDEMState::DEM_KNOWN)
				{
					return;
				}
			}
		}
		DEM.Add(GetKeyForPoint(Point), NewPointData);
	}

	int32 Sigma(const float I) const
	{
		return (I >= 0 ? 1 : -1);
	}

	float Delta_BU(const float e, const float d) const
	{
		if (d_max == 0.f)
		{
			UE_LOG(LogTemp, Error, TEXT("Division By Zero in Delta_BU function, returned 0.f instead!"));
			return 0.f;
		}
		return (e * (1 - Sigma(I_bu) * (1 - FMath::Pow((1 - (d / d_max)), FMath::Abs(I_bu)))));
	}

	/**
	 * non-linear interpolation as suggested in "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj in 2007
	 * @param e Elevation of the ascendant cell
	 * @param d Euclidean distance from ascendant cell to child cell
	 * @return Non-linear interpolated elevation
	 */
	float Delta(const float e, const float d) const
	{
		if (d_max == 0.f)
		{
			UE_LOG(LogTemp, Error, TEXT("Division By Zero in Delta function, returned 0.f instead!"));
			return 0.f;
		}
		return (e * (1 - Sigma(I) * (1 - FMath::Pow((1 - (d / d_max)), FMath::Abs(I)))));
	}

	// removes all values associated with the given point from the map
	void RemoveValuesFromChildrenPoints(const FVector2D Point)
	{
		int32 NumDeleted = ChildrenPoints.Remove(GetKeyForPoint(Point));
		if (NumDeleted == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find any values associated with Point %s in RemoveValuesFromChildrenPoints"), *Point.ToString());
		}
		return;
	}

	/**
	 * interpolates between the two given floats (averages)
	 */
	float InterpolateFloat(const float Value1, const float Value2) const
	{
		return ((Value1 + Value2) / 2.f);
	}

	/**
	 * @DEPRECATED
	 * returns a normal distribution, code taken from https://forums.unrealengine.com/development-discussion/c-gameplay-programming/9284-normal-distribution comment by BNash
	 * @param Mean The Mean value of the normal distribution
	 * @param Deviation The standard deviation of the normal distribution
	 */
	float GetNormalDistribution(const float Mean, const float Deviation) const
	{
		std::random_device rd{};
		std::mt19937 gen{ rd() };
		std::normal_distribution<float> d{ Mean, Deviation };

		return d(gen);
	}

	/**
	 * calculates a signed random displacement as proposed in "Terrain Modeling: A Constrained Fractal Model" by Farès Belhadj in 2007
	 */
	float GetRandomDisplacement(const int32 Iteration) const
	{
		return ((FMath::RandRange(-1.f, 1.f) + rt) * rs * FMath::Pow(2, (-Iteration * n * H)));
	}

	float CalculateDeviation(const float Iteration) const
	{
		return (k * FMath::Pow(2, (-((Iteration) * H))));
	}

	/**
	 * calculates the child points elevation based on interpolation of the ascendant points and a random displacement
	 * This function only considers two ascending points
	 * @param CurrentIteration The current iteration of the calling algorithm
	 * @param ChildPoint The point for which the elevation should be calculated
	 * @param AscendantOne The first ascendant of the child point
	 * @param AscendantTwo The second ascendant of the child point
	 * @return Interpolated elevation of the child point, displaced by a random value
	 */
	float CalculatePointElevation(const int32 CurrentIteration, const FVector ChildPoint, const FVector AscendantOne, const FVector AscendantTwo)
	{
		float PointElevation = InterpolateFloat
		(
			Delta
			(
				AscendantOne.Z,
				FVector2D::Distance
				(
					Vec2Vec2D(AscendantOne),
					Vec2Vec2D(ChildPoint)
				)
			),
			Delta
			(
				AscendantTwo.Z,
				FVector2D::Distance
				(
					Vec2Vec2D(AscendantTwo),
					Vec2Vec2D(ChildPoint)
				)
			)
		);

		PointElevation += (FVector2D::Distance(Vec2Vec2D(AscendantOne), Vec2Vec2D(ChildPoint)) * GetRandomDisplacement(CurrentIteration));
		PointElevation += (FVector2D::Distance(Vec2Vec2D(AscendantTwo), Vec2Vec2D(ChildPoint)) * GetRandomDisplacement(CurrentIteration));
		return PointElevation;
	}
	
	/**
	* calculates the child points elevation based on interpolation of the ascendant points and a random displacement
	* This function considers four ascending points
	* Be aware that AscendantOne & AscendantTwo define one interpolation line and AscendantThree & AscendantFour define another interpolation line
	* @param CurrentIteration The current iteration of the calling algorithm
	* @param ChildPoint The point for which the elevation should be calculated
	* @param AscendantOne The first ascendant of the child point
	* @param AscendantTwo The second ascendant of the child point
	* @param AscendantThree The third ascendant of the child point
	* @param AscendantFour The fourth ascendant of the child point
	* @return Interpolated elevation of the child point, displaced by a random value
	*/
	float CalculatePointElevation(const int32 CurrentIteration, const FVector ChildPoint, const FVector AscendantOne, const FVector AscendantTwo, const FVector AscendantThree, const FVector AscendantFour)
	{
		float PointElevation = InterpolateFloat
		(
			InterpolateFloat
			(
				Delta
				(
					AscendantOne.Z,
					FVector2D::Distance
					(
						Vec2Vec2D(AscendantOne),
						Vec2Vec2D(ChildPoint)
					)
				),
				Delta
				(
					AscendantTwo.Z,
					FVector2D::Distance
					(
						Vec2Vec2D(AscendantTwo),
						Vec2Vec2D(ChildPoint)
					)
				)
			),
			InterpolateFloat
			(
				Delta
				(
					AscendantThree.Z,
					FVector2D::Distance
					(
						Vec2Vec2D(AscendantThree),
						Vec2Vec2D(ChildPoint)
					)
				),
				Delta
				(
					AscendantFour.Z,
					FVector2D::Distance
					(
						Vec2Vec2D(AscendantFour),
						Vec2Vec2D(ChildPoint)
					)
				)
			)
		);

		PointElevation += (FVector2D::Distance(Vec2Vec2D(AscendantOne), Vec2Vec2D(ChildPoint)) * GetRandomDisplacement(CurrentIteration+1));
		PointElevation += (FVector2D::Distance(Vec2Vec2D(AscendantTwo), Vec2Vec2D(ChildPoint)) * GetRandomDisplacement(CurrentIteration+1));
		PointElevation += (FVector2D::Distance(Vec2Vec2D(AscendantThree), Vec2Vec2D(ChildPoint)) * GetRandomDisplacement(CurrentIteration+1));
		PointElevation += (FVector2D::Distance(Vec2Vec2D(AscendantFour), Vec2Vec2D(ChildPoint)) * GetRandomDisplacement(CurrentIteration+1));

		return PointElevation;
	}


	// Creates a FRuntimeMeshVertexSimple from the given Vertex
	FRuntimeMeshVertexSimple CreateRuntimeMeshVertexSimple(const FVector Vertex)
	{
		return FRuntimeMeshVertexSimple(
			Vertex,										// Vertex position
			FVector(0.f, 0.f, 1.f),						// Vertex normal
			FRuntimeMeshTangent(0.f, -1.f, 0.f),		// Vertex tangent
			FColor::White,								
			FVector2D(Vertex.X/500.f, Vertex.Y/500.f)	// Vertex texture coordinates
		);
	}

	/**
	 * checks if the given vertex is located on a border and adds the vertex to the respective array if so
	 * the vertex X and Y values get modified so that they can directly be used as constraints for other DEMs 
	 * that means for example: the X value of a vertex located at the right border will be zeroed, while the X coordinate of a vertex on the left border will become XRightBorder
	 */
	void CheckForBorderVertex(const FVector Vertex)
	{
		// Left border -> in unreals coordinate system, this will become the top border (when viewing in positive X direction)
		if (FMath::IsNearlyEqual(UMyStaticLibrary::GetFloatWithPrecision(Vertex.X, 2), UMyStaticLibrary::GetFloatWithPrecision(XLeftBorder, 2)))
		{
			VerticesLeftBorder.Add(FVector(XRightBorder, Vertex.Y, Vertex.Z));
		}

		// Right border -> in unreals coordinate system, this will become the bottom border (when viewing in positive X direction)
		if (FMath::IsNearlyEqual(UMyStaticLibrary::GetFloatWithPrecision(Vertex.X, 2), UMyStaticLibrary::GetFloatWithPrecision(XRightBorder, 2)))
		{
			VerticesRightBorder.Add(FVector(XLeftBorder, Vertex.Y, Vertex.Z));
		}

		// top border -> in unreals coordinate system, this will become the right border (when viewing in positive X direction)
		if (FMath::IsNearlyEqual(UMyStaticLibrary::GetFloatWithPrecision(Vertex.Y, 2), UMyStaticLibrary::GetFloatWithPrecision(YTopBorder, 2)))
		{
			VerticesTopBorder.Add(FVector(Vertex.X, YBottomBorder, Vertex.Z));
		}

		// bottom border -> in unreals coordinate system, this will become the left border (when viewing in positive X direction)
		if (FMath::IsNearlyEqual(UMyStaticLibrary::GetFloatWithPrecision(Vertex.Y, 2), UMyStaticLibrary::GetFloatWithPrecision(YBottomBorder, 2)))
		{
			VerticesBottomBorder.Add(FVector(Vertex.X, YTopBorder, Vertex.Z));
		}
	}

	void SaveDEMToFile()
	{
		FString SaveDirectory = "D:/Users/Julien/Documents/Unreal Engine Dumps";
		FString FileName = "DEM.txt";
		FString DEMContent = "";

		TArray<FString> KeyArray;
		DEM.GetKeys(KeyArray);
		DEMContent.Append("-------------- Begin of DEM Data --------------\n");
		for (const FString Key : KeyArray)
		{
			FVector2D Point = GetPointFromKey(Key);
			DEMContent.Append(Point.ToString() + "X: ");
			int32 X = static_cast<int32>(Point.X);
			int32 Y = static_cast<int32>(Point.Y);
			DEMContent.Append(FString::FromInt(X) + "Y: " + FString::FromInt(Y) + ": DEM Data: ");


			FDEMData* Data = DEM.Find(Key);
			if (!Data)
			{
				UE_LOG(LogTemp, Error, TEXT("Could not retrieve Value for (existing!) key %s in SaveDEMToFile"), *Key);
				DEMContent.Append(" Elevation: unknown, State: unknown \n");
				continue;
			}
			else
			{
				FString State;
				if (Data->State == EDEMState::DEM_KNOWN) { State = "known"; }
				if (Data->State == EDEMState::DEM_UNKNOWN) { State = "unknown"; }
				DEMContent.Append(" Elevation: " + FString::SanitizeFloat(Data->Elevation, 3) + " State: " + State + "\n");
			}
		}
		DEMContent.Append("-------------- End of DEM Data --------------\n");

		bool AllowOverwriting = true;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// CreateDirectoryTree returns true if the destination
		// directory existed prior to call or has been created
		// during the call.
		if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
		{
			FString VertexAbsolutePath = SaveDirectory + "/" + FileName;

			if (AllowOverwriting || !PlatformFile.FileExists(*VertexAbsolutePath))
			{
				FFileHelper::SaveStringToFile(DEMContent, *VertexAbsolutePath);
			}
		}

		return;
	}

	/**
	 * adds the given vertices to the vertex buffer and the resulting triangle to the triangle buffer
	 * @return The next available VertexBuffer index (the next index to be used)
	 */
	int32 AddTriangleToBuffers(const FVector Vertex1, const FVector Vertex2, const FVector Vertex3, const int32 NextVertexBufferIndex, TArray<FRuntimeMeshVertexSimple>& OUTVertexBuffer, TArray<int32>& OUTTriangleBuffer)
	{
		OUTVertexBuffer.Add(CreateRuntimeMeshVertexSimple(Vertex1));
		OUTVertexBuffer.Add(CreateRuntimeMeshVertexSimple(Vertex2));
		OUTVertexBuffer.Add(CreateRuntimeMeshVertexSimple(Vertex3));
		OUTTriangleBuffer.Add(NextVertexBufferIndex);
		OUTTriangleBuffer.Add(NextVertexBufferIndex + 1);
		OUTTriangleBuffer.Add(NextVertexBufferIndex + 2);
		return (NextVertexBufferIndex + 3);
	}

	// converts the given FVector to FVector2D
	FVector2D Vec2Vec2D(const FVector Vector) const
	{
		return FVector2D(Vector.X, Vector.Y);
	}


	/**
	 * triangle edge algorithm
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
	 * newly created quads are:	Quad1 = {A, E, I, H}
	 *							Quad2 = {E, B, F, I}
	 *							Quad3 = {I, F, C, G}
	 *							Quad4 = {H, I, G, D}
	 *
	 * @param Iteration - the current iteration depth the recursion is in
	 * @param MaxIterations - number of iterations after which the recursion stops
	 */
	void TriangleEdge(const TArray<FVector>* DefiningPoints, const int32 Iteration, const int32 MaxIterations, TArray<FRuntimeMeshVertexSimple>& OUTVertexBuffer, TArray<int32>& OUTTriangleBuffer)
	{
		if (!DefiningPoints)
		{
			UE_LOG(LogTemp, Error, TEXT("DefiningPoints is nullptr in TriangleEdge at iteration %i"), Iteration);
			return;
		}
		if (DefiningPoints->Num() != 4)
		{
			UE_LOG(LogTemp, Error, TEXT("Wrong number of points given to TriangleEdge at iteration %i. Should be 4, are: %i"), Iteration, DefiningPoints->Num());
			return;
		}

		if (Iteration == 0)
		{
			// check if defining points are constraints, if not, we set them to 0.f elevation and state known, since we don't have ascendents to extrapolate their elevation
			EDEMState State;
			if (GetPointState((*DefiningPoints)[0], State))
			{
				if (State == EDEMState::DEM_UNKNOWN)
				{
					SetNewDEMPointData((*DefiningPoints)[0], FDEMData(0.f, EDEMState::DEM_KNOWN));
				}
			}
			if (GetPointState((*DefiningPoints)[1], State))
			{
				if (State == EDEMState::DEM_UNKNOWN)
				{
					SetNewDEMPointData((*DefiningPoints)[1], FDEMData(0.f, EDEMState::DEM_KNOWN));
				}
			}
			if (GetPointState((*DefiningPoints)[2], State))
			{
				if (State == EDEMState::DEM_UNKNOWN)
				{
					SetNewDEMPointData((*DefiningPoints)[2], FDEMData(0.f, EDEMState::DEM_KNOWN));
				}
			}
			if (GetPointState((*DefiningPoints)[3], State))
			{
				if (State == EDEMState::DEM_UNKNOWN)
				{
					SetNewDEMPointData((*DefiningPoints)[3], FDEMData(0.f, EDEMState::DEM_KNOWN));
				}
			}

			// calculate border values
			XLeftBorder = (*DefiningPoints)[0].X;
			XRightBorder = (*DefiningPoints)[2].X;
			YTopBorder = (*DefiningPoints)[2].Y;
			YBottomBorder = (*DefiningPoints)[0].Y;

			CheckForBorderVertex((*DefiningPoints)[0]);
			CheckForBorderVertex((*DefiningPoints)[1]);
			CheckForBorderVertex((*DefiningPoints)[2]);
			CheckForBorderVertex((*DefiningPoints)[3]);
		}

		float HalfWidth  = (((*DefiningPoints)[1].X - (*DefiningPoints)[0].X) / 2.f) + (*DefiningPoints)[0].X;
		float HalfHeight = (((*DefiningPoints)[3].Y - (*DefiningPoints)[0].Y) / 2.f) + (*DefiningPoints)[0].Y;

		// don't need to add DefiningPoints to DEM, because at iteration 0 those are constraints and at iteration i, those points have been added to the DEM at iteration (i-1)

		// calculate new points
		FVector E = FVector(HalfWidth, (*DefiningPoints)[0].Y, 0.f);
		FVector F = FVector((*DefiningPoints)[1].X, HalfHeight, 0.f);
		FVector G = FVector(HalfWidth, (*DefiningPoints)[2].Y, 0.f);
		FVector H = FVector((*DefiningPoints)[0].X, HalfHeight, 0.f);
		FVector I = FVector(HalfWidth, HalfHeight, 0.f);

		/* calculate elevation of newly created points */

		//float Deviation = CalculateDeviation(Iteration);
		//UE_LOG(LogTemp, Error, TEXT("Deviation: %f"), Deviation);

		// E
		EDEMState State;
		if (GetPointState(E, State))
		{
			if (State == EDEMState::DEM_UNKNOWN)
			{
				// ascendents are A & B

				//float PointElevation = InterpolateFloat((*DefiningPoints)[0].Z, (*DefiningPoints)[1].Z);
				//E.Z = PointElevation + GetNormalDistribution(Hilly, Deviation);
				//E.Z = PointElevation + GetNormalDistribution(Hilly, Deviation);
				/*float Displacement = GetRandomDisplacement(Iteration);
				UE_LOG(LogTemp, Warning, TEXT("Displacement: %f"), Displacement);
				E.Z = PointElevation + Displacement;*/
				E.Z = CalculatePointElevation(Iteration, E, (*DefiningPoints)[0], (*DefiningPoints)[1]);

				// add newly created point to DEM
				SetNewDEMPointData(E, FDEMData(E.Z, EDEMState::DEM_KNOWN));

			}
			else
			{
				// since we directly use E's coordinates in our last iteration (and we also pass them on to the next iteration if we're not at the final iteration) we need to get the points elevation from the DEM data
				GetPointElevation(E, E.Z);
			}
			CheckForBorderVertex(E);
		}
		else
		{
			// TODO check if we need to add the point if it cannot be found (most likely not necessary)
			UE_LOG(LogTemp, Error, TEXT("Could not find Point E=%s in DEM Data in TriangleEdge algorithm at iteration %i"), *E.ToString(), Iteration);
		}

		// F
		if (GetPointState(F, State))
		{
			if (State == EDEMState::DEM_UNKNOWN)
			{
				// ascendents are B & C
				/*float PointElevation = InterpolateFloat((*DefiningPoints)[1].Z, (*DefiningPoints)[2].Z);
				F.Z = PointElevation + GetNormalDistribution(Hilly, Deviation);*/
				/*float Displacement = GetRandomDisplacement(Iteration);
				UE_LOG(LogTemp, Warning, TEXT("Displacement: %f"), Displacement);
				F.Z = PointElevation + Displacement;*/
				F.Z = CalculatePointElevation(Iteration, F, (*DefiningPoints)[1], (*DefiningPoints)[2]);

				// add newly created point to DEM
				SetNewDEMPointData(F, FDEMData(F.Z, EDEMState::DEM_KNOWN));
			}
			else
			{
				GetPointElevation(F, F.Z);
			}
			CheckForBorderVertex(F);
		}
		else
		{
			// TODO check if we need to add the point if it cannot be found (most likely not necessary)
			UE_LOG(LogTemp, Error, TEXT("Could not find Point F=%s in DEM Data in TriangleEdge algorithm at iteration %i"), *F.ToString(), Iteration);
		}

		// G
		if (GetPointState(G, State))
		{
			if (State == EDEMState::DEM_UNKNOWN)
			{
				// ascendents are C & D
				/*float PointElevation = InterpolateFloat((*DefiningPoints)[2].Z, (*DefiningPoints)[3].Z);
				G.Z = PointElevation + GetNormalDistribution(Hilly, Deviation);*/
				/*float Displacement = GetRandomDisplacement(Iteration);
				UE_LOG(LogTemp, Warning, TEXT("Displacement: %f"), Displacement);
				G.Z = PointElevation + Displacement;*/
				G.Z = CalculatePointElevation(Iteration, G, (*DefiningPoints)[2], (*DefiningPoints)[3]);

				// add newly created point to DEM
				SetNewDEMPointData(G, FDEMData(G.Z, EDEMState::DEM_KNOWN));
			}
			else
			{
				GetPointElevation(G, G.Z);
			}
			CheckForBorderVertex(G);
		}
		else
		{
			// TODO check if we need to add the point if it cannot be found (most likely not necessary)
			UE_LOG(LogTemp, Error, TEXT("Could not find Point G=%s in DEM Data in TriangleEdge algorithm at iteration %i"), *G.ToString(), Iteration);
		}

		// H
		if (GetPointState(H, State))
		{
			if (State == EDEMState::DEM_UNKNOWN)
			{
				// ascendents are D & A
				/*float PointElevation = InterpolateFloat((*DefiningPoints)[3].Z, (*DefiningPoints)[0].Z);
				H.Z = PointElevation + GetNormalDistribution(Hilly, Deviation);*/
				/*float Displacement = GetRandomDisplacement(Iteration);
				UE_LOG(LogTemp, Warning, TEXT("Displacement: %f"), Displacement);
				H.Z = PointElevation + Displacement;*/
				H.Z = CalculatePointElevation(Iteration, H, (*DefiningPoints)[3], (*DefiningPoints)[0]);

				// add newly created point to DEM
				SetNewDEMPointData(H, FDEMData(H.Z, EDEMState::DEM_KNOWN));
			}
			else
			{
				GetPointElevation(H, H.Z);
			}
			CheckForBorderVertex(H);
		}
		else
		{
			// TODO check if we need to add the point if it cannot be found (most likely not necessary)
			UE_LOG(LogTemp, Error, TEXT("Could not find Point H=%s in DEM Data in TriangleEdge algorithm at iteration %i"), *H.ToString(), Iteration);
		}

		// I
		if (GetPointState(I, State))
		{
			if (State == EDEMState::DEM_UNKNOWN)
			{
				// ascendents are (A & C) && (B & D)
				/*float PointElevation = (InterpolateFloat((*DefiningPoints)[0].Z, (*DefiningPoints)[2].Z) + InterpolateFloat((*DefiningPoints)[1].Z, (*DefiningPoints)[3].Z)) / 2.f;
				I.Z = PointElevation + GetNormalDistribution(Hilly, Deviation);*/
				/*float Displacement = GetRandomDisplacement(Iteration);
				UE_LOG(LogTemp, Warning, TEXT("Displacement: %f"), Displacement);
				I.Z = PointElevation + Displacement;*/
				I.Z = CalculatePointElevation(Iteration, I, (*DefiningPoints)[0], (*DefiningPoints)[2], (*DefiningPoints)[1], (*DefiningPoints)[3]);

				// add newly created point to DEM
				SetNewDEMPointData(I, FDEMData(I.Z, EDEMState::DEM_KNOWN));
			}
			else
			{
				GetPointElevation(I, I.Z);
			}
			CheckForBorderVertex(I);
		}
		else
		{
			// TODO check if we need to add the point if it cannot be found (most likely not necessary)
			UE_LOG(LogTemp, Error, TEXT("Could not find Point I=%s in DEM Data in TriangleEdge algorithm at iteration %i"), *I.ToString(), Iteration);
		}

		// add newly created points to DEM if not already present

		//// E
		//SetNewDEMPointData(E, FDEMData(E.Z, EDEMState::DEM_KNOWN), true);
		//// F
		//SetNewDEMPointData(F, FDEMData(F.Z, EDEMState::DEM_KNOWN), true);
		//// G
		//SetNewDEMPointData(G, FDEMData(G.Z, EDEMState::DEM_KNOWN), true);
		//// H
		//SetNewDEMPointData(H, FDEMData(H.Z, EDEMState::DEM_KNOWN), true);
		//// I
		//SetNewDEMPointData(I, FDEMData(I.Z, EDEMState::DEM_KNOWN), true);

		if (Iteration == MaxIterations)
		{
			// fill vertex & triangle buffer

			// the next available vertex buffer index
			int32 VertexIndex = OUTVertexBuffer.Num();

			// triangle A, E, H
			VertexIndex = AddTriangleToBuffers((*DefiningPoints)[0], H, E, VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle E, I, H
			VertexIndex = AddTriangleToBuffers(H, I, E, VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle E, B, I
			VertexIndex = AddTriangleToBuffers(E, I, (*DefiningPoints)[1], VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle B, F, I
			VertexIndex = AddTriangleToBuffers(I, F, (*DefiningPoints)[1], VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle H, I, D
			VertexIndex = AddTriangleToBuffers(H, (*DefiningPoints)[3], I, VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle I, G, D
			VertexIndex = AddTriangleToBuffers(I, (*DefiningPoints)[3], G, VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle I, F, G
			VertexIndex = AddTriangleToBuffers(I, G, F, VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			// triangle F, C, G
			VertexIndex = AddTriangleToBuffers(F, G, (*DefiningPoints)[2], VertexIndex, OUTVertexBuffer, OUTTriangleBuffer);

			return;
		}
		if (Iteration < MaxIterations)
		{
			// apply triangle edge algorithm on all newly created quads
			TArray<FVector> Quad1;
			Quad1.Add((*DefiningPoints)[0]);
			Quad1.Add(E);
			Quad1.Add(I);
			Quad1.Add(H);
			TArray<FVector> Quad2;
			Quad2.Add(E);
			Quad2.Add((*DefiningPoints)[1]);
			Quad2.Add(F);
			Quad2.Add(I);
			TArray<FVector> Quad3;
			Quad3.Add(I);
			Quad3.Add(F);
			Quad3.Add((*DefiningPoints)[2]);
			Quad3.Add(G);
			TArray<FVector> Quad4;
			Quad4.Add(H);
			Quad4.Add(I);
			Quad4.Add(G);
			Quad4.Add((*DefiningPoints)[3]);

			TriangleEdge(&Quad1, Iteration + 1, MaxIterations, OUTVertexBuffer, OUTTriangleBuffer);
			TriangleEdge(&Quad2, Iteration + 1, MaxIterations, OUTVertexBuffer, OUTTriangleBuffer);
			TriangleEdge(&Quad3, Iteration + 1, MaxIterations, OUTVertexBuffer, OUTTriangleBuffer);
			TriangleEdge(&Quad4, Iteration + 1, MaxIterations, OUTVertexBuffer, OUTTriangleBuffer);

			return;
		}

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
	void SimulateTriangleEdge(const TArray<FVector>* DefiningPoints, const int32 Iteration, const int32 MaxIterations)
	{
		if (!DefiningPoints)
		{
			UE_LOG(LogTemp, Error, TEXT("DefiningPoints is nullptr in SimulateTriangleEdge at iteration %i"), Iteration);
			return;
		}
		if (DefiningPoints->Num() != 4)
		{
			UE_LOG(LogTemp, Error, TEXT("Wrong number of points given to SimulateTriangleEdge at iteration %i. Should be 4, are: %i"), Iteration, DefiningPoints->Num());
			return;
		}

		float HalfWidth  = (((*DefiningPoints)[1].X - (*DefiningPoints)[0].X) / 2.f) + (*DefiningPoints)[0].X;
		float HalfHeight = (((*DefiningPoints)[3].Y - (*DefiningPoints)[0].Y) / 2.f) + (*DefiningPoints)[0].Y;

		/* add defining points to DEM */
		for (FVector Vec : (*DefiningPoints))
		{
			// at this point we don't care if we overwrite already set DEM data, because they all get defaultet
			SetNewDEMPointData(Vec, FDEMData());
		}

		/* calculate new points */
		FVector E = FVector(HalfWidth, (*DefiningPoints)[0].Y, 0.f);
		FVector F = FVector((*DefiningPoints)[1].X, HalfHeight, 0.f);
		FVector G = FVector(HalfWidth, (*DefiningPoints)[2].Y, 0.f);
		FVector H = FVector((*DefiningPoints)[0].X, HalfHeight, 0.f);
		FVector I = FVector(HalfWidth, HalfHeight, 0.f);

		/* calculate DEM diagonal, used in calculation of Delta_BU*/
		if (Iteration == 0)
		{
			// TODO check if 0 is our first Iteration (so that we don't start at 1 and wonder why d_max is not set / still 0.f)
			d_max = FVector2D::Distance(Vec2Vec2D((*DefiningPoints)[3]), Vec2Vec2D((*DefiningPoints)[1]));
			//UE_LOG(LogTemp, Warning, TEXT("d_max is %f"), d_max);

			BottomLeftCorner = (*DefiningPoints)[0];
			BottomRightCorner = (*DefiningPoints)[1];
			TopRightCorner = (*DefiningPoints)[2];
			TopLeftCorner = (*DefiningPoints)[3];
		}

		/* add ascending points to hashmap */
		// E
		TArray<FVector2D> Values_E;
		Values_E.Add(Vec2Vec2D((*DefiningPoints)[0]));
		Values_E.Add(Vec2Vec2D((*DefiningPoints)[1]));
		AscendingPoints.Add(GetKeyForPoint(E), Values_E);
		// F
		TArray<FVector2D> Values_F;
		Values_F.Add(Vec2Vec2D((*DefiningPoints)[1]));
		Values_F.Add(Vec2Vec2D((*DefiningPoints)[2]));
		AscendingPoints.Add(GetKeyForPoint(F), Values_F);
		// G
		TArray<FVector2D> Values_G;
		Values_G.Add(Vec2Vec2D((*DefiningPoints)[2]));
		Values_G.Add(Vec2Vec2D((*DefiningPoints)[3]));
		AscendingPoints.Add(GetKeyForPoint(G), Values_G);
		// H
		TArray<FVector2D> Values_H;
		Values_H.Add(Vec2Vec2D((*DefiningPoints)[0]));
		Values_H.Add(Vec2Vec2D((*DefiningPoints)[3]));
		AscendingPoints.Add(GetKeyForPoint(H), Values_H);
		// I
		TArray<FVector2D> Values_I;
		Values_I.Add(Vec2Vec2D((*DefiningPoints)[0]));
		Values_I.Add(Vec2Vec2D((*DefiningPoints)[1]));
		Values_I.Add(Vec2Vec2D((*DefiningPoints)[2]));
		Values_I.Add(Vec2Vec2D((*DefiningPoints)[3]));
		AscendingPoints.Add(GetKeyForPoint(I), Values_I);


		if (Iteration == MaxIterations)
		{
			SetNewDEMPointData(E, FDEMData());
			SetNewDEMPointData(F, FDEMData());
			SetNewDEMPointData(G, FDEMData());
			SetNewDEMPointData(H, FDEMData());
			SetNewDEMPointData(I, FDEMData());
			return;
		}
		if (Iteration < MaxIterations)
		{
			// apply triangle edge algorithm on all newly created quads
			TArray<FVector> Quad1;
			Quad1.Add((*DefiningPoints)[0]);
			Quad1.Add(E);
			Quad1.Add(I);
			Quad1.Add(H);
			TArray<FVector> Quad2;
			Quad2.Add(E);
			Quad2.Add((*DefiningPoints)[1]);
			Quad2.Add(F);
			Quad2.Add(I);
			TArray<FVector> Quad3;
			Quad3.Add(I);
			Quad3.Add(F);
			Quad3.Add((*DefiningPoints)[2]);
			Quad3.Add(G);
			TArray<FVector> Quad4;
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
			SetNewDEMPointData(Constraint, FDEMData(Constraint.Z, EDEMState::DEM_KNOWN));
		}

		// FIFO Queue
		TQueue<FVector2D, EQueueMode::Spsc> FQ;
		// put all initial constraints in the FIFO Queue
		for (FVector Vec : *(InitialConstraints))
		{
			FQ.Enqueue(FVector2D(Vec.X, Vec.Y));
		}
		while (!FQ.IsEmpty())
		{
			FVector2D E;
			while (FQ.Dequeue(E))
			{
				// get all ascendents A of E
				TArray<FVector2D> A;
				GetAscendingPoints(E, A);
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
						ChildrenPoints.Add(GetKeyForPoint(a), E);
					}
				}
			}

			// care: meaning of A switched, now A is a single point, not an array

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
					float ChildElevation;
					if (!GetPointElevation(Child, ChildElevation))
					{
						// could not retrieve child point information from DEM data
						UE_LOG(LogTemp, Error, TEXT("Could not retrieve child point elevation data from DEM in Midpoint Displacement Bottom-Up. Child point was: %s"), *Child.ToString());
						continue;
					}
					e = e + Delta_BU(ChildElevation, FVector2D::Distance(A, Child));
					n++;
				}
				FDEMData NewData;
				NewData.Elevation = (e / n);
				NewData.State = EDEMState::DEM_KNOWN;
				SetNewDEMPointData(A, NewData);
				// remove A from ChildrenPoints map
				RemoveValuesFromChildrenPoints(A);
				// put A in FQ
				FQ.Enqueue(A);
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

};
