// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "TerrainTile.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "TerrainGeneratorWorker.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "HoverTestGameModeProceduralLevel.h"


// Sets default values
ATerrainManager::ATerrainManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();
	
	GameMode = Cast<AHoverTestGameModeProceduralLevel>(GetWorld()->GetAuthGameMode());

	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not cast game mode to AHoverTestGameModeProceduralLevel"));
	}

	// create threads
	FString ThreadName = "TerrainGeneratorWorkerThread";
	for (int i = 0; i < TerrainSettings.NumberOfThreadsToUse; ++i)
	{
		TerrainCreationQueue.Emplace();
		Threads.Add(
			FRunnableThread::Create(
				new TerrainGeneratorWorker(this, TerrainSettings, &TerrainCreationQueue[i]),
				*ThreadName,
				0,
				EThreadPriority::TPri_Normal,
				FPlatformAffinity::GetNoAffinityMask()
			)
		);
	}

	/**
	 * evil code that will make the game run in the editor, but not in a packaged game
	 */
	//CurrentTrackSector = FIntVector2D();
	//NextTrackSector = FIntVector2D();
	//TopLeftCorner = FIntVector2D();
	//TopRightCorner = FIntVector2D();
	//BottomLeftCorner = FIntVector2D();
	//BottomRightCorner = FIntVector2D();

}

void ATerrainManager::CalculateSectorsNeededAroundGivenLocation(const FVector Location, TArray<FIntVector2D>& OUTSectorsNeeded)
{
	FIntVector2D BaseSector = CalculateSectorFromLocation(Location);

	for (int x = -TerrainSettings.TilesToBeCreatedAroundActorRadius; x <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++x)
	{
		for (int y = -TerrainSettings.TilesToBeCreatedAroundActorRadius; y <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++y)
		{
			OUTSectorsNeeded.Add(FIntVector2D(BaseSector.X + x, BaseSector.Y + y));
		}
	}
	return;
}

void ATerrainManager::CalculateSectorsNeededAroundGivenSector(const FIntVector2D Sector, TArray<FIntVector2D>& OUTSectorsNeeded)
{
	for (int x = -TerrainSettings.TilesToBeCreatedAroundActorRadius; x <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++x)
	{
		for (int y = -TerrainSettings.TilesToBeCreatedAroundActorRadius; y <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++y)
		{
			OUTSectorsNeeded.Add(FIntVector2D(Sector.X + x, Sector.Y + y));
		}
	}

	return;
}

void ATerrainManager::CalculateTrackPath(const TArray<FIntVector2D> SectorsToCreate)
{
	TArray<FIntVector2D> SectorsToCreateTileFor;
	SectorsToCreateTileFor.Append(SectorsToCreate);

	while (SectorsToCreateTileFor.Num() > 0)
	{
		// check if NextTrackSector is contained in tiles that should be created
		if (!SectorsToCreateTileFor.Contains(NextTrackSector))
		{
			// all tiles will not have any tracks in them
			for (FIntVector2D Sector : SectorsToCreateTileFor)
			{
				if (!TrackMap.Contains(Sector))
				{
					TrackMap.Add(Sector, FSectorTrackInfo());
				}
			}

			SectorsToCreateTileFor.Empty();
			return;
		}

		// NextTrackSector is contained in SectorsToCreateTileFor -> remove it, calculate track for it, update
		if (SectorsToCreateTileFor.Remove(NextTrackSector) != 1)
		{
			UE_LOG(LogTemp, Error, TEXT("Removed none or more than one occurence of NextTrackSector in CalculateTrackPointsInSectors in %s"), *GetName());
		}

		if (!TrackMap.Contains(NextTrackSector))
		{
			FSectorTrackInfo TrackInfo = CalculateNewNextTrackSector();
			// !!! CurrentTrackSector now has the value of NextTrackSector !!!

			// PreviousTrackSector and FollowingTrackSector are used to calculate the entry and exit point

			FVector2D EntryPoint;
			FVector2D ExitPoint;

			CalculateEntryExitPoints(TrackInfo, EntryPoint, ExitPoint);
			TrackInfo.TrackEntryPoint = EntryPoint;
			TrackInfo.TrackExitPoint = ExitPoint;

			// calculate exit point elevation
			CalculateTrackExitPointElevation(CurrentTrackSector, TrackInfo, TrackInfo.TrackExitPointElevation);	
			CalculateBezierControlPoints(CurrentTrackSector, TrackInfo, TrackInfo.FirstBezierControlPoint, TrackInfo.SecondBezierControlPoint);

			TrackInfo.CheckpointID = NextAvailableCheckPointID;
			NextAvailableCheckPointID++;

			// calculate points on bezier curve
			CalculatePointsOnBezierCurve(CurrentTrackSector, TrackInfo, TrackInfo.PointsOnBezierCurve);

			// Calculate Y0 and Y1
			CalculateY0Y1(TrackInfo.PointsOnBezierCurve, TrackInfo.TrackExitPointElevation, TrackInfo.Y0Position, TrackInfo.Y1Position);

			// add to TrackMap
			TrackMap.Add(CurrentTrackSector, TrackInfo);
		}
		else
		{
			// this case should not happen
			UE_LOG(LogTemp, Error, TEXT("NextTrackSector was found in TrackMap, which means it was already calculated!"));
		}
		
	}

}

void ATerrainManager::GetAdjacentSectors(const FIntVector2D Sector, TArray<FIntVector2D>& OUTAdjacentSectors)
{
	for (int32 i = -1; i <= 1; ++i)
	{
		for (int32 j = -1; j <= 1; ++j)
		{
			if (i == 0 && j == 0) { continue; }
			OUTAdjacentSectors.Add(FIntVector2D(Sector.X + i, Sector.Y + j));
		}
	}
}

void ATerrainManager::GetRelevantAdjacentSectors(const FIntVector2D Sector, TArray<FIntVector2D>& OUTAdjacentSectors)
{
	// sector above
	OUTAdjacentSectors.Add(FIntVector2D(Sector.X, Sector.Y + 1));
	// sector below
	OUTAdjacentSectors.Add(FIntVector2D(Sector.X, Sector.Y - 1));
	// left sector
	OUTAdjacentSectors.Add(FIntVector2D(Sector.X - 1, Sector.Y));
	// right sector
	OUTAdjacentSectors.Add(FIntVector2D(Sector.X + 1, Sector.Y));
	return;
}

