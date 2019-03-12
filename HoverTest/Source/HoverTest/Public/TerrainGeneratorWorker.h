// Fill out your copyright notice in the Description page of Project Settings.

/**
* code of this class is taken and adapted from https://github.com/midgen/cashgenUE
*/

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/Containers/Queue.h"
#include "MyStaticLibrary.h"

class ATerrainManager;
struct FTerrainSettings;

/**
 * 
 */
class HOVERTEST_API TerrainGeneratorWorker : public FRunnable
{
public:
	TerrainGeneratorWorker(ATerrainManager* Manager, FTerrainSettings Settings, TQueue<FTerrainJob, EQueueMode::Spsc>* Queue);
	~TerrainGeneratorWorker();

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

private:
	ATerrainManager* TerrainManager;
	FTerrainSettings TerrainSettings;
	TQueue<FTerrainJob, EQueueMode::Spsc>* InputQueue;
	FTerrainJob TerrainJob;
	bool IsThreadFinished;

};
