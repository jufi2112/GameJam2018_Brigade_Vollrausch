// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "TerrainTile.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "TerrainGeneratorWorker.h"


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
}

TArray<FIntVector2D> ATerrainManager::CalculateSectorsNeededAroundGivenLocation(FVector Location)
{
	TArray<FIntVector2D> Sectors;

	FIntVector2D BaseSector = CalculateSectorFromLocation(Location);

	for (int x = -TerrainSettings.TilesToBeCreatedAroundActorRadius; x <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++x)
	{
		for (int y = -TerrainSettings.TilesToBeCreatedAroundActorRadius; y <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++y)
		{
			Sectors.Add(FIntVector2D(BaseSector.X + x, BaseSector.Y + y));
		}
	}

	return Sectors;
}

TArray<FIntVector2D> ATerrainManager::CalculateSectorsNeededAroundGivenSector(FIntVector2D Sector)
{
	TArray<FIntVector2D> Sectors;

	for (int x = -TerrainSettings.TilesToBeCreatedAroundActorRadius; x <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++x)
	{
		for (int y = -TerrainSettings.TilesToBeCreatedAroundActorRadius; y <= TerrainSettings.TilesToBeCreatedAroundActorRadius; ++y)
		{
			Sectors.Add(FIntVector2D(Sector.X + x, Sector.Y + y));
		}
	}

	return Sectors;
}

void ATerrainManager::CalculateTrackPath()
{
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
			// calculate_points()

			FVector2D EntryPoint;
			FVector2D ExitPoint;
			// add to TrackMap
		}
		
		CurrentTrackSector = NextTrackSector;
		// update_nexttracksector
	}

}

void ATerrainManager::CalculateTrackPoints(FVector2D & OUTTrackEntryPoint, FVector2D & OUTTrackExitPoint)
{

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
			TerrainCreationQueue[i].Enqueue(Job);
		}
	}

	// check if we need to update mesh data
	for (int i = 0; i < TerrainSettings.MeshUpdatesPerFrame; ++i)
	{
		FTerrainJob Job;
		if (FinishedJobQueue.Dequeue(Job))
		{
			if (Job.TerrainTile == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("TerrainTile pointer in Job is nullptr!"));
			}
			else
			{
				Job.TerrainTile->UpdateMeshData(TerrainSettings, Job.MeshData);
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
		TArray<FIntVector2D> SectorsThatNeedCoverage = CalculateSectorsNeededAroundGivenLocation(ActorToTrack->GetActorLocation());

		// iterate all existing tiles and check if they already cover the sectors the actor needs covered
		for (ATerrainTile* Tile : TilesInUse)
		{
			int32 RemovedItems = SectorsThatNeedCoverage.Remove(Tile->GetCurrentSector());
			if (RemovedItems > 0)
			{
				Tile->AddAssociatedActor();
			}
		}

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

void ATerrainManager::RemoveTrackedActor(AActor * ActorToRemove)
{
	if (ActorToRemove == nullptr) { return; }
	int32 NumberOfRemovedActors = TrackedActors.RemoveSingle(ActorToRemove);

	if (NumberOfRemovedActors != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not remove specified Actor in RemoveTrackedActor in %s"), *GetName());
	}

	// free Tiles associated with this actor
	TArray<FIntVector2D> Sectors = CalculateSectorsNeededAroundGivenLocation(ActorToRemove->GetActorLocation());
	TArray<ATerrainTile*> TilesToFree;

	for (ATerrainTile* Tile : TilesInUse)
	{
		if (Sectors.Remove(Tile->GetCurrentSector()) > 0)
		{
			if (Tile->RemoveAssociatedActor() == 0)
			{
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
	TArray<FIntVector2D> SectorsNeededAtNewPosition = CalculateSectorsNeededAroundGivenSector(NewSector);
	TArray<FIntVector2D> SectorsNeededAtPreviousPosition = CalculateSectorsNeededAroundGivenSector(PreviousSector);

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
			if (Tile->RemoveAssociatedActor() == 0)
			{
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
	SectorsNeededAtNewPosition = CalculateSectorsNeededAroundGivenSector(NewSector);
	SectorsNeededAtPreviousPosition = CalculateSectorsNeededAroundGivenSector(PreviousSector);

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

int32 ATerrainManager::GetTrackPointsForSector(const FIntVector2D Sector, FVector2D & OUTTrackEntryPoint, FVector2D & OUTTrackExitPoint)
{
	FSectorTrackInfo* TrackInfo = TrackMap.Find(Sector);

	if (!TrackInfo)
	{
		return -1;
	}
	else
	{
		if (!TrackInfo->bSectorHasTrack)
		{
			return 0;
		}
		else
		{
			OUTTrackEntryPoint = TrackInfo->TrackEntryPoint;
			OUTTrackExitPoint = TrackInfo->TrackExitPoint;
			return 1;
		}
	}
}