FSectorTrackInfo ATerrainManager::CalculateNewNextTrackSector()
{
	// get all possible next sectors
	TArray<FIntVector2D> PossibleSectors;

	FIntVector2D LeftDirection = NextTrackSector - FIntVector2D(0, 1);
	FIntVector2D UpDirection = NextTrackSector + FIntVector2D(1, 0);
	FIntVector2D RightDirection = NextTrackSector + FIntVector2D(0, 1);
	FIntVector2D DownDirection = NextTrackSector - FIntVector2D(1, 0);

	FSectorTrackInfo TrackInfo = FSectorTrackInfo();

	// check if this is the first track we create -> all directions possible
	if (NextTrackSector == FIntVector2D(0, 0) && CurrentTrackSector == FIntVector2D(0, 0))
	{
		PossibleSectors.Add(LeftDirection);
		PossibleSectors.Add(UpDirection);
		PossibleSectors.Add(RightDirection);
		PossibleSectors.Add(DownDirection);
	}
	else
	{

		// left of our current NextTrackSector
		if (CheckupSector(LeftDirection))
		{
			PossibleSectors.Add(LeftDirection);
		}
		// right
		if (CheckupSector(RightDirection))
		{
			PossibleSectors.Add(RightDirection);
		}
		// up
		if (CheckupSector(UpDirection))
		{
			PossibleSectors.Add(UpDirection);
		}
		// down
		if (CheckupSector(DownDirection))
		{
			PossibleSectors.Add(DownDirection);
		}
	}

	if (PossibleSectors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No possible NextTrackSector could be determined!"));
		return TrackInfo;
	}
	TrackInfo.bSectorHasTrack = true;

	// draw random direction
	int32 RandomVariable = FMath::RandRange(0, PossibleSectors.Num() - 1);
	
	// update previous track sector of NextTrackSector
	TrackInfo.PreviousTrackSector = CurrentTrackSector;

	CurrentTrackSector = NextTrackSector;

	// include CurrentTrackSector in the no-go quad
	AdjustQuad();

	NextTrackSector = PossibleSectors[RandomVariable];

	// update following sector of Current track sector (= previous Next track sector)
	TrackInfo.FollowingTrackSector = NextTrackSector;

	return TrackInfo;
}

