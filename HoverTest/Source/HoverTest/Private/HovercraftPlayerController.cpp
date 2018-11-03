// Fill out your copyright notice in the Description page of Project Settings.

#include "HovercraftPlayerController.h"
#include "Hovercraft.h"
#include "TimerManager.h"
#include "Engine/World.h"

void AHovercraftPlayerController::SetResetPosition(FVector NewResetPosition)
{
	ResetPosition = NewResetPosition;
}

FVector AHovercraftPlayerController::GetResetPosition()
{
	return ResetPosition;
}

float AHovercraftPlayerController::GetResetYaw()
{
	return ResetYaw;
}

void AHovercraftPlayerController::SetResetYaw(float Yaw)
{
	ResetYaw = Yaw;
}

void AHovercraftPlayerController::AfterDelay()
{
	AHovercraft* Pawn = Cast<AHovercraft>(GetPawn());

	if (Pawn)
	{
		if (!Pawn->GetStaticMeshLocation(ResetPosition))
		{
			ResetPosition = FVector(0, 0, 0);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve Controlled Pawn in %s"), *GetName());
	}
}

void AHovercraftPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ResetPosition = FVector(0.f, 0.f, 0.f);
	// execute after one second
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(StartDelayTimerHandle, this, &AHovercraftPlayerController::AfterDelay, 1.f, false);
	}
}
