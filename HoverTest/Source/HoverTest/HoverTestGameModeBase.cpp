// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTestGameModeBase.h"
#include "TrackObserver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Hovercraft.h"
#include "Checkpoint.h"
#include "HovercraftPlayerController.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "FinishLine.h"
#include "Classes/GameFramework/Actor.h"

void AHoverTestGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		for (TActorIterator<ATrackObserver> ActorIter(GetWorld()); ActorIter; ++ActorIter)
		{
			TrackObserver = *ActorIter;
		}

		for (TActorIterator<AFinishLine> ActorIter(GetWorld()); ActorIter; ++ActorIter)
		{
			FinishLineInternPointer = *ActorIter;
		}
	}

	if (!TrackObserver)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find a TrackObserver in %s. Did you forget to add one to the level?"), *GetName());
	}
	else
	{
		NumberOfCheckpointsOnTrack = TrackObserver->Checkpoints.Num();
	}

	if (!FinishLineInternPointer)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find a FinishLine in %s. Did you forget to add one to the level?"), *GetName());
	}
}

ATrackObserver * AHoverTestGameModeBase::GetTrackObserver() const
{
	return TrackObserver;
}

void AHoverTestGameModeBase::RegisterTrackObserver(ATrackObserver * Observer)
{
	TrackObserver = Observer;
}

void AHoverTestGameModeBase::HandlePlayerHovercraftCheckpointOverlap(AHovercraft * Hovercraft, AHovercraftPlayerController* PlayerController, ACheckpoint * Checkpoint)
{
	if (!Hovercraft || !Checkpoint || !PlayerController) 
	{
		return;
	}

	// check if player is passing the same checkpoint like last time (happens for example when resetting). Ignore if so
	if (IsSameCheckpointForHovercraft(Checkpoint, Hovercraft)) { return; }

	// check if this checkpoint is the correct next checkpoint for the player
	if (IsCorrectNextCheckpointForHovercraft(Checkpoint, Hovercraft))
	{
		Hovercraft->SetIndexOfLastCheckpoint(Checkpoint->GetCheckpointIndex());
		PlayerController->SetResetPosition(Hovercraft->GetActorLocation());
		PlayerController->SetResetYaw(Hovercraft->GetActorRotation().Yaw);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Missed a checkpoint! Use F to reset to latest checkpoint."));
		}
	}
}

void AHoverTestGameModeBase::HandleAIHovercraftCheckpointOverlap(AHovercraft * Hovercraft, ACheckpoint * Checkpoint)
{
	if (!Hovercraft || !Checkpoint) { return; }
	if (IsSameCheckpointForHovercraft(Checkpoint, Hovercraft)) { return; }
	if (IsCorrectNextCheckpointForHovercraft(Checkpoint, Hovercraft))
	{
		Hovercraft->SetIndexOfLastCheckpoint(Checkpoint->GetCheckpointIndex());
	}

}

void AHoverTestGameModeBase::HandleHovercraftFinishLineOverlap(AHovercraft * Hovercraft, AFinishLine * FinishLine)
{
	if (!Hovercraft || !FinishLine) { return; }

	// check if hovercraft has all checkpoints passed
	if (Hovercraft->GetIndexOfLastCheckpoint() == (NumberOfCheckpointsOnTrack - 1))
	{
		// if yes: finish lap & start new lap
		float LapTime = Hovercraft->ResetLapTimer();
		UE_LOG(LogTemp, Warning, TEXT("New lap started."));
		UE_LOG(LogTemp, Warning, TEXT("Lap time: %f"), LapTime);
		Hovercraft->SetIndexOfLastCheckpoint(-1);
		
		// check if player craft
		AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(Hovercraft->GetController());
		if (PC)
		{
			/* player */
			PC->SetResetPosition(Hovercraft->GetActorLocation());
			PC->SetResetYaw(Hovercraft->GetActorRotation().Yaw);
		}
	}
	else
	{
		// if no:
			// if player has -2 as checkpoint index: start new lap
		if (Hovercraft->GetIndexOfLastCheckpoint() == -2)
		{
			UE_LOG(LogTemp, Warning, TEXT("New initial lap started"));
			Hovercraft->SetStopLapTime(true);
			Hovercraft->SetIndexOfLastCheckpoint(-1);

			// check if player craft
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(Hovercraft->GetController());
			if (PC)
			{
				/* player */
				PC->SetResetPosition(Hovercraft->GetActorLocation());
				PC->SetResetYaw(Hovercraft->GetActorRotation().Yaw);
			}
		}
	}



}

AActor* AHoverTestGameModeBase::GetAINextCheckpointActor(const int32 LastCheckpointIndex)
{
	if (!TrackObserver || !FinishLineInternPointer) { return false; }

	// -1 = spawned, already passed finish line but not yet passed first checkpoint
	// -2 = spawned, not yet passed finish line
	if (LastCheckpointIndex <= -2 || LastCheckpointIndex == (NumberOfCheckpointsOnTrack - 1))
	{
		return FinishLineInternPointer;
	}
	// invalid LastCheckpointIndex
	else if (LastCheckpointIndex >= NumberOfCheckpointsOnTrack)
	{
		return nullptr;
	}
	// valid last checkpoint -> seach location of next checkpoint
	else
	{
		return TrackObserver->Checkpoints[LastCheckpointIndex + 1];
	}
}

bool AHoverTestGameModeBase::IsCorrectNextCheckpointForHovercraft(ACheckpoint * Checkpoint, AHovercraft * Hovercraft) const
{
	if (!Checkpoint || !Hovercraft) { return false; }
	if (!TrackObserver || NumberOfCheckpointsOnTrack == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not check if %s is the correct next checkpoint for %s because the TrackObserver in GameMode is invalid or has no checkpoints assigned."), *Checkpoint->GetName(), *Hovercraft->GetName());
		return false;
	}

	int32 HovercraftLastCheckpointIndex = Hovercraft->GetIndexOfLastCheckpoint();
	int32 CheckpointIndex = Checkpoint->GetCheckpointIndex();
	if (CheckpointIndex == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("Uninitialized checkpoint: %s"), *Checkpoint->GetName());
		return false;
	}

	// hard coded :)
	//if (HovercraftLastCheckpointIndex == -1 && CheckpointIndex == 0) { return true; }
	//// this case should not appear -> missed finish line
	//if (HovercraftLastCheckpointIndex == (NumberOfCheckpointsOnTrack - 1) && CheckpointIndex == 0) { return true; }
	// general case
	if (HovercraftLastCheckpointIndex == (CheckpointIndex - 1)) 
	{ 
		return true; 
	}
	else
	{
		return false;
	}
}

bool AHoverTestGameModeBase::IsSameCheckpointForHovercraft(ACheckpoint * Checkpoint, AHovercraft * Hovercraft) const
{
	if (!Checkpoint || !Hovercraft) { return false; }

	return (Hovercraft->GetIndexOfLastCheckpoint() == Checkpoint->GetCheckpointIndex());
}
