// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainGeneratorWorker.h"
#include "TerrainManager.h"
#include "TerrainGenerator.h"
#include "TerrainTile.h"

TerrainGeneratorWorker::TerrainGeneratorWorker(ATerrainManager* Manager, FTerrainSettings Settings, TQueue<FTerrainJob, EQueueMode::Spsc>* Queue)
{
	TerrainManager = Manager;
	TerrainSettings = Settings;
	InputQueue = Queue;

}

TerrainGeneratorWorker::~TerrainGeneratorWorker()
{

}

bool TerrainGeneratorWorker::Init()
{
	IsThreadFinished = false;
	return true;
}

uint32 TerrainGeneratorWorker::Run()
{
	while (!IsThreadFinished)
	{
		if (InputQueue->Dequeue(TerrainJob))
		{
			//UMyStaticLibrary::CreateComplexMeshData(TerrainSettings, TerrainJob.MeshData);
			//UE_LOG(LogTemp, Error, TEXT("No terrain generation functions implemented yet!"));
			FDEM DEM = FDEM
			(
				TerrainSettings.FractalNoiseTerrainSettings.H, 
				TerrainSettings.FractalNoiseTerrainSettings.I, 
				TerrainSettings.FractalNoiseTerrainSettings.I_bu, 
				TerrainSettings.FractalNoiseTerrainSettings.rt, 
				TerrainSettings.FractalNoiseTerrainSettings.rs, 
				TerrainSettings.FractalNoiseTerrainSettings.n, 
				TerrainSettings.TerrainMaterialTransitionLowMediumElevation, 
				TerrainSettings.TerrainMaterialTransitionMediumHighElevation, 
				TerrainSettings.TransitionElevationVariationLowMedium,
				TerrainSettings.TransitionElevationVariationMediumHigh
			);

			if (!TerrainManager) 
			{ 
				UE_LOG(LogTemp, Error, TEXT("Provided TerrainManager is nullptr in TerrainGeneratorWorker"));
				return 1; 
			}

			// array to save all constraints for the new DEM
			TArray<FVector> Constraints;
			// array to save all border constraints for the new DEM
			TArray<FBorderVertex> BorderConstraints;
			// bools to check if corner points already definded by a constraint
			bool bBottomLeftCorner = false;
			bool bBottomRightCorner = false;
			bool bTopRightCorner = false;
			bool bTopLeftCorner = false;
			// array with the defining points for the DEM (the corners of the DEM)
			TArray<FVector> DefiningPoints;
			DefiningPoints.Init(FVector(), 4);
			DefiningPoints[0] = FVector(0.f, 0.f, 0.f);
			DefiningPoints[1] = FVector(0.f, TerrainSettings.TileEdgeSize, 0.f);
			DefiningPoints[2] = FVector(TerrainSettings.TileEdgeSize, TerrainSettings.TileEdgeSize, 0.f);
			DefiningPoints[3] = FVector(TerrainSettings.TileEdgeSize, 0.f, 0.f);

			// get adjacent tiles
			TArray<ATerrainTile*> AdjacentTiles;
			TerrainManager->GetAdjacentTiles(TerrainJob.TerrainTile->GetCurrentSector(), AdjacentTiles, true);

			//UE_LOG(LogTemp, Warning, TEXT("Found %i relevant adjacent tiles"), AdjacentTiles.Num());

			for (ATerrainTile* Tile : AdjacentTiles)
			{
				if (!Tile->GetVerticesOnBorderSet()) { continue; }
				else
				{
					TArray<FBorderVertex> Verts;
					// top tile?
					if (Tile->GetCurrentSector() == (TerrainJob.TerrainTile->GetCurrentSector() + FIntVector2D(1, 0)))
					{
						Tile->GetVerticesBottomBorder(Verts);
						BorderConstraints.Append(Verts);
						if (!bTopRightCorner)
						{
							DefiningPoints[2].Z = Tile->GetBottomRightCorner().Z;
							bTopRightCorner = true;
						}
						if (!bTopLeftCorner)
						{
							DefiningPoints[3].Z = Tile->GetBottomLeftCorner().Z;
							bTopLeftCorner = true;
						}
						//UE_LOG(LogTemp, Warning, TEXT("Found a top tile (tile %s) with %i constraints"), *Tile->GetCurrentSector().ToString(), Verts.Num());
						continue;
					}
					// bottom tile?
					if (Tile->GetCurrentSector() == (TerrainJob.TerrainTile->GetCurrentSector() - FIntVector2D(1, 0)))
					{
						Tile->GetVerticesTopBorder(Verts);
						BorderConstraints.Append(Verts);
						if (!bBottomLeftCorner)
						{
							DefiningPoints[0].Z = Tile->GetTopLeftCorner().Z;
							bBottomLeftCorner = true;
						}
						if (!bBottomRightCorner)
						{
							DefiningPoints[1].Z = Tile->GetTopRightCorner().Z;
							bBottomRightCorner = true;
						}
						//UE_LOG(LogTemp, Warning, TEXT("Found a bottom tile (tile %s) with %i constraints"), *Tile->GetCurrentSector().ToString(), Verts.Num());
						continue;
					}
					// right tile?
					if (Tile->GetCurrentSector() == (TerrainJob.TerrainTile->GetCurrentSector() + FIntVector2D(0, 1)))
					{
						Tile->GetVerticesLeftBorder(Verts);
						BorderConstraints.Append(Verts);
						if (!bBottomRightCorner)
						{
							DefiningPoints[1].Z = Tile->GetBottomLeftCorner().Z;
							bBottomRightCorner = true;
						}
						if (!bTopRightCorner)
						{
							DefiningPoints[2].Z = Tile->GetTopLeftCorner().Z;
							bTopRightCorner = true;
						}
						//UE_LOG(LogTemp, Warning, TEXT("Found a right tile (tile %s) with %i constraints"), *Tile->GetCurrentSector().ToString(), Verts.Num());
						continue;
					}
					// left tile?
					if (Tile->GetCurrentSector() == (TerrainJob.TerrainTile->GetCurrentSector() - FIntVector2D(0, 1)))
					{
						Tile->GetVerticesRightBorder(Verts);
						BorderConstraints.Append(Verts);
						if (!bBottomLeftCorner)
						{
							DefiningPoints[0].Z = Tile->GetBottomRightCorner().Z;
							bBottomLeftCorner = true;
						}
						if (!bTopLeftCorner)
						{
							DefiningPoints[3].Z = Tile->GetTopRightCorner().Z;
							bTopLeftCorner = true;
						}
						//UE_LOG(LogTemp, Warning, TEXT("Found a left tile (tile %s) with %i constraints"), *Tile->GetCurrentSector().ToString(), Verts.Num());
						continue;
					}
				}
			}

			// check if we got all defining points, if not, use default values
			if (!bBottomLeftCorner)
			{
				DefiningPoints[0].Z = TerrainSettings.Point1Elevation;
				bBottomLeftCorner = true;
			}
			if (!bBottomRightCorner)
			{
				DefiningPoints[1].Z = TerrainSettings.Point2Elevation;
				bBottomRightCorner = true;
			}
			if (!bTopRightCorner)
			{
				DefiningPoints[2].Z = TerrainSettings.Point3Elevation;
				bTopRightCorner = true;
			}
			if (!bTopLeftCorner)
			{
				DefiningPoints[3].Z = TerrainSettings.Point3Elevation;
				bTopLeftCorner = true;
			}

			if ((Constraints.Num() == 0) && (BorderConstraints.Num() == 0))
			{
				// add default values for the moment
				/* vertex data hardcoded for the moment */
				Constraints.Append(DefiningPoints);
				Constraints.Add(FVector(TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.Point5Elevation));
				
			}

			TerrainJob.MeshData.Add(FMeshData());

			// get possible track entry and exit points
			FVector2D TrackEntryPoint;
			FVector2D TrackExitPoint;

			int32 SectorProcessed = TerrainManager->GetTrackPointsForSector(TerrainJob.TerrainTile->GetCurrentSector(), TrackEntryPoint, TrackExitPoint);
			// make sure we already processed the current sector in TerrainManager (should always be processed, but better safe than sorry
			while (SectorProcessed == -1)
			{
				FPlatformProcess::Sleep(0.01f);
				SectorProcessed = TerrainManager->GetTrackPointsForSector(TerrainJob.TerrainTile->GetCurrentSector(), TrackEntryPoint, TrackExitPoint);
			}

			// check if the tile should have a track inside
			if (SectorProcessed == 1)
			{
				// calculate track mesh
				TArray<FRuntimeMeshVertexSimple> TrackVertexBuffer;
				TArray<int32> TrackTriangleBuffer;
				TArray<FRuntimeMeshVertexSimple> Array1;
				TArray<int32> Array2;
				TerrainManager->GenerateTrackMesh(TrackEntryPoint, TrackExitPoint, Array1, Array2);
			}

			DEM.SimulateTriangleEdge(&DefiningPoints, 0, TerrainSettings.FractalNoiseTerrainSettings.TriangleEdgeIterations);
			DEM.MidpointDisplacementBottomUp(&Constraints, &BorderConstraints);
			DEM.TriangleEdge(&DefiningPoints, 0, TerrainSettings.FractalNoiseTerrainSettings.TriangleEdgeIterations);// , TerrainJob.MeshData);
			DEM.CopyBufferToMeshData(TerrainJob.MeshData);
			DEM.CalculateBorderVertexNormals();

			/*UE_LOG(LogTemp, Error, TEXT("DEM constraints array log for tile %s in sector %s"), *TerrainJob.TerrainTile->GetName(), *TerrainJob.TerrainTile->GetCurrentSector().ToString());
			UE_LOG(LogTemp, Warning, TEXT("#VerticesTopBorder: %i"), DEM.VerticesTopBorder.Num());
			UE_LOG(LogTemp, Warning, TEXT("#VerticesBottomBorder: %i"), DEM.VerticesBottomBorder.Num());
			UE_LOG(LogTemp, Warning, TEXT("#VerticesLeftBorder: %i"), DEM.VerticesLeftBorder.Num());
			UE_LOG(LogTemp, Warning, TEXT("#VerticesRightBorder: %i"), DEM.VerticesRightBorder.Num());
			UE_LOG(LogTemp, Error, TEXT("Finished"));*/

			TerrainJob.TerrainTile->SetVerticesLeftBorder(DEM.VerticesLeftBorder);
			TerrainJob.TerrainTile->SetVerticesRightBorder(DEM.VerticesRightBorder);
			TerrainJob.TerrainTile->SetVerticesTopBorder(DEM.VerticesTopBorder);
			TerrainJob.TerrainTile->SetVerticesBottomBorder(DEM.VerticesBottomBorder);
			TerrainJob.TerrainTile->SetBottomLeftCorner(DEM.BottomLeftCorner);
			TerrainJob.TerrainTile->SetBottomRightCorner(DEM.BottomRightCorner);
			TerrainJob.TerrainTile->SetTopRightCorner(DEM.TopRightCorner);
			TerrainJob.TerrainTile->SetTopLeftCorner(DEM.TopLeftCorner);
			TerrainJob.TerrainTile->AllVerticesOnBorderSet();

			/*UMyStaticLibrary::SaveBuffersToFile(TerrainJob.MeshData[0].VertexBuffer, TerrainJob.MeshData[0].TriangleBuffer);
			DEM.SaveDEMToFile();*/


			TerrainManager->FinishedJobQueue.Enqueue(TerrainJob);
		}
		else
		{
			FPlatformProcess::Sleep(0.01f);
		}
	}
	return 1;
}

void TerrainGeneratorWorker::Stop()
{
	IsThreadFinished = true;
}

void TerrainGeneratorWorker::Exit()
{

}
