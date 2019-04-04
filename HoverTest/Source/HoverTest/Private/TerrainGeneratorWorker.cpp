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
			FDEM DEM = FDEM(TerrainSettings.FractalNoiseTerrainSettings.H, TerrainSettings.FractalNoiseTerrainSettings.I, TerrainSettings.FractalNoiseTerrainSettings.I_bu, TerrainSettings.FractalNoiseTerrainSettings.rt, TerrainSettings.FractalNoiseTerrainSettings.rs, TerrainSettings.FractalNoiseTerrainSettings.n);
			/* vertex data hardcoded for the moment */
			TArray<FVector> InitialValues;
			InitialValues.Add(FVector(0.f, 0.f, TerrainSettings.Point1Elevation));
			InitialValues.Add(FVector(TerrainSettings.TileEdgeSize, 0.f, TerrainSettings.Point2Elevation));
			InitialValues.Add(FVector(TerrainSettings.TileEdgeSize, TerrainSettings.TileEdgeSize, TerrainSettings.Point3Elevation));
			InitialValues.Add(FVector(0.f, TerrainSettings.TileEdgeSize, TerrainSettings.Point4Elevation));

			TArray<FVector> DEMConstraints;
			DEMConstraints.Append(InitialValues);
			DEMConstraints.Add(FVector(TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.TileEdgeSize / 2.f, TerrainSettings.Point5Elevation));

			TerrainJob.MeshData.Add(FMeshData());

			DEM.SimulateTriangleEdge(&InitialValues, 0, TerrainSettings.TriangleEdgeIterations);
			DEM.MidpointDisplacementBottomUp(&DEMConstraints);
			DEM.TriangleEdge(&InitialValues, 0, TerrainSettings.TriangleEdgeIterations, TerrainJob.MeshData[0].VertexBuffer, TerrainJob.MeshData[0].TriangleBuffer);

			TerrainJob.TerrainTile->SetVerticesLeftBorder(DEM.VerticesLeftBorder);
			TerrainJob.TerrainTile->SetVerticesRightBorder(DEM.VerticesRightBorder);
			TerrainJob.TerrainTile->SetVerticesTopBorder(DEM.VerticesTopBorder);
			TerrainJob.TerrainTile->SetVerticesBottomBorder(DEM.VerticesBottomBorder);
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