bool ATerrainManager::CheckSectorWithinQuad(const FIntVector2D Sector)
{
	if (Sector.X <= TopLeftCorner.X && Sector.X >= BottomLeftCorner.X && Sector.Y >= TopLeftCorner.Y && Sector.Y <= TopRightCorner.Y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ATerrainManager::CheckupSector(const FIntVector2D Sector)
{
	if (!TrackMap.Contains(Sector))
	{
		// not yet calculated -> check if it lies within our no-go quad
		if (!CheckSectorWithinQuad(Sector))
		{
			return true;
		}
	}
	return false;
}

void ATerrainManager::CalculateEntryExitPoints(const FSectorTrackInfo TrackInfo, FVector2D & OUTEntryPoint, FVector2D & OUTExitPoint)
{
	// exit point
	if (TrackInfo.FollowingTrackSector == CurrentTrackSector + FIntVector2D(1, 0))
	{
		// Top
		OUTExitPoint = FVector2D(TerrainSettings.TileEdgeSize, TerrainSettings.TileEdgeSize / 2.f);
	}
	else if (TrackInfo.FollowingTrackSector == CurrentTrackSector + FIntVector2D(0, 1))
	{
		// Right
		OUTExitPoint = FVector2D(TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.TileEdgeSize);
	}
	else if (TrackInfo.FollowingTrackSector == CurrentTrackSector - FIntVector2D(1, 0))
	{
		// Bottom
		OUTExitPoint = FVector2D(0.f, TerrainSettings.TileEdgeSize / 2.f);
	}
	else if (TrackInfo.FollowingTrackSector == CurrentTrackSector - FIntVector2D(0, 1))
	{
		// Left
		OUTExitPoint = FVector2D(TerrainSettings.TileEdgeSize / 2.f, 0.f);
	}

	// special case: start of the game, where CurrentTrackSector == (0,0)
	if (CurrentTrackSector == FIntVector2D())
	{
		// entry point needs to be at the opposite direction of exit point
		OUTEntryPoint = FVector2D(TerrainSettings.TileEdgeSize, TerrainSettings.TileEdgeSize) - OUTExitPoint;
		return;
	}

	// entry point
	if (TrackInfo.PreviousTrackSector == CurrentTrackSector + FIntVector2D(1, 0))
	{
		// Top
		OUTEntryPoint = FVector2D(TerrainSettings.TileEdgeSize, TerrainSettings.TileEdgeSize / 2.f);
	}
	else if (TrackInfo.PreviousTrackSector == CurrentTrackSector + FIntVector2D(0, 1))
	{
		// Right
		OUTEntryPoint = FVector2D(TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.TileEdgeSize);
	}
	else if (TrackInfo.PreviousTrackSector == CurrentTrackSector - FIntVector2D(1, 0))
	{
		// Bottom
		OUTEntryPoint = FVector2D(0.f, TerrainSettings.TileEdgeSize / 2.f);
	}
	else if (TrackInfo.PreviousTrackSector == CurrentTrackSector - FIntVector2D(0, 1))
	{
		// Left
		OUTEntryPoint = FVector2D(TerrainSettings.TileEdgeSize / 2.f, 0.f);
	}

	return;
}

void ATerrainManager::AdjustQuad()
{
	if (CurrentTrackSector.X > TopLeftCorner.X)
	{
		TopLeftCorner.X = CurrentTrackSector.X;
		TopRightCorner.X = CurrentTrackSector.X;
	}

	if (CurrentTrackSector.X < BottomLeftCorner.X)
	{
		BottomLeftCorner.X = CurrentTrackSector.X;
		BottomRightCorner.X = CurrentTrackSector.X;
	}

	if (CurrentTrackSector.Y > TopRightCorner.Y)
	{
		TopRightCorner.Y = CurrentTrackSector.Y;
		BottomRightCorner.Y = CurrentTrackSector.Y;
	}

	if (CurrentTrackSector.Y < TopLeftCorner.Y)
	{
		TopLeftCorner.Y = CurrentTrackSector.Y;
		BottomLeftCorner.Y = CurrentTrackSector.Y;
	}

	return;

}

bool ATerrainManager::CalculateTrackExitPointElevation(const FIntVector2D Sector, const FSectorTrackInfo TrackInfo, float& OUTExitPointElevation)
{
	//UE_LOG(LogTemp, Error, TEXT("Track exit point elevations for sector %s"), *Sector.ToString());
	if (Sector == FIntVector2D(0, 0))
	{
		// very first sector, use default elevation for entry point height
		OUTExitPointElevation = TerrainSettings.TrackGenerationSettings.DefaultEntryPointHeight + (UMyStaticLibrary::GetNormalDistribution(TerrainSettings.TrackGenerationSettings.Steepness_Mean, TerrainSettings.TrackGenerationSettings.Steepness_Deviation, -1.f, 1.f) * TerrainSettings.TrackGenerationSettings.MaximumElevationDifference);
		//UE_LOG(LogTemp, Warning, TEXT("Exit point elevation is %f"), OUTExitPointElevation);
		//OUTExitPointElevation = TerrainSettings.TrackGenerationSettings.DefaultEntryPointHeight + FMath::RandRange(-TerrainSettings.TrackGenerationSettings.MaximumElevationDifference, TerrainSettings.TrackGenerationSettings.MaximumElevationDifference);
		return true;
	}
	const FSectorTrackInfo PreviousTrackInfo = TrackMap.FindRef(TrackInfo.PreviousTrackSector);
	if (!TrackMap.Contains(TrackInfo.PreviousTrackSector))
	{
		//UE_LOG(LogTemp, Error, TEXT("Could not find previous track sector %s in function CalculateTrackExitPointElevation for sector %s"), *TrackInfo.PreviousTrackSector.ToString(), *Sector.ToString());
		return false;
	}
	// calculate exit elevation: exit elevation <-- start elevation + Random[-MaximumElevationDifference, MaximumElevationDifference]
	OUTExitPointElevation = PreviousTrackInfo.TrackExitPointElevation + (UMyStaticLibrary::GetNormalDistribution(TerrainSettings.TrackGenerationSettings.Steepness_Mean, TerrainSettings.TrackGenerationSettings.Steepness_Deviation, -1.f, 1.f) * TerrainSettings.TrackGenerationSettings.MaximumElevationDifference);
	//UE_LOG(LogTemp, Warning, TEXT("Exit point elevation is %f"), OUTExitPointElevation);
	//OUTExitPointElevation = PreviousTrackInfo.TrackExitPointElevation + FMath::RandRange(-TerrainSettings.TrackGenerationSettings.MaximumElevationDifference, TerrainSettings.TrackGenerationSettings.MaximumElevationDifference);
	return true;

}

void ATerrainManager::CalculateBezierControlPoints(const FIntVector2D Sector, const FSectorTrackInfo TrackInfo, FVector & OUTControlPointOne, FVector & OUTControlPointTwo)
{
	if (Sector != FIntVector2D(0,0) && !TrackMap.Contains(TrackInfo.PreviousTrackSector))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find previous track sector of sector %s in TrackMap."), *Sector.ToString());
		return;
	}
	FSectorTrackInfo PreviousTrackInfo = TrackMap.FindRef(TrackInfo.PreviousTrackSector);
	const FVector2D MiddlePoint = FVector2D(TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.TileEdgeSize / 2.f);

	FVector ControlPoint1;
	FVector ControlPoint2;
	/**
	 * first control point is:
	 *		vector of second control point of previous sector to end point of previous sector (called 'Vector')
	 *		added to start point of current sector (called 'EntryPoint')
	 */
	if (Sector == FIntVector2D(0, 0))
	{
		// default control point one to tile middle with default entry point height in case we have no control point in a previous sector
		ControlPoint1 = FVector(MiddlePoint, TerrainSettings.TrackGenerationSettings.DefaultEntryPointHeight);
	}
	else
	{
		FVector Vector = FVector(PreviousTrackInfo.TrackExitPoint, PreviousTrackInfo.TrackExitPointElevation) - PreviousTrackInfo.SecondBezierControlPoint;
		FVector EntryPoint = FVector(TrackInfo.TrackEntryPoint, PreviousTrackInfo.TrackExitPointElevation);
		ControlPoint1 = EntryPoint + Vector;
	}

	//UE_LOG(LogTemp, Error, TEXT("Bezier control points for sector %s"), *Sector.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("First control point %s"), *ControlPoint1.ToString());

	/**
	 * calculation of control point 2 follows instructions given by Michael Franke in 'Dynamische Streckengenerierung und deren Einbettung in ein Terrain' in 2011
	 */
	//float RandomNumber = UMyStaticLibrary::GetNormalDistribution(TerrainSettings.TrackGenerationSettings.CURVINESS_MEAN, TerrainSettings.TrackGenerationSettings.Curviness, 0.f, 1.f);
	float RandomNumber = UMyStaticLibrary::GetNormalDistribution(TerrainSettings.TrackGenerationSettings.CURVINESS_MEAN, TerrainSettings.TrackGenerationSettings.CurvinessDisplacement, -0.5f, 0.5f);
	float Displacement = RandomNumber * (TerrainSettings.TileEdgeSize / 4.f);
	//UE_LOG(LogTemp, Warning, TEXT("Second control point displacement: %f"), Displacement);
	// vector from middle point to track exit point
	FVector2D LineEndPointMiddlePoint = TrackInfo.TrackExitPoint - MiddlePoint;
	// point halfway between middle point and track exit point
	FVector2D EndPointMiddlePointHalf = MiddlePoint + (LineEndPointMiddlePoint / 2.f);
	// control point before rotation is applied
	FVector2D IntermediatePoint = EndPointMiddlePointHalf + (LineEndPointMiddlePoint.GetSafeNormal() * Displacement);

	float RotationAngle = UMyStaticLibrary::GetNormalDistribution(0.f, TerrainSettings.TrackGenerationSettings.CurvinessRotation, -1.f, 1.f) * TerrainSettings.TrackGenerationSettings.MaximumRotationAngle;

	//UE_LOG(LogTemp, Warning, TEXT("Second control point rotation angle: %f"), RotationAngle);

	float Angle = RotationAngle * (UKismetMathLibrary::GetPI() / 180.f);	// convert to radians

	ControlPoint2.X = FMath::Cos(Angle) * (IntermediatePoint.X - TrackInfo.TrackExitPoint.X) -
		FMath::Sin(Angle) * (IntermediatePoint.Y - TrackInfo.TrackExitPoint.Y) + TrackInfo.TrackExitPoint.X;
	ControlPoint2.Y = FMath::Sin(Angle) * (IntermediatePoint.X - TrackInfo.TrackExitPoint.X) +
		FMath::Cos(Angle) * (IntermediatePoint.Y - TrackInfo.TrackExitPoint.Y) + TrackInfo.TrackExitPoint.Y;

	float AverageElevation = PreviousTrackInfo.TrackExitPointElevation + (TrackInfo.TrackExitPointElevation - PreviousTrackInfo.TrackExitPointElevation) / 2.f;
	ControlPoint2.Z = UMyStaticLibrary::GetNormalDistribution(AverageElevation, TerrainSettings.TrackGenerationSettings.Hilliness);

	//UE_LOG(LogTemp, Warning, TEXT("Second control point %s"), *ControlPoint2.ToString());

	OUTControlPointOne = ControlPoint1;
	OUTControlPointTwo = ControlPoint2;

}

void ATerrainManager::CalculatePointsOnBezierCurve(const FIntVector2D Sector, const FSectorTrackInfo TrackInfo, TArray<FVector>& OUTPointsOnBezierCurve)
{
	FSectorTrackInfo PreviousTrackInfo = TrackMap.FindRef(TrackInfo.PreviousTrackSector);

	FVector BezierPoints[4];

	if (Sector == FIntVector2D(0, 0))
	{
		BezierPoints[0] = FVector(TrackInfo.TrackEntryPoint, TerrainSettings.TrackGenerationSettings.DefaultEntryPointHeight);
	}
	else
	{
		BezierPoints[0] = FVector(TrackInfo.TrackEntryPoint, PreviousTrackInfo.TrackExitPointElevation);
	}

	BezierPoints[1] = TrackInfo.FirstBezierControlPoint;
	BezierPoints[2] = TrackInfo.SecondBezierControlPoint;
	BezierPoints[3] = FVector(TrackInfo.TrackExitPoint, TrackInfo.TrackExitPointElevation);


	FVector::EvaluateBezier(BezierPoints, TerrainSettings.TrackGenerationSettings.TrackResolution, OUTPointsOnBezierCurve);
}

void ATerrainManager::CalculateY0Y1(const TArray<FVector>& PointsOnTrack, const float ExitPointElevation, FVector& OUTY0, FVector& OUTY1)
{
	// calculate last track segment
	/**
	 *			Last Track Segment:	(border is at top, track middle line goes from W0 to X0, Z0Y0 is left track border, Z1Y1 is right track border)
	 *
	 *			Y0--------------------X0--------------------Y1
	 *			|					  |						|
	 *			|					  | 					|
	 *			|					  | 					|
	 *			H0					  |						H1
	 *			|					  |						|
	 *			|					  |						|
	 *			Z0--------------------W0--------------------Z1
	 *
	 * H0 and H1 are helper points that lie on the respective line (Z0Y0 or Z1Y1)
	 */
	FVector W0 = PointsOnTrack[PointsOnTrack.Num() - 2];
	FVector X0 = PointsOnTrack[PointsOnTrack.Num() - 1];
	FVector W0X0 = X0 - W0;
	FVector Normal = FVector::CrossProduct(W0X0, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

	FVector Z0 = W0 + (TerrainSettings.TrackGenerationSettings.TrackWidth / 2.f) * Normal;
	FVector Z1 = W0 + (-TerrainSettings.TrackGenerationSettings.TrackWidth / 2.f) * Normal;

	FVector H0 = Z0 + W0X0.GetSafeNormal();
	FVector H1 = Z1 + W0X0.GetSafeNormal();

	// calculate intersection point of lines Z0H0 / Z1H1 and tile border with Points PointA and PointB
	FVector2D PointA;
	FVector2D PointB;
	// guess tile border
	float TileSize = TerrainSettings.TileEdgeSize;
	switch (UMyStaticLibrary::GuessTileBorder(X0, TileSize))
	{
		case ETileBorder::ETB_Bottom:
			PointA = FVector2D(0.f, 0.f);
			PointB = FVector2D(0.f, TileSize);
			break;
		case ETileBorder::ETB_Top:
			PointA = FVector2D(TileSize, 0.f);
			PointB = FVector2D(TileSize, TileSize);
			break;
		case ETileBorder::ETB_Left:
			PointA = FVector2D(0.f, 0.f);
			PointB = FVector2D(TileSize, 0.f);
			break;
		case ETileBorder::ETB_Right:
			PointA = FVector2D(0.f, TileSize);
			PointB = FVector2D(TileSize, TileSize);
			break;
	}
	FVector2D IntersectionZ0H0;
	FVector2D IntersectionZ1H1;
	UMyStaticLibrary::CalculateIntersectionPoint(FVector2D(Z0.X, Z0.Y), FVector2D(H0.X, H0.Y), PointA, PointB, IntersectionZ0H0);
	UMyStaticLibrary::CalculateIntersectionPoint(FVector2D(Z1.X, Z1.Y), FVector2D(H1.X, H1.Y), PointA, PointB, IntersectionZ1H1);
	OUTY0 = FVector(IntersectionZ0H0, ExitPointElevation);
	OUTY1 = FVector(IntersectionZ1H1, ExitPointElevation);
	return;
}

// Called every frame
void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// check if free tiles can be deleted
	for (int i = FreeTiles.Num() - 1; i >= 0; --i)
	{
		if (FreeTiles[i]->GetTileStatus() == ETileStatus::TILE_FREE)
		{
			if (GetWorld()->TimeSeconds - FreeTiles[i]->GetTimeSinceTileFreed() >= TerrainSettings.SecondsUntilFreeTileGetsDeleted)
			{
				// delete the tile
				ATerrainTile* Tile = FreeTiles.Pop();
				Tile->Destroy();
			}
		}
	}

	// check if we need to create mesh data
	for (int i = 0; i < TerrainSettings.NumberOfThreadsToUse; ++i)
	{
		FTerrainJob Job;
		// TODO check if we need a limit on mesh data (Job) memory usage
		if (PendingTerrainJobQueue.Dequeue(Job))
		{
			if (!bHasTileBeenAddedToQueue)
			{
				bHasTileBeenAddedToQueue = true;
			}
			TilesInProcessCounter++;
			TerrainCreationQueue[i].Enqueue(Job);
		}
	}

	// check if we need to update mesh data
	for (int i = 0; i < TerrainSettings.MeshUpdatesPerFrame; ++i)
	{
		FTerrainJob Job;
		if (FinishedJobQueue.Dequeue(Job))
		{
			TilesInProcessCounter--;
			if (Job.TerrainTile == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("TerrainTile pointer in Job is nullptr!"));
			}
			else
			{
				if (bShouldCheckSectorsNeedCoverageForReset)
				{
					SectorsNeedCoverageForReset.Remove(Job.TerrainTile->GetCurrentSector());
				}
				SectorsCurrentlyProcessed.AddUnique(Job.TerrainTile->GetCurrentSector());
				/* check if we need to recalculate the tile 'behind' our track start point to match the border elevations */
				if (!bRecalculatedTileBehindStartPoint && Job.TerrainTile->GetCurrentSector() == FIntVector2D(0, 0))
				{
					bRecalculatedTileBehindStartPoint = true;
					FSectorTrackInfo TrackInfo = TrackMap.FindRef(FIntVector2D(0, 0));

					// calculate which sector lies 'behind' track entry point
					FVector2D EntryPoint = TrackInfo.TrackEntryPoint;
					float EdgeSize = TerrainSettings.TileEdgeSize;
					// entry point is at the top?
					if (EntryPoint.X > 0.75 * EdgeSize)
					{
						RecalculateTileForSector(FIntVector2D(1, 0));
					}
					// entry point at bottom?
					else if (EntryPoint.X < 0.25 * EdgeSize)
					{
						RecalculateTileForSector(FIntVector2D(-1, 0));
					}
					// entry point left?
					else if (EntryPoint.Y < 0.25 * EdgeSize)
					{
						RecalculateTileForSector(FIntVector2D(0, -1));
					}
					// entry point right?
					else if (EntryPoint.Y > 0.75 * EdgeSize)
					{
						RecalculateTileForSector(FIntVector2D(0, 1));
					}
				}
				Job.TerrainTile->UpdateMeshData(TerrainSettings, Job.MeshData);
			}
		}
	}

	// check if we need to spawn checkpoints
	while (!PendingCheckpointSpawnQueue.IsEmpty())
	{
		FCheckpointSpawnJob SpawnJob;
		if (PendingCheckpointSpawnQueue.Dequeue(SpawnJob))
		{
			UWorld* World = GetWorld();
			if (World)
			{
				AActor* Actor = World->SpawnActor(CheckpointClassToSpawn, &SpawnJob.CheckpointTransform, FActorSpawnParameters());
				AProceduralCheckpoint* Checkpoint = Cast<AProceduralCheckpoint>(Actor);
				if (Checkpoint)
				{
					Checkpoint->SetCheckpointID(SpawnJob.CheckpointID);
					Checkpoint->SetActorScale3D(FVector(1.f, TerrainSettings.TrackGenerationSettings.TrackWidth / 100.f, 1.f));
					// search tile that is responsible for the sector and add checkpoint reference
					for (ATerrainTile* Tile : TilesInUse)
					{
						if (Tile->GetCurrentSector() == SpawnJob.CheckpointSector)
						{
							Tile->SetCheckpointReference(Checkpoint);
							break;
						}
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Spawned Checkpoint could not be cast to AProceduralCheckpoint in %s"), *GetName());
				}
			}
		}
	}

	// check if we can spawn the player
	if (bHasTileBeenAddedToQueue && TilesInProcessCounter == 0 && RemainingPlayersToSpawn > 0)
	{
		RemainingPlayersToSpawn--;
		// notify gamemode to spawn player
		if (GameMode)
		{
			GameMode->SpawnPlayerFromTerrainManager(TerrainSettings.TransitionElevationOffset, TerrainSettings.TransitionInterpolationSpeed, TerrainSettings.TransitionDeltaToStop);
		}
	}

	if (bShouldCheckSectorsNeedCoverageForReset)
	{
		if (SectorsNeedCoverageForReset.Num() == 0)
		{
			// notify game mode
			UWorld* World = GetWorld();
			if (World)
			{
				AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(World->GetAuthGameMode());
				if (GameMode)
				{
					GameMode->AllowDefaultPawnToTransitionToEndLocation();
					bShouldCheckSectorsNeedCoverageForReset = false;
					// just in case
					SectorsNeedCoverageForReset.Empty();
				}
			}
		}
	}
}

void ATerrainManager::CreateAndInitializeTiles(int32 NumberOfTilesToCreate)
{
	for (int i = 0; i < NumberOfTilesToCreate; ++i)
	{
		ATerrainTile* Tile = GetWorld()->SpawnActor<ATerrainTile>(FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FActorSpawnParameters());
		// at first spawn them all at the same location
		if (Tile == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnActor return pointer is nullptr"));
		}
		else
		{
			Tile->SetupTile(TerrainSettings, FIntVector2D(0, 0));
			FreeTiles.Add(Tile);
		}

	}
}

void ATerrainManager::AddActorToTrack(AActor * ActorToTrack)
{
	if (ActorToTrack == nullptr) { return; }
	TrackedActors.Add(ActorToTrack);

	if (bGenerateTerrainOnActorRegister)
	{
		// calculate the sectors the actor needs covered
		TArray<FIntVector2D> SectorsThatNeedCoverage;
		CalculateSectorsNeededAroundGivenLocation(ActorToTrack->GetActorLocation(), SectorsThatNeedCoverage);

		// iterate all existing tiles and check if they already cover the sectors the actor needs covered
		for (ATerrainTile* Tile : TilesInUse)
		{
			int32 RemovedItems = SectorsThatNeedCoverage.Remove(Tile->GetCurrentSector());
			if (RemovedItems > 0)
			{
				Tile->AddAssociatedActor();
			}
		}

		CalculateTrackPath(SectorsThatNeedCoverage);

		// use free tiles to cover sectors
		while (SectorsThatNeedCoverage.Num() > 0 && FreeTiles.Num() > 0)
		{
			FIntVector2D Sector = SectorsThatNeedCoverage.Pop();
			ATerrainTile* Tile = FreeTiles.Pop();
			Tile->UpdateTilePosition(TerrainSettings, Sector);
			Tile->AddAssociatedActor();
			FTerrainJob Job;
			Job.TerrainTile = Tile;
			PendingTerrainJobQueue.Enqueue(Job);

			TilesInUse.Add(Tile);
		}

		// check if we need to create additional tiles to cover the sectors
		if (SectorsThatNeedCoverage.Num() > 0)
		{
			CreateAndInitializeTiles(SectorsThatNeedCoverage.Num());
			// should be exactly the number of tiles we need, but better safe than sorry
			while (SectorsThatNeedCoverage.Num() > 0 && FreeTiles.Num() > 0)
			{
				FIntVector2D Sector = SectorsThatNeedCoverage.Pop();
				ATerrainTile* Tile = FreeTiles.Pop();

				Tile->UpdateTilePosition(TerrainSettings, Sector);
				Tile->AddAssociatedActor();
				FTerrainJob Job;
				Job.TerrainTile = Tile;
				PendingTerrainJobQueue.Enqueue(Job);

				TilesInUse.Add(Tile);
			}

			if (SectorsThatNeedCoverage.Num() != 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Something went wrong in %s, not all sectors could get assigned a tile to"), *GetName());
			}
			// check if something went wrong and we created too many free tiles
			if (FreeTiles.Num() != 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Something went wrong in %s, too many free tiles were created to cover sectors"), *GetName());
			}
		}
	}
}

void ATerrainManager::BuildTerrainAroundSector(const FIntVector2D Sector)
{
	// calculate the sectors around the given sector that need coverage
	TArray<FIntVector2D> SectorsThatNeedCoverage;
	CalculateSectorsNeededAroundGivenSector(Sector, SectorsThatNeedCoverage);

	// check if some sectors are already covered
	for (const ATerrainTile* Tile : TilesInUse)
	{
		SectorsThatNeedCoverage.Remove(Tile->GetCurrentSector());
	}

	CalculateTrackPath(SectorsThatNeedCoverage);

	// use free tiles to cover sectors
	while (SectorsThatNeedCoverage.Num() > 0 && FreeTiles.Num() > 0)
	{
		FIntVector2D Sec = SectorsThatNeedCoverage.Pop();
		ATerrainTile* Tile = FreeTiles.Pop();

		Tile->UpdateTilePosition(TerrainSettings, Sec);
		// don't increase associated actor count

		FTerrainJob Job;
		Job.TerrainTile = Tile;
		PendingTerrainJobQueue.Enqueue(Job);

		TilesInUse.Add(Tile);
	}

	// check if we need to create additional tiles to cover the sectors
	if (SectorsThatNeedCoverage.Num() > 0)
	{
		CreateAndInitializeTiles(SectorsThatNeedCoverage.Num());
		while (SectorsThatNeedCoverage.Num() > 0 && FreeTiles.Num() > 0)
		{
			FIntVector2D Sec = SectorsThatNeedCoverage.Pop();
			ATerrainTile* Tile = FreeTiles.Pop();

			Tile->UpdateTilePosition(TerrainSettings, Sec);
			// don't increase associated actor count

			FTerrainJob Job;
			Job.TerrainTile = Tile;
			PendingTerrainJobQueue.Enqueue(Job);

			TilesInUse.Add(Tile);
		}

		if (SectorsThatNeedCoverage.Num() != 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Something went wrong in %s, not all sectors could get assigned a tile to"), *GetName());
		}
		// check if something went wrong and we created too many free tiles
		if (FreeTiles.Num() != 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Something went wrong in %s, too many free tiles were created to cover sectors"), *GetName());
		}
	}
}

void ATerrainManager::RemoveTrackedActor(AActor * ActorToRemove)
{
	if (ActorToRemove == nullptr) { return; }
	int32 NumberOfRemovedActors = TrackedActors.RemoveSingle(ActorToRemove);

	if (NumberOfRemovedActors != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not remove specified Actor in RemoveTrackedActor in %s"), *GetName());
	}

	// free Tiles associated with this actor
	TArray<FIntVector2D> Sectors;
	CalculateSectorsNeededAroundGivenLocation(ActorToRemove->GetActorLocation(), Sectors);
	TArray<ATerrainTile*> TilesToFree;

	for (ATerrainTile* Tile : TilesInUse)
	{
		if (Sectors.Remove(Tile->GetCurrentSector()) > 0)
		{
			if (Tile->RemoveAssociatedActor() <= 0)
			{
				SectorsCurrentlyProcessed.Remove(Tile->GetCurrentSector());
				Tile->FreeTile();
				TilesToFree.Add(Tile);
			}
		}
	}

	for (ATerrainTile* Tile : TilesToFree)
	{
		TilesInUse.Remove(Tile);
		FreeTiles.Add(Tile);
	}

	TilesToFree.Empty();
}

FIntVector2D ATerrainManager::CalculateSectorFromLocation(FVector CurrentWorldLocation)
{
	float XSector = 0.f;
	float YSector = 0.f;
	/*if (TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize != 0)
	{
		XSector = CurrentWorldLocation.X / (TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize);
		if (XSector < 0)
		{
			XSector--;
		}
	}
	if (TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize != 0)
	{
		YSector = CurrentWorldLocation.Y / (TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize);
		if (YSector < 0)
		{
			YSector--;
		}
	}*/
	if (TerrainSettings.TileEdgeSize != 0)
	{
		XSector = CurrentWorldLocation.X / TerrainSettings.TileEdgeSize;
		if (XSector < 0)
		{
			XSector--;
		}
		YSector = CurrentWorldLocation.Y / TerrainSettings.TileEdgeSize;
		if (YSector < 0)
		{
			YSector--;
		}
	}
	return FIntVector2D(static_cast<int32>(XSector), static_cast<int32>(YSector));
}

void ATerrainManager::BeginDestroy()
{
	for (auto& Thread : Threads)
	{
		if (Thread != nullptr)
		{
			Thread->Kill();
		}
	}
	Super::BeginDestroy();
}

void ATerrainManager::HandleTrackedActorChangedSector(AActor * TrackedActor, FIntVector2D PreviousSector, FIntVector2D NewSector)
{
	// my rhymes are lit

	// identify sectors that are no longer needed by this actor
	TArray<FIntVector2D> SectorsNeededAtNewPosition;
	TArray<FIntVector2D> SectorsNeededAtPreviousPosition; 

	CalculateSectorsNeededAroundGivenSector(NewSector, SectorsNeededAtNewPosition);
	CalculateSectorsNeededAroundGivenSector(PreviousSector, SectorsNeededAtPreviousPosition);

	for (const FIntVector2D Vec : SectorsNeededAtNewPosition)
	{
		SectorsNeededAtPreviousPosition.Remove(Vec);
	}

	TArray<ATerrainTile*> TilesToFree;
	// identify tiles that cover these sectors
	for (ATerrainTile* Tile : TilesInUse)
	{
		if (SectorsNeededAtPreviousPosition.Remove(Tile->GetCurrentSector()) > 0)
		{
			// if these tiles aren't needed by any other actor aswell -> free them
			if (Tile->RemoveAssociatedActor() <= 0)
			{
				SectorsCurrentlyProcessed.Remove(Tile->GetCurrentSector());
				Tile->FreeTile();
				TilesToFree.Add(Tile);
			}
		}
	}

	for (ATerrainTile* Tile : TilesToFree)
	{
		TilesInUse.Remove(Tile);
		FreeTiles.Add(Tile);
	}

	SectorsNeededAtNewPosition.Empty();
	SectorsNeededAtPreviousPosition.Empty();
	TilesToFree.Empty();

	// identify new sectors needed for the new actor location
	CalculateSectorsNeededAroundGivenSector(NewSector, SectorsNeededAtNewPosition);
	CalculateSectorsNeededAroundGivenSector(PreviousSector, SectorsNeededAtPreviousPosition);

	for (const FIntVector2D Sector : SectorsNeededAtPreviousPosition)
	{
		SectorsNeededAtNewPosition.Remove(Sector);
	}

	// check all tiles in use if they already cover the needed sectors
	// identify sectors that need new tiles (i.e. that are uncovered by existing tiles)
	for (ATerrainTile* Tile : TilesInUse)
	{
		int32 RemovedSectors = SectorsNeededAtNewPosition.Remove(Tile->GetCurrentSector());
		if (RemovedSectors > 0)
		{
			Tile->AddAssociatedActor();
		}
	}

	// check how many free tiles are available
	// (if necessary: create new tiles if free tiles do not suffice to cover all newly needed sectors)
	if (SectorsNeededAtNewPosition.Num() > FreeTiles.Num())
	{
		CreateAndInitializeTiles(SectorsNeededAtNewPosition.Num() - FreeTiles.Num());
	}

	CalculateTrackPath(SectorsNeededAtNewPosition);

	// update free tiles to cover new sectors and increase associatedactors count
	while (SectorsNeededAtNewPosition.Num() > 0 && FreeTiles.Num() > 0)
	{
		FIntVector2D Sector = SectorsNeededAtNewPosition.Pop();
		ATerrainTile* Tile = FreeTiles.Pop();

		Tile->UpdateTilePosition(TerrainSettings, Sector);
		Tile->AddAssociatedActor();
		FTerrainJob Job;
		Job.TerrainTile = Tile;
		PendingTerrainJobQueue.Enqueue(Job);

		TilesInUse.Add(Tile);
	}
}

void ATerrainManager::GetAdjacentTiles(const FIntVector2D Sector, TArray<ATerrainTile*>& OUTAdjacentTiles, const bool OnlyReturnRelevantTiles)
{
	TArray<FIntVector2D> AdjacentSectors;
	if (OnlyReturnRelevantTiles)
	{
		GetRelevantAdjacentSectors(Sector, AdjacentSectors);
	}
	else
	{
		GetAdjacentSectors(Sector, AdjacentSectors);
	}

	for (ATerrainTile* Tile : TilesInUse)
	{
		if (AdjacentSectors.Contains(Tile->GetCurrentSector()))
		{
			OUTAdjacentTiles.Add(Tile);
		}
	}
}

int32 ATerrainManager::GetTrackPointsForSector(const FIntVector2D Sector, FVector & OUTTrackEntryPoint, FVector & OUTTrackExitPoint)
{
	FSectorTrackInfo TrackInfo = TrackMap.FindRef(Sector);

	if (!TrackMap.Contains(Sector))
	{
		UE_LOG(LogTemp, Warning, TEXT("Sector %s not yet processed"), *Sector.ToString());
		return -1;
	}
	else
	{
		if (!TrackInfo.bSectorHasTrack)
		{
			return 0;
		}
		else
		{
			// get elevation of previous sector's track exit point
			FSectorTrackInfo PreviousTrackInfo = TrackMap.FindRef(TrackInfo.PreviousTrackSector);
			if (!TrackMap.Contains(TrackInfo.PreviousTrackSector))
			{
				// TODO check if that makes sense
				UE_LOG(LogTemp, Error, TEXT("Could not find previous track sector in GetTrackPointsForSector for sector %s"), *Sector.ToString());
				return -1;
			}

			if (Sector == FIntVector2D(0, 0))
			{
				OUTTrackEntryPoint = FVector(TrackInfo.TrackEntryPoint, TerrainSettings.TrackGenerationSettings.DefaultEntryPointHeight);
			}
			else
			{
				OUTTrackEntryPoint = FVector(TrackInfo.TrackEntryPoint, PreviousTrackInfo.TrackExitPointElevation);
			}
			OUTTrackExitPoint = FVector(TrackInfo.TrackExitPoint, TrackInfo.TrackExitPointElevation);
			return 1;
		}
	}
}

void ATerrainManager::GenerateTrackMesh(const FIntVector2D Sector, TArray<FRuntimeMeshVertexSimple>& OUTVertexBuffer, TArray<int32>& OUTTriangleBuffer, TArray<FTrackSegment>& OUTTrackSegments)
{
	if (!TrackMap.Contains(Sector))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find track info for sector %s in TrackMap."), *Sector.ToString());
		return;
	}

	FSectorTrackInfo TrackInfo = TrackMap.FindRef(Sector);
	FSectorTrackInfo PreviousTrackInfo = TrackMap.FindRef(TrackInfo.PreviousTrackSector);

	//float HalfHeight = TrackInfo.TrackExitPointElevation - ((TrackInfo.TrackExitPointElevation - PreviousTrackInfo.TrackExitPointElevation) / 2.f);
		//EndPoint.Z - ((EndPoint.Z - StartPoint.Z) / 2.f);


	/*FVector BezierPoints[4]; 
	BezierPoints[1] = TrackInfo.FirstBezierControlPoint;
	BezierPoints[2] = TrackInfo.SecondBezierControlPoint;

	GetTrackPointsForSector(Sector, BezierPoints[0], BezierPoints[3]);*/

	// calculate mesh
	TArray<FVector> PointsOnTrack = TrackInfo.PointsOnBezierCurve;
	//FVector::EvaluateBezier(BezierPoints, TerrainSettings.TrackGenerationSettings.TrackResolution, PointsOnTrack);
#
	// check if we should spawn a checkpoint
	if (TrackInfo.CheckpointID != -1 && PointsOnTrack.Num() >= 3)
	{
		FVector SpawnLocation;
		FVector SpawnDirection;
		if (Sector == FIntVector2D(0, 0))
		{
			SpawnLocation = FVector(Sector.X * TerrainSettings.TileEdgeSize + PointsOnTrack[1].X, Sector.Y * TerrainSettings.TileEdgeSize + PointsOnTrack[1].Y, PointsOnTrack[1].Z);
			SpawnDirection = PointsOnTrack[2] - PointsOnTrack[1];
		}
		else
		{
			SpawnLocation = FVector(Sector.X * TerrainSettings.TileEdgeSize + PointsOnTrack[0].X, Sector.Y * TerrainSettings.TileEdgeSize + PointsOnTrack[0].Y, PointsOnTrack[0].Z);
			SpawnDirection = PointsOnTrack[1] - PointsOnTrack[0];
		}
		FQuat SpawnRotation = SpawnDirection.Rotation().Quaternion();
		FVector SpawnScaling = FVector(1.f, 1.f, 1.f);
		FTransform Transform;
		Transform.SetLocation(SpawnLocation);
		Transform.SetRotation(SpawnRotation);
		Transform.SetScale3D(SpawnScaling);

		// actors need to be spawned in the game thread
		PendingCheckpointSpawnQueue.Enqueue(FCheckpointSpawnJob(TrackInfo.CheckpointID, Transform, Sector));
	}

	// check if we can set the player spawn point
	if (Sector == FIntVector2D(0, 0))
	{
		if (PointsOnTrack.Num() > 3 && TerrainSettings.TrackSegmentToSpawnPlayerAt < (PointsOnTrack.Num() -1))
		{
			FVector SpawnLocation = PointsOnTrack[TerrainSettings.TrackSegmentToSpawnPlayerAt];
			FVector SpawnDirection = PointsOnTrack[TerrainSettings.TrackSegmentToSpawnPlayerAt + 1] - PointsOnTrack[TerrainSettings.TrackSegmentToSpawnPlayerAt];
			SpawnLocation += (UKismetMathLibrary::GetForwardVector(SpawnDirection.Rotation()) * TerrainSettings.TrackSegmentToSpawnPlayerAtOffset);
			SpawnLocation.Z += TerrainSettings.PlayerSpawnElevationOffset;
			FQuat SpawnRotation = SpawnDirection.Rotation().Quaternion();
			FVector SpawnScaling = FVector(1.f, 1.f, 1.f);

			FTransform Transform;
			Transform.SetLocation(SpawnLocation);
			Transform.SetRotation(SpawnRotation);
			Transform.SetScale3D(SpawnScaling);

			if (GameMode)
			{
				GameMode->SetPlayerSpawn(Transform);
			}
		}
	}

	for (int32 i = 0; i < PointsOnTrack.Num(); ++i)
	{
		// calculate normal

		/**
		 * X1X0 is the vector from the current track mid point to the next track mid point
		 * Normal is the normal of X1X0 and (0,0,1)
		 * Y0 and Y1 are the track border points (left and right) of the current track mid point
		 */
		FVector Y0;
		FVector Y1;
		FVector Normal;
		if (i == 0)
		{
			int32 TileSize = TerrainSettings.TileEdgeSize;
			FVector Pt = PointsOnTrack[i];		// shorter writing

			// calculate bezier points when calculating control points

			if (Sector == FIntVector2D(0, 0))
			{
				switch (UMyStaticLibrary::GuessTileBorder(Pt, TileSize))
				{
					case ETileBorder::ETB_Bottom:
						Normal = FVector::CrossProduct(FVector(1, 0, 0), FVector(0, 0, 1)).GetSafeNormal();
						break;
					case ETileBorder::ETB_Right:
						Normal = FVector::CrossProduct(FVector(0, -1, 0), FVector(0, 0, 1)).GetSafeNormal();
						break;
					case ETileBorder::ETB_Top:
						Normal = FVector::CrossProduct(FVector(-1, 0, 0), FVector(0, 0, 1)).GetSafeNormal();
						break;
					case ETileBorder::ETB_Left:
						Normal = FVector::CrossProduct(FVector(0, 1, 0), FVector(0, 0, 1)).GetSafeNormal();
						break;
					case ETileBorder::ETB_Invalid:
						UE_LOG(LogTemp, Error, TEXT("Could not guess on which border the first point on the bézier curve lies."));
						break;
				}

				Y0 = PointsOnTrack[i] + (TerrainSettings.TrackGenerationSettings.TrackWidth / 2.f) * Normal;
				Y1 = PointsOnTrack[i] + (-TerrainSettings.TrackGenerationSettings.TrackWidth / 2.f) * Normal;

			}
			else
			{
				// load intersection points from TrackInfo (Y0 and Y1 values from previous tile)
				Y0 = PreviousTrackInfo.Y0Position;
				Y1 = PreviousTrackInfo.Y1Position;

				// 'guess' on what border of the tile the entry point lies
				switch (UMyStaticLibrary::GuessTileBorder(Pt, TileSize))
				{
					case ETileBorder::ETB_Bottom:
						Y0.X = 0.f;
						Y1.X = 0.f;
						break;
					case ETileBorder::ETB_Right:
						Y0.Y = TileSize;
						Y1.Y = TileSize;
						break;
					case ETileBorder::ETB_Top:
						Y0.X = TileSize;
						Y1.X = TileSize;
						break;
					case ETileBorder::ETB_Left:
						Y0.Y = 0.f;
						Y1.Y = 0.f;
						break;
					case ETileBorder::ETB_Invalid:
						UE_LOG(LogTemp, Error, TEXT("Could not guess on which border the first point on the bézier curve lies."));
						break;
				}
			}
		}
		else if (i == PointsOnTrack.Num() - 1)
		{
			Y0 = TrackInfo.Y0Position;
			Y1 = TrackInfo.Y1Position;
		}
		else
		{
			FVector X1X0 = PointsOnTrack[i + 1] - PointsOnTrack[i];
			Normal = FVector::CrossProduct(X1X0, FVector(0, 0, 1)).GetSafeNormal();

			Y0 = PointsOnTrack[i] + (TerrainSettings.TrackGenerationSettings.TrackWidth / 2.f) * Normal;
			Y1 = PointsOnTrack[i] + (-TerrainSettings.TrackGenerationSettings.TrackWidth / 2.f) * Normal;
		}


		OUTVertexBuffer.Add(CreateRuntimeMeshVertexSimple(PointsOnTrack[i], FVector(0, 0, 1)));
		OUTVertexBuffer.Add(CreateRuntimeMeshVertexSimple(Y0, FVector(0, 0, 1)));
		OUTVertexBuffer.Add(CreateRuntimeMeshVertexSimple(Y1, FVector(0, 0, 1)));

		// create triangles
		// can only create triangles if we have at least two midpoints calculated
		if (i > 0)
		{
			/**
			 * track segments looks like this: (viewing from point X1)
			 *
			 *			Y2----------X1----------Y3
			 *			|			|			|
			 *			|			|			|
			 *			|			|			|
			 *			Y0----------X0----------Y1
			 *
			 *
			 * points:	Y3 = Num - 1
			 *			Y2 = Num - 2
			 *			X1 = Num - 3
			 *			Y1 = Num - 4
			 *			Y0 = Num - 5
			 *			X0 = Num - 6
			 *
			 *	(note that order of Y2 and Y3 (and Y0 and Y1) may be reversed due to normal calculation (but since its reveresed for all points, it doesn't matter)
			 */
			int32 Num = OUTVertexBuffer.Num();
			// triangle Y0 X0 Y2
			OUTTriangleBuffer.Add(Num - 5);
			OUTTriangleBuffer.Add(Num - 6);
			OUTTriangleBuffer.Add(Num - 2);

			// triangle X0 X1 Y2
			OUTTriangleBuffer.Add(Num - 6);
			OUTTriangleBuffer.Add(Num - 3);
			OUTTriangleBuffer.Add(Num - 2);

			// triangle X0 Y1 X1
			OUTTriangleBuffer.Add(Num - 6);
			OUTTriangleBuffer.Add(Num - 4);
			OUTTriangleBuffer.Add(Num - 3);

			// triangle Y1 Y3 X1
			OUTTriangleBuffer.Add(Num - 4);
			OUTTriangleBuffer.Add(Num - 1);
			OUTTriangleBuffer.Add(Num - 3);

			OUTTrackSegments.Add(FTrackSegment(OUTVertexBuffer[Num - 5].Position, OUTVertexBuffer[Num - 4].Position, OUTVertexBuffer[Num - 1].Position, OUTVertexBuffer[Num - 2].Position, TerrainSettings.TileEdgeSize, TerrainSettings.TrackGenerationSettings.PointInsideErrorTolerance, TerrainSettings.TrackGenerationSettings.TrackElevationOffset));
		}
	}
}

