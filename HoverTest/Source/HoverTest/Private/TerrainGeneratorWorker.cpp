// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainGeneratorWorker.h"
#include "TerrainManager.h"

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
			UMyStaticLibrary::CreateComplexMeshData(TerrainSettings, TerrainJob.TerrainMeshData, TerrainJob.TrackMeshData);
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