void ATerrainManager::RecalculateTileForSector(const FIntVector2D Sector)
{
	for (ATerrainTile* Tile : TilesInUse)
	{
		if (Tile->GetCurrentSector() == Sector)
		{
			FTerrainJob Job;
			Job.TerrainTile = Tile;
			PendingTerrainJobQueue.Enqueue(Job);
			return;
		}
	}	
}

bool ATerrainManager::IsLocationCoveredByTile(const FVector Location)
{
	const FIntVector2D SectorNeeded = CalculateSectorFromLocation(Location);
	//bool bIsSectorCovered = false;

	// check if sector is covered by a tile
	return SectorsCurrentlyProcessed.Contains(SectorNeeded);
	/*for (const ATerrainTile* Tile : TilesInUse)
	{
		if (Tile->GetCurrentSector() == SectorNeeded)
		{
			bIsSectorCovered = true;
			break;
		}
	}

	return bIsSectorCovered;*/
}

float ATerrainManager::GetTerrainSettingsTransitionElevationOffset() const
{
	return TerrainSettings.TransitionElevationOffset;
}

float ATerrainManager::GetTerrainSettingsTransitionInterpolationSpeed() const
{
	return TerrainSettings.TransitionInterpolationSpeed;
}

float ATerrainManager::GetTerrainSettingsTransitionDeltaToStop() const
{
	return TerrainSettings.TransitionDeltaToStop;
}

void ATerrainManager::BeginTileGenerationForReset(const FVector Location)
{
	bRecalculatedTileBehindStartPoint = false;
	// calculate tiles needed around the given sector
	SectorsNeedCoverageForReset.Empty();
	CalculateSectorsNeededAroundGivenLocation(Location, SectorsNeedCoverageForReset);

	// remove those that are already processed
	for (const FIntVector2D Vec : SectorsCurrentlyProcessed)
	{
		SectorsNeedCoverageForReset.Remove(Vec);
	}

	// create tiles
	BuildTerrainAroundSector(CalculateSectorFromLocation(Location));

	bShouldCheckSectorsNeedCoverageForReset = true;
}

bool ATerrainManager::ContainsSectorTrack(const FIntVector2D Sector) const
{
	FSectorTrackInfo TrackInfo = TrackMap.FindRef(Sector);
	return TrackInfo.bSectorHasTrack;
}

// Creates a FRuntimeMeshVertexSimple from the given Vertex
FRuntimeMeshVertexSimple ATerrainManager::CreateRuntimeMeshVertexSimple(const FVector Vertex, const FVector Normal) const
{
	return FRuntimeMeshVertexSimple(
		Vertex,										// Vertex position
		Normal,											// Vertex normal
		FRuntimeMeshTangent(0.f, -1.f, 0.f),			// Vertex tangent
		FColor::White,
		FVector2D(Vertex.X / 500.f, Vertex.Y / 500.f)	// Vertex texture coordinates
	);
}



